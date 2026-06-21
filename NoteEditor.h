#ifndef NOTEEDITOR_H
#define NOTEEDITOR_H

#include <QString>
#include <QTimer>
#include <QWidget>

class QTextEdit;
class QTextBrowser;
class QLabel;
class QStackedWidget;
class QPushButton;
class MarkdownHighlighter;

// Редактор заметки в стиле Obsidian: единый редактор с переключением
// режима (Редактирование / Чтение). Поддерживает wiki-ссылки [[Имя]].
// Заметки хранятся как .md файлы в Storage::notesDir().
class NoteEditor : public QWidget {
  Q_OBJECT
public:
  explicit NoteEditor(QWidget *parent = nullptr);
  ~NoteEditor();

  // Загрузить заметку по относительному пути
  void loadNote(const QString &relativePath);

  // Очистить редактор (при удалении или снятии выделения)
  void clearNote();

  // Принудительное немедленное сохранение (без ожидания таймера).
  // Вызывается при переключении на другую заметку / закрытии.
  void flushSave();

  QString getNoteName() const { return m_relativePath; }

signals:
  // Wiki-ссылка указывает на персонажа — MainWindow ищет filePath по имени.
  void requestOpenCharacter(const QString &name);
  // Wiki-ссылка указывает на другую заметку — MainWindow открывает её вкладку.
  void requestOpenNote(const QString &name);
  // Заметка сохранена на диск — NoteRepository обновляет список.
  void saved();

private slots:
  void markDirty();     // перезапуск таймера автосейва + индикатор
  void renderPreview(); // перерендерить превью из текста редактора
  void onLinkClicked(const class QUrl &url); // клик по wiki-ссылке
  void doSave();        // фактическая запись на диск
  void toggleMode();    // переключение Редактирование <-> Чтение

private:
  QString m_relativePath;   // относительный путь заметки (без расширения)
  QStackedWidget *stackedWidget;
  QTextEdit *editor;
  QTextBrowser *preview;
  QLabel *statusLabel;
  QPushButton *toggleModeBtn;
  MarkdownHighlighter *highlighter;

  QTimer *m_renderTimer;    // debounce 300мс перед рендером превью
  QTimer *m_autosaveTimer;  // 3000мс бездействия → doSave
  QTimer *m_statusTimer;    // 2000мс показа "Сохранено ✓"
  bool m_loaded = false;    // блокировка markDirty при начальной загрузке

  void loadFromFile();
  // Преобразует [[Name]] → [Name](wiki:<percent-encoded>) в тексте.
  static QString injectWikiLinks(const QString &markdown);
};

#endif // NOTEEDITOR_H
