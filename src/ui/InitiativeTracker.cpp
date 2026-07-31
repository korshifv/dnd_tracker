#include "InitiativeTracker.h"
#include "CharacterCard.h"
#include "CharacterDocument.h"
#include "Storage.h"
#include "TrackerColumn.h"
#include "TouchUtils.h"
#include <QDir>
#include <QFile>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>

InitiativeTracker::InitiativeTracker(QWidget *parent) : QWidget(parent) {
  m_saveTimer = new QTimer(this);
  m_saveTimer->setSingleShot(true);
  m_saveTimer->setInterval(1500);
  connect(m_saveTimer, &QTimer::timeout, this, [this]() { saveState(); });

  auto *root = new QVBoxLayout(this);
  root->setContentsMargins(10, 10, 10, 10);
  root->setSpacing(10);

  // === ⚔️ ПАНЕЛЬ УПРАВЛЕНИЯ БОЕМ (COMBAT BANNER) ===
  QFrame *banner = new QFrame(this);
  banner->setObjectName("combatBanner");
  banner->setStyleSheet(
      "QFrame#combatBanner { background-color: #1E1E28; border: 2px solid #8C62FF; "
      "border-radius: 12px; padding: 10px; }"
      "QLabel#turnText { font-size: 18px; font-weight: bold; color: #FFFFFF; }"
      "QLabel#roundText { font-size: 14px; font-weight: 600; color: #A57BFF; }");

  auto *bannerLayout = new QHBoxLayout(banner);
  bannerLayout->setContentsMargins(10, 6, 10, 6);
  bannerLayout->setSpacing(12);

  m_turnLabel = new QLabel("⚔️ БОЙ НЕ НАЧАТ", banner);
  m_turnLabel->setObjectName("turnText");

  m_roundLabel = new QLabel("Раунд: 1", banner);
  m_roundLabel->setObjectName("roundText");

  m_nextTurnBtn = new QPushButton("▶ СЛЕДУЮЩИЙ ХОД", banner);
  m_nextTurnBtn->setObjectName("primaryBtn");
  m_nextTurnBtn->setMinimumHeight(44);
  m_nextTurnBtn->setStyleSheet(
      "QPushButton#primaryBtn { background-color: #8C62FF; color: white; "
      "font-size: 15px; font-weight: bold; border-radius: 8px; padding: 8px 16px; }"
      "QPushButton#primaryBtn:hover { background-color: #9D75FF; }");
  connect(m_nextTurnBtn, &QPushButton::clicked, this, &InitiativeTracker::nextTurn);

  auto *resetBtn = new QPushButton("↻ Сброс", banner);
  resetBtn->setMinimumHeight(40);
  connect(resetBtn, &QPushButton::clicked, this, &InitiativeTracker::resetCombat);

  bannerLayout->addWidget(m_turnLabel, 1);
  bannerLayout->addWidget(m_roundLabel);
  bannerLayout->addWidget(m_nextTurnBtn);
  bannerLayout->addWidget(resetBtn);

  root->addWidget(banner);

  // === ВЕРХНИЕ КНОПКИ УПРАВЛЕНИЯ ГРУППАМИ ===
  auto *topPanel = new QHBoxLayout();

  auto *addColBtn = new QPushButton("+ Добавить группу", this);
  addColBtn->setFixedHeight(40);
  connect(addColBtn, &QPushButton::clicked, this, &InitiativeTracker::addColumn);

  auto *sortAllBtn = new QPushButton("⇅ Сортировать всё", this);
  sortAllBtn->setFixedHeight(40);
  connect(sortAllBtn, &QPushButton::clicked, this, &InitiativeTracker::sortAllColumns);

  auto *clearBtn = new QPushButton("Очистить все");
  clearBtn->setFixedHeight(40);
  connect(clearBtn, &QPushButton::clicked, this, &InitiativeTracker::clearAllData);

  topPanel->addWidget(addColBtn);
  topPanel->addWidget(sortAllBtn);
  topPanel->addStretch();
  topPanel->addWidget(clearBtn);

  root->addLayout(topPanel);

  // === ОБЛАСТЬ СИНХРОННОГО СКРОЛЛИНГА КОЛОНОК ===
  auto *scroll = new QScrollArea(this);
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);
  scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  TouchUtils::enableTouchScroll(scroll);

  QWidget *c = new QWidget();
  columnsLayout = new QHBoxLayout(c);
  columnsLayout->setAlignment(Qt::AlignLeft);
  columnsLayout->setContentsMargins(0, 0, 0, 0);
  columnsLayout->setSpacing(12);
  scroll->setWidget(c);

  root->addWidget(scroll);

  loadState();

  if (columnsLayout->count() == 0) {
    addColumn();
  }
}

QList<CharacterCard *> InitiativeTracker::getAllCardsSorted() const {
  QList<CharacterCard *> allCards;
  for (int i = 0; i < columnsLayout->count(); ++i) {
    auto *col = qobject_cast<TrackerColumn *>(columnsLayout->itemAt(i)->widget());
    if (col) {
      allCards.append(col->getCards());
    }
  }

  // Сортировка по убыванию инициативы
  std::sort(allCards.begin(), allCards.end(), [](CharacterCard *a, CharacterCard *b) {
    return a->getInitiative() > b->getInitiative();
  });

  return allCards;
}

void InitiativeTracker::sortAllColumns() {
  for (int i = 0; i < columnsLayout->count(); ++i) {
    auto *col = qobject_cast<TrackerColumn *>(columnsLayout->itemAt(i)->widget());
    if (col) {
      col->sortInitiative();
    }
  }
  updateTurnBanner();
}

void InitiativeTracker::nextTurn() {
  QList<CharacterCard *> cards = getAllCardsSorted();
  if (cards.isEmpty()) {
    m_turnLabel->setText("⚔️ БОЙ НЕ НАЧАТ (нет бойцов)");
    return;
  }

  m_currentTurnIndex++;
  if (m_currentTurnIndex >= cards.size()) {
    m_currentTurnIndex = 0;
    m_roundCount++;
  }

  updateTurnBanner();
}

void InitiativeTracker::resetCombat() {
  m_roundCount = 1;
  m_currentTurnIndex = -1;
  updateTurnBanner();
}

void InitiativeTracker::updateTurnBanner() {
  QList<CharacterCard *> cards = getAllCardsSorted();
  
  m_roundLabel->setText(QString("Раунд: %1").arg(m_roundCount));

  if (cards.isEmpty() || m_currentTurnIndex < 0 || m_currentTurnIndex >= cards.size()) {
    m_turnLabel->setText("⚔️ НАЖМИТЕ «СЛЕДУЮЩИЙ ХОД»");
    return;
  }

  CharacterCard *activeCard = cards.at(m_currentTurnIndex);
  QString name = activeCard->getName();
  if (name.isEmpty()) name = "Безымянный";
  int init = activeCard->getInitiative();

  m_turnLabel->setText(QString("⚔️ СЕЙЧАС ХОДИТ: %1 (Инициатива: %2)").arg(name).arg(init));
}

void InitiativeTracker::addColumn() {
  auto *col = new TrackerColumn(
      "Группа " + QString::number(columnsLayout->count() + 1), this);
  connect(col, &TrackerColumn::sheetRequested, this, &InitiativeTracker::sheetRequested);
  connect(col, &TrackerColumn::documentRequested, this, &InitiativeTracker::requestDocumentBinding);
  connect(col, &TrackerColumn::contentsChanged, this, &InitiativeTracker::scheduleSave);
  columnsLayout->addWidget(col);
  emit scheduleSave();
}

void InitiativeTracker::scheduleSave() {
  m_saveTimer->start();
}

void InitiativeTracker::saveState() {
  QJsonObject state;
  QJsonArray columnsArr;

  for (int i = 0; i < columnsLayout->count(); ++i) {
    auto *col = qobject_cast<TrackerColumn *>(columnsLayout->itemAt(i)->widget());
    if (!col) continue;

    QJsonObject colObj;
    colObj["title"] = col->getTitle();

    QJsonArray cardsArr;
    for (auto *card : col->getCards()) {
      QJsonObject cardObj;
      if (card->getDocument()) {
        cardObj["path"] = card->getDocument()->getFilePath();
      } else {
        cardObj["path"] = card->property("currentFilePath").toString();
      }
      cardObj["state"] = card->getEphemeralState();
      cardsArr.append(cardObj);
    }
    colObj["cards"] = cardsArr;
    columnsArr.append(colObj);
  }
  state["columns"] = columnsArr;
  state["roundCount"] = m_roundCount;
  state["currentTurnIndex"] = m_currentTurnIndex;

  QFile file(Storage::stateFilePath());
  if (file.open(QIODevice::WriteOnly)) {
    file.write(QJsonDocument(state).toJson());
  }
}

void InitiativeTracker::loadState() {
  QFile file(Storage::stateFilePath());
  if (!file.open(QIODevice::ReadOnly))
    return;

  QJsonParseError error;
  QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
  if (doc.isNull()) return;

  QJsonObject state = doc.object();
  m_roundCount = state["roundCount"].toInt(1);
  m_currentTurnIndex = state["currentTurnIndex"].toInt(-1);

  QJsonArray columnsArr = state["columns"].toArray();

  for (const auto &colVal : columnsArr) {
    QJsonObject colObj = colVal.toObject();
    auto *col = new TrackerColumn(colObj["title"].toString(), this);
    connect(col, &TrackerColumn::sheetRequested, this, &InitiativeTracker::sheetRequested);
    connect(col, &TrackerColumn::documentRequested, this, &InitiativeTracker::requestDocumentBinding);
    connect(col, &TrackerColumn::contentsChanged, this, &InitiativeTracker::scheduleSave);
    columnsLayout->addWidget(col);

    QJsonArray cardsArr = colObj["cards"].toArray();
    for (const auto &cardVal : cardsArr) {
      QJsonObject cardObj = cardVal.toObject();
      QString path = cardObj["path"].toString();
      QJsonObject ephemeralState = cardObj["state"].toObject();
      col->insertCharacter(-1, path, ephemeralState);
    }
  }

  updateTurnBanner();
}

void InitiativeTracker::clearAllData() {
  if (QMessageBox::question(this, "Очистка данных",
                            "Вы уверены? Все сохраненные состояния и локальные "
                            "файлы персонажей будут удалены.") != QMessageBox::Yes) {
    return;
  }

  QFile::remove(Storage::stateFilePath());

  QDir dir(Storage::charactersDir());
  if (dir.exists()) {
    dir.removeRecursively();
    dir.mkpath(".");
  }

  QLayoutItem *child;
  while ((child = columnsLayout->takeAt(0)) != nullptr) {
    if (child->widget()) delete child->widget();
    delete child;
  }

  m_roundCount = 1;
  m_currentTurnIndex = -1;

  addColumn();
  updateTurnBanner();
}

void InitiativeTracker::save() {
  m_saveTimer->stop();
  saveState();
}

void InitiativeTracker::reloadCardsForFile(const QString &filePath) {
  for (int i = 0; i < columnsLayout->count(); ++i) {
    auto *col = qobject_cast<TrackerColumn *>(columnsLayout->itemAt(i)->widget());
    if (!col) continue;
    for (auto *card : col->getCards()) {
      if (card->getDocument() && card->getDocument()->getFilePath() == filePath)
        card->reloadFromDocument();
    }
  }
}

InitiativeTracker::~InitiativeTracker() {}
