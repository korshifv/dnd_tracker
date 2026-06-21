#include "NoteEditor.h"
#include "Storage.h"
#include "MarkdownHighlighter.h"
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QRegularExpression>
#include <QStackedWidget>
#include <QTextBrowser>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>
#include <QPushButton>
#include <QApplication>

NoteEditor::NoteEditor(QWidget *parent)
    : QWidget(parent) {
  // Таймер рендера превью: debounce 300мс, чтобы не рендерить на каждое нажатие.
  m_renderTimer = new QTimer(this);
  m_renderTimer->setSingleShot(true);
  m_renderTimer->setInterval(300);
  connect(m_renderTimer, &QTimer::timeout, this, &NoteEditor::renderPreview);

  // Автосейв: 3с бездействия → запись на диск.
  m_autosaveTimer = new QTimer(this);
  m_autosaveTimer->setSingleShot(true);
  m_autosaveTimer->setInterval(3000);
  connect(m_autosaveTimer, &QTimer::timeout, this, &NoteEditor::doSave);

  // Индикатор статуса: 2с показа "Сохранено ✓".
  m_statusTimer = new QTimer(this);
  m_statusTimer->setSingleShot(true);
  m_statusTimer->setInterval(2000);
  connect(m_statusTimer, &QTimer::timeout, this,
          [this]() { statusLabel->setText(""); });

  auto *root = new QVBoxLayout(this);
  root->setContentsMargins(10, 10, 10, 10);

  // Верхняя панель с кнопкой переключения режима
  auto *topBar = new QHBoxLayout();
  topBar->addStretch();
  
  toggleModeBtn = new QPushButton("👁 Режим чтения");
  toggleModeBtn->setCheckable(true);
  connect(toggleModeBtn, &QPushButton::clicked, this, &NoteEditor::toggleMode);
  topBar->addWidget(toggleModeBtn);
  root->addLayout(topBar);

  // Единый виджет-контейнер
  stackedWidget = new QStackedWidget();

  editor = new QTextEdit();
  editor->setPlaceholderText("Пишите заметку в Markdown. "
                             "Используйте [[Имя]] для ссылок на персонажей "
                             "или другие заметки.");
  editor->setFrameShape(QFrame::NoFrame);
  editor->setStyleSheet("QTextEdit { padding: 10px; border: none; }");
  // Применяем хайлайтер
  highlighter = new MarkdownHighlighter(editor->document());

  preview = new QTextBrowser();
  preview->setOpenExternalLinks(false); // обрабатываем клики сами
  preview->setFrameShape(QFrame::NoFrame);
  preview->setStyleSheet("QTextBrowser { padding: 10px; border: none; background: transparent; }");

  stackedWidget->addWidget(editor);
  stackedWidget->addWidget(preview);
  root->addWidget(stackedWidget, 1);

  // Подключение сигналов.
  connect(editor, &QTextEdit::textChanged, this, [this]() {
    if (!m_loaded)
      return;
    m_renderTimer->start();
    markDirty();
  });
  connect(preview, &QTextBrowser::anchorClicked, this, &NoteEditor::onLinkClicked);

  // Индикатор статуса внизу.
  statusLabel = new QLabel("");
  statusLabel->setStyleSheet("color: palette(link); font-size: 0.8em;");
  statusLabel->setAlignment(Qt::AlignRight);
  root->addWidget(statusLabel);

  // Изначально пустое состояние (ничего не выбрано)
  clearNote();
}

void NoteEditor::clearNote() {
  flushSave();
  m_loaded = false;
  m_relativePath.clear();
  editor->clear();
  editor->setDisabled(true);
  preview->clear();
  preview->setDisabled(true);
  toggleModeBtn->setDisabled(true);
  statusLabel->setText("Выберите заметку слева");
}

void NoteEditor::loadNote(const QString &relativePath) {
  flushSave();
  m_loaded = false;
  m_relativePath = relativePath;
  
  editor->setEnabled(true);
  preview->setEnabled(true);
  toggleModeBtn->setEnabled(true);
  statusLabel->setText("");
  
  loadFromFile();
  renderPreview();

  m_loaded = true;
}

void NoteEditor::toggleMode() {
  if (toggleModeBtn->isChecked()) {
    // Включаем режим чтения
    toggleModeBtn->setText("✏ Редактировать");
    renderPreview();
    stackedWidget->setCurrentWidget(preview);
  } else {
    // Включаем режим редактирования
    toggleModeBtn->setText("👁 Режим чтения");
    stackedWidget->setCurrentWidget(editor);
  }
}

void NoteEditor::loadFromFile() {
  QFile f(Storage::notesDir() + "/" + m_relativePath + ".md");
  if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    editor->setPlainText(QString::fromUtf8(f.readAll()));
  } else {
    editor->clear();
  }
}

void NoteEditor::markDirty() {
  if (!m_loaded)
    return;
  m_autosaveTimer->start();
  m_statusTimer->stop();
  statusLabel->setText("Изменения…");
}

void NoteEditor::renderPreview() {
  // Markdown с инжектированными wiki-ссылками → рендер Qt.
  const QString md = injectWikiLinks(editor->toPlainText());
  preview->setMarkdown(md);
}

void NoteEditor::onLinkClicked(const QUrl &url) {
  // Кастомная схема wiki: открывает персонажа или заметку по имени.
  const QString s = url.toString();
  if (s.startsWith("wiki:")) {
    const QString name = QUrl::fromPercentEncoding(s.mid(5).toUtf8());
    
    // Эвристика: ищем рекурсивно в notesDir
    QString foundRelPath = Storage::findNoteRecursively(name);
    if (!foundRelPath.isEmpty()) {
      emit requestOpenNote(foundRelPath);
    } else {
      emit requestOpenCharacter(name);
    }
  }
}

void NoteEditor::doSave() {
  if (m_relativePath.isEmpty() || !m_loaded) return;
  QFile f(Storage::notesDir() + "/" + m_relativePath + ".md");
  if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    f.write(editor->toPlainText().toUtf8());
    f.close();
    emit saved();
    statusLabel->setText("Сохранено ✓");
    m_statusTimer->start();
  }
}

void NoteEditor::flushSave() {
  m_autosaveTimer->stop();
  doSave();
}

QString NoteEditor::injectWikiLinks(const QString &markdown) {
  // [[Имя]] → [Имя](wiki:%percent-encoded%).
  // Используем отрицательный взгляд вперёд/назад, чтобы не задеть уже
  // обработанные ссылки и экранированные скобки.
  QString result = markdown;
  static const QRegularExpression re("(?<!\\[)\\[\\[([^\\]\\n]+?)\\]\\]");
  QRegularExpressionMatchIterator it = re.globalMatch(result);
  // Собираем замены с конца, чтобы индексы не сбивались.
  QList<QRegularExpressionMatch> matches;
  while (it.hasNext())
    matches.prepend(it.next());
  for (const auto &m : matches) {
    const QString name = m.captured(1).trimmed();
    const QString encoded =
        QString::fromUtf8(QUrl::toPercentEncoding(name));
    const QString replacement =
        QString("[%1](wiki:%2)").arg(name, encoded);
    result.replace(m.capturedStart(), m.capturedLength(), replacement);
  }
  return result;
}

NoteEditor::~NoteEditor() {
  flushSave();
}
