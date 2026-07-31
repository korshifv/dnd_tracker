#include "MainWindow.h"
#include "CharacterCard.h"
#include "CharacterRepository.h"
#include "CharacterSheet.h"
#include "InitiativeTracker.h"
#include "NoteRepository.h"
#include "NoteEditor.h"
#include "Storage.h"
#include "CharacterDocument.h"
#include "TouchUtils.h"

#include <QCloseEvent>
#include <QFile>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTabBar>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGuiApplication>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
  setWindowTitle("D&D Tracker");

#ifdef Q_OS_ANDROID
  showMaximized();
#else
  resize(1200, 800);
#endif

  auto *rootLayout = new QVBoxLayout(this);
  rootLayout->setContentsMargins(0, 0, 0, 0);
  rootLayout->setSpacing(0);

  m_stack = new QStackedWidget(this);

  // === СТРАНИЦА 0: Главный контейнер с 3 кнопками навигации ===
  QWidget *mainPage = new QWidget(this);
  auto *mainLayout = new QVBoxLayout(mainPage);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  tabs = new QTabWidget(mainPage);
  tabs->setTabsClosable(false);
  tabs->setMovable(false);

  // Перемещаем панель навигации вниз на мобильных устройствах
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
  tabs->setTabPosition(QTabWidget::South);
#else
  if (QGuiApplication::primaryScreen() && QGuiApplication::primaryScreen()->size().width() < 768) {
    tabs->setTabPosition(QTabWidget::South);
  }
#endif

  // Вкладка 0: Инициатива
  tracker = new InitiativeTracker(this);
  tabs->addTab(tracker, "Инициатива");
  connect(tracker, &InitiativeTracker::sheetRequested, this, &MainWindow::openCharacterSheet);
  connect(tracker, &InitiativeTracker::requestDocumentBinding, this,
          [this](CharacterCard *card, const QString &filePath) {
            if (auto *doc = getDocument(filePath)) {
              card->setDocument(doc);
            }
          });

  // Вкладка 1: Чарники
  repository = new CharacterRepository(this);
  tabs->addTab(repository, "Чарники");
  connect(repository, &CharacterRepository::openRequested, this, &MainWindow::openCharacterSheet);

  // Вкладка 2: Заметки
  noteRepo = new NoteRepository(this);
  tabs->addTab(noteRepo, "Заметки");
  connect(noteRepo, &NoteRepository::requestOpenCharacter, this,
          [this](const QString &charName) {
            QString path = CharacterRepository::filePathForName(charName);
            if (!path.isEmpty())
              openCharacterSheet(path);
          });

  TouchUtils::enableTouchScroll(tabs);
  mainLayout->addWidget(tabs);
  m_stack->addWidget(mainPage); // Стек 0

  // === СТРАНИЦА 1: Полноэкранный просмотрщик чарника ===
  m_sheetContainer = new QWidget(this);
  auto *sheetContainerLayout = new QVBoxLayout(m_sheetContainer);
  sheetContainerLayout->setContentsMargins(0, 0, 0, 0);
  sheetContainerLayout->setSpacing(0);

  // Верхний навигационный бары чарника
  QWidget *topBarWidget = new QWidget(m_sheetContainer);
  topBarWidget->setStyleSheet("background-color: #1A1A24; border-bottom: 1px solid #2A2A38;");
  auto *topBarLayout = new QHBoxLayout(topBarWidget);
  topBarLayout->setContentsMargins(10, 8, 10, 8);

  QPushButton *backBtn = new QPushButton("← Назад", topBarWidget);
  backBtn->setMinimumHeight(42);
  backBtn->setStyleSheet(
      "QPushButton { background-color: #262634; color: white; font-weight: bold; "
      "border-radius: 8px; padding: 6px 14px; }"
      "QPushButton:hover { background-color: #36364A; }");
  connect(backBtn, &QPushButton::clicked, this, &MainWindow::closeCharacterSheet);

  m_sheetTitleLabel = new QLabel("Персонаж", topBarWidget);
  m_sheetTitleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #A57BFF;");

  topBarLayout->addWidget(backBtn);
  topBarLayout->addWidget(m_sheetTitleLabel, 1, Qt::AlignCenter);
  topBarLayout->addSpacing(60); // Балансировка ширины

  sheetContainerLayout->addWidget(topBarWidget);

  m_stack->addWidget(m_sheetContainer); // Стек 1

  rootLayout->addWidget(m_stack);
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
  if (filePath.isEmpty()) return;

  CharacterDocument *doc = getDocument(filePath);
  if (!doc) return;

  // Очищаем предыдущий открытый активный чарник
  if (m_activeSheet) {
    m_activeSheet->flushSave();
    m_activeSheet->deleteLater();
    m_activeSheet = nullptr;
  }

  // Создаем полноэкранный чарник
  m_activeSheet = new CharacterSheet(doc, m_sheetContainer);
  connect(m_activeSheet, &CharacterSheet::saved, tracker, &InitiativeTracker::reloadCardsForFile);

  m_sheetContainer->layout()->addWidget(m_activeSheet);

  QString title = doc->getName();
  if (title.isEmpty()) title = "Персонаж";
  m_sheetTitleLabel->setText(title);

  // Переключаемся на полноэкранный просмотр (Стек 1)
  m_stack->setCurrentIndex(1);
}

void MainWindow::closeCharacterSheet() {
  if (m_activeSheet) {
    m_activeSheet->flushSave();
  }
  // Возвращаемся на главный экран (Стек 0)
  m_stack->setCurrentIndex(0);
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if (m_activeSheet) {
    m_activeSheet->flushSave();
  }
  noteRepo->flushSave();
  tracker->save();
  event->accept();
}

MainWindow::~MainWindow() {}
