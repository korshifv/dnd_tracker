#include "MainWindow.h"
#include "CharacterCard.h"
#include "TrackerColumn.h"
#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QStandardPaths>
#include <QVBoxLayout>

// Конструктор главного окна
MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
  setWindowTitle("D&D Tracker");

  // Установка начального размера окна (1200x800)
  resize(1200, 800);

  // Основной вертикальный лейаут
  auto *root = new QVBoxLayout(this);

  // Верхняя панель с кнопками
  auto *topPanel = new QHBoxLayout();

  // Кнопка создания новой группы (колонки)
  auto *btn = new QPushButton("+ Создать новую группу", this);
  btn->setFixedHeight(45);
  // Применение стилей к кнопке (темный фон, белый текст, скругление)
  btn->setStyleSheet(
      "background: #333; color: white; border-radius: 8px; font-weight: bold;");
  connect(btn, &QPushButton::clicked, this, &MainWindow::addColumn);

  // Кнопка очистки данных
  auto *clearBtn = new QPushButton("Очистить все");
  clearBtn->setFixedHeight(45);
  clearBtn->setStyleSheet("background: #500; color: white; border-radius: 8px; "
                          "font-weight: bold; width: 120px;");
  connect(clearBtn, &QPushButton::clicked, this, &MainWindow::clearAllData);

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
void MainWindow::addColumn() {
  // Создаем новую колонку с заголовком "Группа N"
  // columnsLayout->count() + 1 используется для нумерации
  columnsLayout->addWidget(new TrackerColumn(
      "Группа " + QString::number(columnsLayout->count() + 1), this));
}

void MainWindow::closeEvent(QCloseEvent *event) {
  saveState();
  event->accept();
}

void MainWindow::saveState() {
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
      if (!card->getFilePath().isEmpty()) {
        QJsonObject cardObj;
        cardObj["path"] = card->getFilePath();
        cardsArr.append(cardObj);
      }
    }
    colObj["cards"] = cardsArr;
    columnsArr.append(colObj);
  }
  state["columns"] = columnsArr;

  QFile file("dnd_state.json");
  if (file.open(QIODevice::WriteOnly)) {
    file.write(QJsonDocument(state).toJson());
  }
}

void MainWindow::loadState() {
  QFile file("dnd_state.json");
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

  for (auto colVal : columnsArr) {
    QJsonObject colObj = colVal.toObject();
    auto *col = new TrackerColumn(colObj["title"].toString(), this);
    columnsLayout->addWidget(col);

    QJsonArray cardsArr = colObj["cards"].toArray();
    for (auto cardVal : cardsArr) {
      QJsonObject cardObj = cardVal.toObject();
      QString path = cardObj["path"].toString();

      // Если файл существует, добавляем его
      if (QFile::exists(path)) {
        // Мы должны получить доступ к методу добавления конкретного файла в
        // TrackerColumn Но у TrackerColumn есть только addCharacter(), который
        // создает пустую. Нам нужно немного "магии": создать пустую и загрузить
        // в нее файл.

        // TODO: В идеале в TrackerColumn добавить метод
        // addCharacterFromFile(path) Сейчас сделаем "хак" через доступ к
        // layout, так как TrackerColumn инкапсулирован Но лучше добавить
        // открытый метод в TrackerColumn. Учитывая текущую структуру, мы не
        // можем просто так вызвать loadLssJson снаружи. План: мы будем
        // "имитировать" добавление. Сделаем публичный метод в TrackerColumn:
        // loadCharacter(path)
        col->loadCharacter(path);
      }
    }
  }
}

void MainWindow::clearAllData() {
  if (QMessageBox::question(this, "Очистка данных",
                            "Вы уверены? Все сохраненные состояния и локальные "
                            "файлы персонажей будут удалены.") !=
      QMessageBox::Yes) {
    return;
  }

  // Удаляем файл состояния
  QFile::remove("dnd_state.json");

  // Удаляем папку data
  QDir dir("data");
  if (dir.exists()) {
    dir.removeRecursively();
  }

  // Перезапускаем интерфейс (удаляем все колонки)
  QLayoutItem *child;
  while ((child = columnsLayout->takeAt(0)) != 0) {
    if (child->widget())
      delete child->widget();
    delete child;
  }

  // Добавляем одну пустую
  addColumn();
}

MainWindow::~MainWindow() {}