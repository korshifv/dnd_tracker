#include "InitiativeTracker.h"
#include "CharacterCard.h"
#include "CharacterDocument.h"
#include "Storage.h"
#include "TrackerColumn.h"
#include <QDir>
#include <QFile>
#include <QFont>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>

// Конструктор вкладки инициативы
InitiativeTracker::InitiativeTracker(QWidget *parent) : QWidget(parent) {
  // Debounce-таймер: после любого изменения состояния трекера ждём 1500мс
  // тишины и сохраняем. Если за это время произошло ещё одно изменение —
  // таймер перезапускается.
  m_saveTimer = new QTimer(this);
  m_saveTimer->setSingleShot(true);
  m_saveTimer->setInterval(1500);
  connect(m_saveTimer, &QTimer::timeout, this, [this]() { saveState(); });

  // Основной вертикальный лейаут
  auto *root = new QVBoxLayout(this);

  // Верхняя панель с кнопками
  auto *topPanel = new QHBoxLayout();

  // Кнопка создания новой группы (колонки)
  auto *btn = new QPushButton("+ Создать новую группу", this);
  btn->setFixedHeight(38);
  QFont btnFont = btn->font();
  btnFont.setBold(true);
  btn->setFont(btnFont);
  connect(btn, &QPushButton::clicked, this, &InitiativeTracker::addColumn);

  // Кнопка очистки данных
  auto *clearBtn = new QPushButton("Очистить все");
  clearBtn->setFixedHeight(38);
  clearBtn->setFont(btnFont);
  connect(clearBtn, &QPushButton::clicked, this,
          &InitiativeTracker::clearAllData);

  topPanel->addWidget(btn);
  topPanel->addStretch();
  topPanel->addWidget(clearBtn);

  // Область прокрутки для колонок, чтобы они помещались если их много
  auto *s = new QScrollArea(this);
  s->setWidgetResizable(true);       // Контент внутри растягивается
  s->setFrameShape(QFrame::NoFrame); // Без рамок

  // Контейнер для колонок
  QWidget *c = new QWidget();
  columnsLayout = new QHBoxLayout(c);
  columnsLayout->setAlignment(Qt::AlignLeft); // Колонки прижимаются влево
  s->setWidget(c);

  // Добавляем элементы в главный лейаут
  root->addLayout(topPanel);
  root->addWidget(s);

  // Загружаем сохраненное состояние
  loadState();

  // Если ничего не загрузилось, создаем одну пустую колонку
  if (columnsLayout->count() == 0) {
    addColumn();
  }
}

// Добавляет новую колонку трекера в интерфейс
void InitiativeTracker::addColumn() {
  auto *col = new TrackerColumn(
      "Группа " + QString::number(columnsLayout->count() + 1), this);
  // Пробрасываем запрос чарника от карточек наверх к MainWindow.
  connect(col, &TrackerColumn::sheetRequested, this,
          &InitiativeTracker::sheetRequested);
  connect(col, &TrackerColumn::documentRequested, this,
          &InitiativeTracker::requestDocumentBinding);
  // Любое изменение содержимого колонки → debounce-сохранение.
  connect(col, &TrackerColumn::contentsChanged, this,
          &InitiativeTracker::scheduleSave);
  columnsLayout->addWidget(col);
  emit scheduleSave(); // Новая колонка — это тоже изменение состояния
}

void InitiativeTracker::scheduleSave() {
  // Перезапуск одноразового таймера (если уже тикает — сбросит и начнёт заново).
  m_saveTimer->start();
}

void InitiativeTracker::saveState() {
  QJsonObject state;
  QJsonArray columnsArr;

  for (int i = 0; i < columnsLayout->count(); ++i) {
    auto *col =
        qobject_cast<TrackerColumn *>(columnsLayout->itemAt(i)->widget());
    if (!col)
      continue;

    QJsonObject colObj;
    colObj["title"] = col->getTitle();

    QJsonArray cardsArr;
    for (auto *card : col->getCards()) {
      QJsonObject cardObj;
      // Мы сохраняем карточку в любом случае, даже если у нее нет привязанного файла
      // (например, это созданный вручную монстр с эфемеральным стейтом).
      if (card->getDocument()) {
        cardObj["path"] = card->getDocument()->getFilePath();
      } else {
        cardObj["path"] = card->property("currentFilePath").toString(); // fallback
      }
      cardObj["state"] = card->getEphemeralState();
      cardsArr.append(cardObj);
    }
    colObj["cards"] = cardsArr;
    columnsArr.append(colObj);
  }
  state["columns"] = columnsArr;

  // Фикс #6: пишем в AppData, а не в CWD.
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
  if (doc.isNull()) {
    QMessageBox::warning(this, "Ошибка загрузки",
                         "Не удалось прочитать файл состояния: " +
                             error.errorString());
    return;
  }

  QJsonObject state = doc.object();
  QJsonArray columnsArr = state["columns"].toArray();

  for (const auto &colVal : columnsArr) {
    QJsonObject colObj = colVal.toObject();
    auto *col = new TrackerColumn(colObj["title"].toString(), this);
    connect(col, &TrackerColumn::sheetRequested, this,
            &InitiativeTracker::sheetRequested);
    connect(col, &TrackerColumn::documentRequested, this,
            &InitiativeTracker::requestDocumentBinding);
    connect(col, &TrackerColumn::contentsChanged, this,
            &InitiativeTracker::scheduleSave);
    columnsLayout->addWidget(col);

    QJsonArray cardsArr = colObj["cards"].toArray();
    for (const auto &cardVal : cardsArr) {
      QJsonObject cardObj = cardVal.toObject();
      QString path = cardObj["path"].toString();
      QJsonObject ephemeralState = cardObj["state"].toObject();
      
      // Загружаем карточку через TrackerColumn
      col->insertCharacter(-1, path, ephemeralState);
    }
  }
}

void InitiativeTracker::clearAllData() {
  if (QMessageBox::question(this, "Очистка данных",
                            "Вы уверены? Все сохраненные состояния и локальные "
                            "файлы персонажей будут удалены.") !=
      QMessageBox::Yes) {
    return;
  }

  // Удаляем файл состояния
  QFile::remove(Storage::stateFilePath());

  // Удаляем папку персонажов
  QDir dir(Storage::charactersDir());
  if (dir.exists()) {
    dir.removeRecursively();
    dir.mkpath("."); // пересоздаём пустую
  }

  // Перезапускаем интерфейс (удаляем все колонки)
  QLayoutItem *child;
  while ((child = columnsLayout->takeAt(0)) != nullptr) {
    if (child->widget())
      delete child->widget();
    delete child;
  }

  // Добавляем одну пустую
  addColumn();
}

void InitiativeTracker::save() {
  // Останавливаем debounce-таймер и сохраняем немедленно.
  m_saveTimer->stop();
  saveState();
}

void InitiativeTracker::reloadCardsForFile(const QString &filePath) {
  // Перебираем все колонки и их карточки; перезагружаем те, что привязаны к
  // filePath. Фикс #2 — после сохранения чарника карточки обновляются.
  for (int i = 0; i < columnsLayout->count(); ++i) {
    auto *col =
        qobject_cast<TrackerColumn *>(columnsLayout->itemAt(i)->widget());
    if (!col)
      continue;
    for (auto *card : col->getCards()) {
      if (card->getDocument() && card->getDocument()->getFilePath() == filePath)
        card->reloadFromDocument();
    }
  }
}

InitiativeTracker::~InitiativeTracker() {}
