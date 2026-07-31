#include "CharacterCard.h"
#include "Storage.h"
#include "CharacterDocument.h"
#include <QApplication>
#include <QColorDialog>
#include <QDrag>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSizePolicy>
#include <QSpinBox>
#include <QVBoxLayout>

CharacterCard::CharacterCard(QWidget *parent) : QFrame(parent) {
  // Плавающая карточка: без собственной рамки у контейнера, визуальный объём
  // задаёт внутреннее тело на palette(base) со скруглением. Никаких HEX.
  setObjectName("CharacterCard");
  setFrameShape(QFrame::NoFrame);
  setAutoFillBackground(false);
  // Минимальный scoped-стиль только для скругления и тонкой границы palette(mid).
  setStyleSheet("QFrame#cardBody { background-color: palette(base); "
                "border: 1px solid palette(mid); border-radius: 10px; }"
                "QLineEdit { border: 1px solid palette(mid); border-radius: 4px; "
                "padding: 2px; }"
                "QSpinBox { border: 1px solid palette(mid); border-radius: 4px; "
                "padding: 2px; }");
  setFixedHeight(185);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  opacityEffect = new QGraphicsOpacityEffect(this);
  setGraphicsEffect(opacityEffect);

  auto *rootLayout = new QVBoxLayout(this);
  rootLayout->setContentsMargins(4, 4, 4, 4);

  // ВНУТРЕННИЙ КОРПУС — видимая часть карточки на palette(base).
  QFrame *body = new QFrame(this);
  body->setObjectName("cardBody");

  auto *mainLayout = new QVBoxLayout(body);
  mainLayout->setContentsMargins(12, 10, 12, 10);
  mainLayout->setSpacing(8);

  // --- ВЕРХНИЙ РЯД: Аватар, Инициатива, Имя, «перс» ---
  auto *topLayout = new QHBoxLayout();

  avatarBtn = new QPushButton();
  avatarBtn->setFixedSize(32, 32);
  setRandomColor(avatarBtn); // Рандомный цвет кружка
  connect(avatarBtn, &QPushButton::clicked, this, &CharacterCard::pickColor);

  initSpin = new QSpinBox();
  initSpin->setRange(-20, 99);
  initSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
  initSpin->setFixedWidth(35);
  initSpin->setToolTip("Инициатива");

  nameEdit = new QLineEdit("Персонаж");

  QPushButton *persBtn = new QPushButton("перс");
  persBtn->setFixedSize(32, 22);
  persBtn->setStyleSheet(
      "font-size: 9px; border: 1px solid palette(mid); color: "
      "palette(button-text); background-color: palette(button);");
  persBtn->setToolTip("Выбрать или открыть чарник персонажа");
  connect(persBtn, &QPushButton::clicked, this, &CharacterCard::onPersButton);

  auto *delBtn = new QPushButton("×");
  delBtn->setFixedSize(22, 22);
  delBtn->setStyleSheet("border: none; font-size: 18px; font-weight: bold; "
                        "background: transparent;");
  connect(delBtn, &QPushButton::clicked, this, &CharacterCard::deleteLater);

  topLayout->addWidget(avatarBtn);
  topLayout->addWidget(new QLabel("In:"));
  topLayout->addWidget(initSpin);
  topLayout->addWidget(nameEdit);
  topLayout->addWidget(persBtn);
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

// --- Кнопка «перс»: стейт-зависимая логика ---
// Если персонаж ещё не привязан — открывает диалог выбора из хранилища.
// Если уже привязан — запрашивает открытие чарника во вкладке.
void CharacterCard::onPersButton() {
  if (currentFilePath.isEmpty()) {
    QString path = pickCharacterPath();
    if (path.isEmpty())
      return;

    // MainWindow is parent logic can't easily be accessed, so InitiativeTracker/Column
    // actually should provide Document. But for simplicity, we emit signal that we want to load,
    // OR we load Document here. However MainWindow manages cache.
    // Instead of instantiating Document here, we can set currentFilePath and emit bindingChanged.
    // Then TrackerColumn handles creating Document via MainWindow?
    // Actually, MainWindow only has access to CharacterDocument cache. We can just emit a signal
    // but the task says: "Передавать указатель на этот документ в CharacterSheet и CharacterCard."
    // If we pick a new file, we can just set currentFilePath and ask parent to load it?
    // Let's just emit bindingChanged and let InitiativeTracker/MainWindow set the Document.
    currentFilePath = path;
    emit documentRequested(this, currentFilePath);
    emit bindingChanged();
  } else {
    requestSheet();
  }
}

// Диалог выбора персонажа из хранилища.
// Сканит Storage::charactersDir() и показывает список имён через QFileDialog.
QString CharacterCard::pickCharacterPath() {
  QDir dir(Storage::charactersDir());
  if (!dir.exists())
    return {};

  const QStringList filters{"*.json"};
  const QStringList files =
      dir.entryList(filters, QDir::Files, QDir::Name);
  if (files.isEmpty())
    return {};

  // Формируем отображаемые имена: берём имя персонажа из JSON для читаемости.
  QStringList displayNames;
  QHash<QString, QString> nameToPath;
  for (const auto &fileName : files) {
    QFile f(dir.absoluteFilePath(fileName));
    if (f.open(QIODevice::ReadOnly)) {
      QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
      QJsonObject root = doc.object();
      QString innerJson = root.value("data").toString();
      QJsonObject data = QJsonDocument::fromJson(innerJson.toUtf8()).object();
      QString name =
          data.value("name").toObject().value("value").toString();
      if (name.isEmpty())
        name = QFileInfo(fileName).completeBaseName();
      displayNames.append(name);
      nameToPath[name] = dir.absoluteFilePath(fileName);
    }
  }

  bool ok = false;
  const QString chosen = QInputDialog::getItem(
      this, "Выбрать персонажа", "Персонаж:", displayNames, 0, false, &ok);
  if (!ok || chosen.isEmpty())
    return {};

  return nameToPath.value(chosen);
}

// Диалог выбора цвета аватара (QColorDialog → RGB → setColor).
void CharacterCard::pickColor() {
  QColor current = avatarBtn->palette().color(QPalette::Button);
  QColor color =
      QColorDialog::getColor(current, this, "Цвет аватара");
  if (!color.isValid())
    return;
  setColor(avatarBtn, color.red(), color.green(), color.blue());
}

// Применяет цвет к виджету аватара (кружок с тонкой рамкой).
void CharacterCard::setColor(QWidget *widget, int r, int g, int b) {
  widget->setStyleSheet(
      QString("border: 2px solid palette(text); border-radius: 16px; "
              "background-color: rgb(%1,%2,%3);")
          .arg(r)
          .arg(g)
          .arg(b));
}

// Загрузка данных из JSON файла LSS
void CharacterCard::setDocument(CharacterDocument *doc) {
  if (m_document == doc) return;
  m_document = doc;
  if (m_document) {
    currentFilePath = m_document->getFilePath();
    reloadFromDocument();
  } else {
    currentFilePath.clear();
  }
}

CharacterDocument* CharacterCard::getDocument() const {
  return m_document;
}

void CharacterCard::reloadFromDocument() {
  if (!m_document) return;
  
  nameEdit->setText(m_document->getName());
  hpSpin->setValue(m_document->getHp());
  
  int ac = m_document->getArmorClass();
  acLabel->setText(QString("КД: %1").arg(ac));
  
  int init = m_document->getInitiative();
  if (init != 0)
    initSpin->setValue(init);
}

QJsonObject CharacterCard::getEphemeralState() const {
  QJsonObject state;
  state["name"] = nameEdit->text();
  state["hp"] = hpSpin->value();
  state["initiative"] = initSpin->value();
  state["status"] = statusEdit->text();
  
  // Сохраняем цвет аватара
  QColor color = avatarBtn->palette().color(QPalette::Button);
  state["avatarColor"] = QString("%1,%2,%3").arg(color.red()).arg(color.green()).arg(color.blue());
  return state;
}

void CharacterCard::setEphemeralState(const QJsonObject &state) {
  if (state.contains("name")) nameEdit->setText(state["name"].toString());
  if (state.contains("hp")) hpSpin->setValue(state["hp"].toInt());
  if (state.contains("initiative")) initSpin->setValue(state["initiative"].toInt());
  if (state.contains("status")) statusEdit->setText(state["status"].toString());
  
  if (state.contains("avatarColor")) {
    QStringList rgb = state["avatarColor"].toString().split(",");
    if (rgb.size() == 3) {
      setColor(avatarBtn, rgb[0].toInt(), rgb[1].toInt(), rgb[2].toInt());
    }
  }
}

void CharacterCard::requestSheet() {
  if (!m_document) {
    QMessageBox::warning(this, "Внимание",
                         "Сначала привяжите персонажа (кнопка «перс»)! Либо это безымянный монстр.");
    return;
  }
  emit sheetRequested(currentFilePath);
}

void CharacterCard::syncVitalityToDocument() {
  if (m_document) {
    m_document->setHp(hpSpin->value());
  }
}

void CharacterCard::setRandomColor(QWidget *widget) {
  int r = QRandomGenerator::global()->bounded(160, 256);
  int g = QRandomGenerator::global()->bounded(160, 256);
  int b = QRandomGenerator::global()->bounded(160, 256);
  setColor(widget, r, g, b);
}

void CharacterCard::animateAppearance() {
  QPropertyAnimation *anim = new QPropertyAnimation(opacityEffect, "opacity");
  anim->setDuration(300);
  anim->setStartValue(0.0);
  anim->setEndValue(1.0);
  anim->start(QAbstractAnimation::DeleteWhenStopped);
}

int CharacterCard::getInitiative() const { return initSpin->value(); }

QString CharacterCard::getName() const {
  return nameEdit ? nameEdit->text() : QString();
}

QString CharacterCard::getFilePath() const { return currentFilePath; }

void CharacterCard::setFilePath(const QString &path) { currentFilePath = path; }

void CharacterCard::applyDamage() {
  hpSpin->setValue(hpSpin->value() - modSpin->value());
  modSpin->setValue(0);
  syncVitalityToDocument();
}

void CharacterCard::applyHeal() {
  hpSpin->setValue(hpSpin->value() + modSpin->value());
  modSpin->setValue(0);
  syncVitalityToDocument();
}

// Обработчик нажатия мыши
void CharacterCard::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    // Сохраняем начальную позицию для корректного старта Drag & Drop.
    dragStartPos = event->position().toPoint();
    event->accept();
  } else if (event->button() == Qt::RightButton) {
    // Контекстное меню: действия с привязкой персонажа.
    auto *menu = new QMenu(this);
    if (currentFilePath.isEmpty()) {
      menu->addAction("Выбрать персонажа…",
                       this, &CharacterCard::onPersButton);
    } else {
      menu->addAction("Открыть чарник",
                       this, &CharacterCard::requestSheet);
      menu->addSeparator();
      menu->addAction("Сменить персонажа…", this, [this]() {
        // Сбрасываем привязку и открываем диалог выбора.
        setDocument(nullptr);
        nameEdit->clear();
        hpSpin->setValue(10);
        acLabel->setText("КД: --");
        initSpin->setValue(0);
        onPersButton();
      });
    }
    menu->exec(event->globalPosition().toPoint());
    menu->deleteLater();
    event->accept();
  } else {
    QFrame::mousePressEvent(event);
  }
}

void CharacterCard::mouseMoveEvent(QMouseEvent *event) {
  // На мобильных / тач-устройствах не запускаем Drag, чтобы не блокировать пальцевый скролл списка
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
  if (!(event->buttons() & Qt::LeftButton))
    return;

  // Стартуем drag только при достаточном перемещении (стандартный порог Qt).
  if ((event->position().toPoint() - dragStartPos).manhattanLength() <
      QApplication::startDragDistance())
    return;

  QDrag *drag = new QDrag(this);
  QMimeData *mimeData = new QMimeData;

  mimeData->setData("application/x-charactercard",
                    QByteArray::number((qintptr)this));

  drag->setMimeData(mimeData);
  drag->setPixmap(grab());
  drag->setHotSpot(event->position().toPoint());

  drag->exec(Qt::MoveAction);
#else
  QFrame::mouseMoveEvent(event);
#endif
}

CharacterCard::~CharacterCard() {}
