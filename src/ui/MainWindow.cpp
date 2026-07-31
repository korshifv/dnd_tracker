#include "MainWindow.h"
#include "CharacterCard.h"
#include "CharacterRepository.h"
#include "CharacterSheet.h"
#include "InitiativeTracker.h"
#include "NoteRepository.h"
#include "NoteEditor.h"
#include "Storage.h"
#include "CharacterDocument.h"
#include <QCloseEvent>
#include <QFile>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTabWidget>
#include <QVBoxLayout>

#include "TouchUtils.h"
#include <QTabBar>
#include <QGuiApplication>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
  setWindowTitle("D&D Tracker");

#ifdef Q_OS_ANDROID
  showMaximized();
#else
  resize(1300, 900);
#endif

  auto *root = new QVBoxLayout(this);
  root->setContentsMargins(0, 0, 0, 0);
  root->setSpacing(0);

  tabs = new QTabWidget(this);
  tabs->setTabsClosable(true);
  tabs->setMovable(true);

  // На Android / мобилках перемещаем панель навигации ВНИЗ (Bottom Bar) для удобства пальцев
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
  tabs->setTabPosition(QTabWidget::South);
#else
  // На десктопе проверяем ширину экрана: если маленький — переставляем вниз
  if (QGuiApplication::primaryScreen() && QGuiApplication::primaryScreen()->size().width() < 768) {
    tabs->setTabPosition(QTabWidget::South);
  }
#endif

  connect(tabs, &QTabWidget::tabCloseRequested, this,
          &MainWindow::onTabCloseRequested);

  // Вкладка 0: трекер инициативы.
  tracker = new InitiativeTracker(this);
  tabs->addTab(tracker, "⚔ Инициатива");
  connect(tracker, &InitiativeTracker::sheetRequested, this,
          &MainWindow::openCharacterSheet);
  connect(tracker, &InitiativeTracker::requestDocumentBinding, this,
          [this](CharacterCard *card, const QString &filePath) {
            if (auto *doc = getDocument(filePath)) {
              card->setDocument(doc);
            }
          });

  // Вкладка 1: хранилище персонажей.
  repository = new CharacterRepository(this);
  tabs->addTab(repository, "📜 Чарники");
  connect(repository, &CharacterRepository::openRequested, this,
          &MainWindow::openCharacterSheet);

  // Вкладка 2: заметки (Obsidian-style wiki-links).
  noteRepo = new NoteRepository(this);
  tabs->addTab(noteRepo, "📝 Заметки");
  connect(noteRepo, &NoteRepository::requestOpenCharacter, this,
          [this](const QString &charName) {
            QString path = CharacterRepository::filePathForName(charName);
            if (!path.isEmpty())
              openCharacterSheet(path);
          });

  // Убираем мелкие крестики закрытия у фиксированных вкладок (0, 1, 2)
  for (int i = 0; i < FIXED_TAB_COUNT; ++i) {
    tabs->tabBar()->setTabButton(i, QTabBar::RightSide, nullptr);
    tabs->tabBar()->setTabButton(i, QTabBar::LeftSide, nullptr);
  }

  // Включаем тач-скроллинг
  TouchUtils::enableTouchScroll(tabs);

  root->addWidget(tabs);
}

CharacterDocument* MainWindow::getDocument(const QString &filePath) {
  if (m_documents.contains(filePath))
    return m_documents.value(filePath);

  auto *doc = new CharacterDocument(this);
  if (!doc->load(filePath)) {
    delete doc;
    return nullptr;
  }
  m_documents.insert(filePath, doc);
  return doc;
}

void MainWindow::openCharacterSheet(const QString &filePath) {
  if (filePath.isEmpty())
    return;

  CharacterDocument *doc = getDocument(filePath);
  if (!doc) return;

  // Заголовок вкладки — имя персонажа.
  QString title = doc->getName();
  if (title.isEmpty())
    title = "Чарник";

  openSheetTab(filePath, title);
}

void MainWindow::openSheetTab(const QString &filePath, const QString &title) {
  // Дедупликация: динамический поиск вкладки.
  for (int i = FIXED_TAB_COUNT; i < tabs->count(); ++i) {
    if (auto *sheet = qobject_cast<CharacterSheet*>(tabs->widget(i))) {
      if (sheet->getFilePath() == filePath) {
        tabs->setCurrentIndex(i);
        return;
      }
    }
  }

  CharacterDocument *doc = getDocument(filePath);
  if (!doc) return;

  auto *sheet = new CharacterSheet(doc, this);
  // После сохранения перезагружаем карточки трекера с этим filePath (фикс #2).
  connect(sheet, &CharacterSheet::saved, tracker,
          &InitiativeTracker::reloadCardsForFile);

  const int idx = tabs->addTab(sheet, title);
  tabs->setCurrentIndex(idx);
}

void MainWindow::onTabCloseRequested(int index) {
  // Защита незакрываемых вкладок (Инициатива, Чарники, Заметки).
  if (index < FIXED_TAB_COUNT)
    return;

  QWidget *w = tabs->widget(index);
  if (auto *sheet = qobject_cast<CharacterSheet*>(w)) {
    sheet->flushSave();
    
    // Если закрыта последняя вкладка с этим документом, можно освободить память
    // Но так как у нас может быть карточка инициативы, ссылающаяся на него,
    // мы пока оставим кэш на совести MainWindow (освободится при закрытии приложения),
    // или можно реализовать счетчик ссылок.
  }

  tabs->removeTab(index);
  delete w;
}

void MainWindow::closeEvent(QCloseEvent *event) {
  // FlushSave всех открытых чарников
  for (int i = FIXED_TAB_COUNT; i < tabs->count(); ++i) {
    if (auto *sheet = qobject_cast<CharacterSheet*>(tabs->widget(i))) {
      sheet->flushSave();
    }
  }

  // Также flushSave для NoteRepository (который теперь содержит NoteEditor)
  noteRepo->flushSave();

  // InitiativeTracker встроен во вкладку — сохраняем состояние явно.
  tracker->save();
  event->accept();
}

MainWindow::~MainWindow() {}
