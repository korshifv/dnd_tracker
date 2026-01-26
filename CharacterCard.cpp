#include "CharacterCard.h"
#include "CharacterSheet.h"
#include "FileUtils.h"
#include <QDrag>
#include <QFileDialog>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSpinBox>
#include <QVBoxLayout>

CharacterCard::CharacterCard(QWidget *parent) : QFrame(parent) {
  // Оптимизация: делаем фон прозрачным, чтобы избежать черных углов при
  // скруглении границ
  setAttribute(Qt::WA_TranslucentBackground);
  setObjectName("CharacterCard");

  // Централизованная настройка стилей для элементов внутри карточки
  // QFrame#CharacterCard - контейнер всей карточки (прозрачный)
  // QFrame#cardBody - видимая часть с белым фоном и рамкой
  setStyleSheet("QFrame#CharacterCard { border: none; }"
                "QLineEdit { color: palette(text); background-color: "
                "palette(base); border: 1px solid palette(mid); border-radius: "
                "4px; font-weight: bold; padding: 2px; }"
                "QSpinBox { color: palette(text); background-color: "
                "palette(base); border: 1px solid palette(mid); border-radius: "
                "4px; padding: 2px; font-weight: bold; }"
                "QPushButton { color: palette(button-text); background-color: "
                "palette(button); border: 1px solid palette(mid); "
                "border-radius: 4px; font-weight: bold; }");
  setFixedHeight(185);

  opacityEffect = new QGraphicsOpacityEffect(this);
  setGraphicsEffect(opacityEffect);

  auto *rootLayout = new QVBoxLayout(this);
  rootLayout->setContentsMargins(5, 5, 5, 5);

  // ВНУТРЕННИЙ КОРПУС (те самые рамки)
  QFrame *body = new QFrame(this);
  body->setObjectName("cardBody");
  body->setStyleSheet("QFrame#cardBody { border: 2px solid palette(mid); "
                      "border-radius: 12px; background: palette(window); "
                      "color: palette(window-text); }");

  auto *mainLayout = new QVBoxLayout(body);
  mainLayout->setContentsMargins(12, 10, 12, 10);
  mainLayout->setSpacing(8);

  // --- ВЕРХНИЙ РЯД: Аватар, Инициатива, Имя, LSS ---
  auto *topLayout = new QHBoxLayout();

  QPushButton *avatarBtn = new QPushButton();
  avatarBtn->setFixedSize(32, 32);
  setRandomColor(avatarBtn); // Рандомный цвет кружка
  connect(avatarBtn, &QPushButton::clicked, this,
          &CharacterCard::showFullSheet);

  initSpin = new QSpinBox();
  initSpin->setRange(-20, 99);
  initSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
  initSpin->setFixedWidth(35);
  initSpin->setToolTip("Инициатива");

  nameEdit = new QLineEdit("Персонаж");

  QPushButton *lssBtn = new QPushButton("LSS");
  lssBtn->setFixedSize(32, 22);
  lssBtn->setStyleSheet(
      "font-size: 9px; border: 1px solid palette(mid); color: "
      "palette(button-text); background-color: palette(button);");
  lssBtn->setToolTip("Импортировать JSON из LSS");
  connect(lssBtn, &QPushButton::clicked, this, &CharacterCard::openLssFile);

  auto *delBtn = new QPushButton("×");
  delBtn->setFixedSize(22, 22);
  delBtn->setStyleSheet("border: none; font-size: 18px; font-weight: bold; "
                        "background: transparent;");
  connect(delBtn, &QPushButton::clicked, this, &CharacterCard::deleteLater);

  topLayout->addWidget(avatarBtn);
  topLayout->addWidget(new QLabel("In:"));
  topLayout->addWidget(initSpin);
  topLayout->addWidget(nameEdit);
  topLayout->addWidget(lssBtn);
  topLayout->addWidget(delBtn);
  mainLayout->addLayout(topLayout);

  // --- СРЕДНИЙ РЯД: Калькулятор ХП + КД ---
  auto *calcLayout = new QHBoxLayout();

  hpSpin = new QSpinBox();
  hpSpin->setRange(-99, 999);
  hpSpin->setValue(10);
  hpSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
  hpSpin->setFixedWidth(50);

  acLabel = new QLabel("КД: --");
  acLabel->setStyleSheet(
      "font-weight: bold; border: 1px solid palette(mid); "
      "border-radius: 4px; padding: 2px 4px; color: palette(window-text);");

  modSpin = new QSpinBox();
  modSpin->setRange(0, 500);
  modSpin->setFixedWidth(40);
  modSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);

  auto *dmgBtn = new QPushButton("-");
  dmgBtn->setFixedSize(28, 28);
  connect(dmgBtn, &QPushButton::clicked, this, &CharacterCard::applyDamage);

  auto *healBtn = new QPushButton("+");
  healBtn->setFixedSize(28, 28);
  connect(healBtn, &QPushButton::clicked, this, &CharacterCard::applyHeal);

  calcLayout->addWidget(new QLabel("HP:"));
  calcLayout->addWidget(hpSpin);
  calcLayout->addWidget(acLabel);
  calcLayout->addStretch();
  calcLayout->addWidget(new QLabel("+/-:"));
  calcLayout->addWidget(modSpin);
  calcLayout->addWidget(dmgBtn);
  calcLayout->addWidget(healBtn);
  mainLayout->addLayout(calcLayout);

  // --- НИЖНИЙ РЯД: Статусы ---
  statusEdit = new QLineEdit();
  statusEdit->setPlaceholderText("Статусы...");
  statusEdit->setStyleSheet("font-style: italic; font-weight: normal;");
  mainLayout->addWidget(statusEdit);

  rootLayout->addWidget(body);
  animateAppearance(); // Плавное всплытие при создании
}

void CharacterCard::openLssFile() {
  QString path = QFileDialog::getOpenFileName(this, "Выбрать чарник LSS", "",
                                              "JSON Files (*.json)");
  if (path.isEmpty())
    return;

  QString newPath = FileUtils::copyToData(path);

  QFile file(newPath);
  if (file.open(QIODevice::ReadOnly)) {
    loadLssJson(file.readAll());
    setFilePath(newPath); // Сохраняем путь к локальной копии
  }
}

// Загрузка данных из JSON файла LSS
void CharacterCard::loadLssJson(const QByteArray &rawData) {
  QJsonDocument doc = QJsonDocument::fromJson(rawData);
  if (doc.isNull()) {
    QMessageBox::warning(this, "Ошибка",
                         "Не удалось распарсить JSON файл персонажа.");
    return;
  }

  rootLssJson = doc.object(); // Сохраняем весь корневой объект файла LSS

  // Специфика формата LSS: основные данные хранятся внутри строкового поля
  // "data" Необходимо извлечь эту строку и распарсить её как отдельный JSON
  // документ
  QString innerJsonStr = rootLssJson["data"].toString();
  QJsonDocument innerDoc = QJsonDocument::fromJson(innerJsonStr.toUtf8());
  if (innerDoc.isNull())
    return;

  characterData = innerDoc.object();

  // Заполняем поля карточки
  nameEdit->setText(characterData["name"].toObject()["value"].toString());

  QJsonObject vitality = characterData["vitality"].toObject();
  hpSpin->setValue(vitality["hp-current"].toObject()["value"].toInt());

  int ac = vitality["ac"].toObject()["value"].toInt();
  acLabel->setText(QString("КД: %1").arg(ac));

  int init = vitality["initiative"].toObject()["value"].toInt();
  if (init != 0)
    initSpin->setValue(init);
}

void CharacterCard::showFullSheet() {
  if (characterData.isEmpty()) {
    QMessageBox::warning(this, "Внимание",
                         "Сначала импортируйте JSON файл персонажа!");
    return;
  }
  // Открываем диалог, передавая указатель на текущую карточку как родителя
  // Передаем полную структуру (root) и распарсенные данные (characterData)
  CharacterSheet *sheet = new CharacterSheet(rootLssJson, characterData, this);
  sheet->show();
}

void CharacterCard::setRandomColor(QWidget *widget) {
  int r = QRandomGenerator::global()->bounded(160, 256);
  int g = QRandomGenerator::global()->bounded(160, 256);
  int b = QRandomGenerator::global()->bounded(160, 256);
  widget->setStyleSheet(
      QString("border: 2px solid palette(text); border-radius: 16px; "
              "background-color: rgb(%1,%2,%3);")
          .arg(r)
          .arg(g)
          .arg(b));
}

void CharacterCard::animateAppearance() {
  QPropertyAnimation *anim = new QPropertyAnimation(opacityEffect, "opacity");
  anim->setDuration(300);
  anim->setStartValue(0.0);
  anim->setEndValue(1.0);
  anim->start(QAbstractAnimation::DeleteWhenStopped);
}

int CharacterCard::getInitiative() const { return initSpin->value(); }

QString CharacterCard::getFilePath() const { return currentFilePath; }

void CharacterCard::setFilePath(const QString &path) { currentFilePath = path; }

void CharacterCard::applyDamage() {
  hpSpin->setValue(hpSpin->value() - modSpin->value());
  modSpin->setValue(0);
}

void CharacterCard::applyHeal() {
  hpSpin->setValue(hpSpin->value() + modSpin->value());
  modSpin->setValue(0);
}

// Обработчик нажатия мыши
void CharacterCard::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    // Сохраняем начальную позицию для старта Drag & Drop
    event->accept();
  } else {
    QFrame::mousePressEvent(event);
  }
}

void CharacterCard::mouseMoveEvent(QMouseEvent *event) {
  if (!(event->buttons() & Qt::LeftButton))
    return;

  QDrag *drag = new QDrag(this);
  QMimeData *mimeData = new QMimeData;

  // Передаем указатель на текущую карточку как MIME-данные
  mimeData->setData("application/x-charactercard",
                    QByteArray::number((qintptr)this));

  drag->setMimeData(mimeData);

  // Можно добавить "фантомное" изображение карточки при перетаскивании
  drag->setPixmap(grab());
  drag->setHotSpot(event->position().toPoint());

  drag->exec(Qt::MoveAction);
}

CharacterCard::~CharacterCard() {}