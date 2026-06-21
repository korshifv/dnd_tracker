#include "CharacterSheet.h"
#include "CharacterDocument.h"
#include "JsonUtils.h"
#include "Storage.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QStyle>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <cmath>

// --- WeaponEditorDialog Implementation ---
WeaponEditorDialog::WeaponEditorDialog(const WeaponData &data, int profBonus,
                                       const QMap<QString, int> &statMods,
                                       QWidget *parent)
    : QDialog(parent), pb(profBonus), mods(statMods) {

  setWindowTitle("Редактирование оружия");
  setFixedWidth(400);

  auto *l = new QVBoxLayout(this);
  l->setSpacing(15);
  l->setContentsMargins(20, 20, 20, 20);

  nameEdit = new QLineEdit(data.name);
  nameEdit->setPlaceholderText("Название оружия");
  nameEdit->setStyleSheet("font-size: 18px; font-weight: bold;");
  l->addWidget(nameEdit);

  auto *grid = new QGridLayout();
  grid->addWidget(new QLabel("Характеристика:"), 0, 0);
  abilityCombo = new QComboBox();
  abilityCombo->addItem("Нет", "none");
  abilityCombo->addItem("Сила", "str");
  abilityCombo->addItem("Ловкость", "dex");
  abilityCombo->addItem("Тело", "con");
  abilityCombo->addItem("Интеллект", "int");
  abilityCombo->addItem("Мудрость", "wis");
  abilityCombo->addItem("Харизма", "cha");
  abilityCombo->setCurrentIndex(abilityCombo->findData(data.ability));
  grid->addWidget(abilityCombo, 0, 1);

  profCheck = new QCheckBox("Владение");
  profCheck->setChecked(data.isProf);
  grid->addWidget(profCheck, 1, 0);

  grid->addWidget(new QLabel("Доп. бонус:"), 1, 1);
  magicSpin = new QSpinBox();
  magicSpin->setRange(-10, 10);
  magicSpin->setValue(data.magicBonus);
  grid->addWidget(magicSpin, 1, 2);

  l->addLayout(grid);

  hitBonusLabel = new QLabel("Итоговый бонус: +0");
  hitBonusLabel->setStyleSheet("color: palette(highlight); font-size: 14px;");
  l->addWidget(hitBonusLabel);

  l->addWidget(new QLabel("Урон / Вид:"));
  dmgEdit = new QLineEdit(data.damage);
  dmgEdit->setPlaceholderText("например: 1к8 колющий");
  l->addWidget(dmgEdit);

  l->addWidget(new QLabel("Заметки:"));
  notesEdit = new QTextEdit(data.notes);
  notesEdit->setMaximumHeight(100);
  l->addWidget(notesEdit);

  auto *btns = new QHBoxLayout();
  auto *delBtn = new QPushButton("УДАЛИТЬ");
  delBtn->setObjectName("delBtn");
  delBtn->setStyleSheet(
      "QPushButton { color: palette(highlight); border: 1px solid "
      "palette(highlight); padding: 8px; border-radius: 4px; } "
      "QPushButton:hover { background: palette(midlight); }");
  connect(delBtn, &QPushButton::clicked, this, [this]() {
    deleted = true;
    accept();
  });

  auto *okBtn = new QPushButton("ОК");
  okBtn->setObjectName("saveBtn");
  connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);

  btns->addWidget(delBtn);
  btns->addStretch();
  btns->addWidget(okBtn);
  l->addLayout(btns);

  connect(abilityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &WeaponEditorDialog::updateHitBonus);
  connect(profCheck, &QCheckBox::toggled, this,
          &WeaponEditorDialog::updateHitBonus);
  connect(magicSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &WeaponEditorDialog::updateHitBonus);

  updateHitBonus();
}

void WeaponEditorDialog::updateHitBonus() {
  QString ab = abilityCombo->currentData().toString();
  int m = (ab == "none") ? 0 : mods.value(ab, 0);
  int total = m + (profCheck->isChecked() ? pb : 0) + magicSpin->value();
  hitBonusLabel->setText(
      QString("Итоговый бонус: %1%2").arg(total >= 0 ? "+" : "").arg(total));
}

WeaponData WeaponEditorDialog::getWeaponData() const {
  return {nameEdit->text(),       abilityCombo->currentData().toString(),
          profCheck->isChecked(), magicSpin->value(),
          dmgEdit->text(),        notesEdit->toPlainText()};
}

// --- CharacterSheet Implementation ---
CharacterSheet::CharacterSheet(CharacterDocument *doc, QWidget *parent)
    : QWidget(parent), m_document(doc),
      weaponListLayout(nullptr), attacksBlockFrame(nullptr) {

  const QJsonObject &data = m_document->getData();

  // Раньше это был QDialog с resize(1300,950)/WA_DeleteOnClose/локальным QSS.
  // Теперь мы встраиваемся во вкладку: размером управляет вкладка,
  // lifetime — тоже. Стили — нативная палитра + scoped-правила ниже.
  setMinimumSize(1100, 800);
  applyThemeStyle();

  // Таймеры автосейва: 3с бездействия → saveToFile(); 2с показа "Сохранено ✓".
  m_autosaveTimer = new QTimer(this);
  m_autosaveTimer->setSingleShot(true);
  m_autosaveTimer->setInterval(3000);
  connect(m_autosaveTimer, &QTimer::timeout, this, &CharacterSheet::saveToFile);
  m_statusTimer = new QTimer(this);
  m_statusTimer->setSingleShot(true);
  m_statusTimer->setInterval(2000);
  connect(m_statusTimer, &QTimer::timeout, this,
          [this]() { saveStatusLabel->setText(""); });

  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(10, 10, 10, 10);
  mainLayout->setSpacing(5);

  tabs = new QTabWidget(this);

  auto *charTab = new QWidget();
  auto *charLayout = new QVBoxLayout(charTab);
  charLayout->setContentsMargins(5, 5, 5, 5);
  auto *scroll = new QScrollArea();
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);
  auto *dashboard = new QWidget();
  dashboard->setObjectName("dashboard");
  scroll->setWidget(dashboard);
  charLayout->addWidget(scroll);

  setupMainLayout(dashboard, data);
  tabs->addTab(charTab, "ПЕРСОНАЖ");
  setupMagicTab(data);
  mainLayout->addWidget(tabs);

  updateAllCalculations();

  // Восстанавливаем состояние полей, которые раньше не сохранялись.
  // Делаем после создания всех виджетов, чтобы не зависеть от порядка блоков.
  inspirationCheck->setChecked(JsonUtils::safeGetBool(data, {"inspiration"}));

  // Подписываемся на изменения из документа
  connect(m_document, &CharacterDocument::hpChanged, this, &CharacterSheet::onDocumentHpChanged);

  // Подключаем автосейв ко всем редактируемым полям. Header- и stat-поля
  // подключены в своих create*-методах; здесь — остальные.
  connectDirtySpin(globalProfSpin);
  connectDirtyCheck(inspirationCheck);
  connectDirtySpin(acSpin);
  connectDirtyField(speedEdit);
  connectDirtyField(hdValueEdit);
  connectDirtySpin(hpMaxSpin);
  connectDirtySpin(hpCurrentSpin);
  connectDirtySpin(hpTempSpin);
  for (const auto &k : saveProfBtns.keys())
    connectDirtyBtn(saveProfBtns[k]);
  for (const auto &k : skillProfBtns.keys())
    connectDirtyBtn(skillProfBtns[k]);
  for (auto *btn : deathSavesSuccess)
    connectDirtyBtn(btn);
  for (auto *btn : deathSavesFail)
    connectDirtyBtn(btn);
  connectDirtyEdit(featuresEdit);
  connectDirtyEdit(inventoryEdit);
  connectDirtyField(spellClassEdit);
  connect(spellAbilityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &CharacterSheet::markDirty);
  for (const auto &k : magicEdits.keys())
    connectDirtyEdit(magicEdits[k]);
  for (const auto &k : spellSlotSpins.keys())
    connectDirtySpin(spellSlotSpins[k]);
  // traitsEdit/idealsEdit/bondsEdit/flawsEdit создаются в createPersonalityBlock
  // и сохранены в члены — подключаем здесь.
  if (traitsEdit) connectDirtyEdit(traitsEdit);
  if (idealsEdit) connectDirtyEdit(idealsEdit);
  if (bondsEdit) connectDirtyEdit(bondsEdit);
  if (flawsEdit) connectDirtyEdit(flawsEdit);

  // Все виджеты созданы и заполнены — разрешаем автосейв. До этого момента
  // вызовы setValue/setText/pl.:setChecked (в т.ч. updateAllCalculations)
  // не должны запускать таймер сохранения.
  m_loaded = true;
}

QString CharacterSheet::getFilePath() const {
  return m_document->getFilePath();
}

void CharacterSheet::onDocumentHpChanged(int newHp) {
  // Обновляем UI без триггера автосейва
  bool oldLoaded = m_loaded;
  m_loaded = false;
  hpCurrentSpin->setValue(newHp);
  m_loaded = oldLoaded;
}

void CharacterSheet::setupMainLayout(QWidget *dashboard,
                                     const QJsonObject &data) {
  auto *l = new QVBoxLayout(dashboard);
  l->setContentsMargins(10, 10, 10, 10);
  l->setSpacing(15);

  // Помечает блок как плавающую панель (палитровый стиль задан в applyThemeStyle).
  auto panel = [](QWidget *w) {
    w->setObjectName("cardPanel");
    return w;
  };

  l->addWidget(panel(createHeaderBlock(data)));

  auto *colsL = new QHBoxLayout();
  colsL->setSpacing(10);

  auto *col1 = new QVBoxLayout();
  auto *col2 = new QVBoxLayout();
  auto *col3 = new QVBoxLayout();
  auto *col4 = new QVBoxLayout();

  QStringList stats = {"str", "dex", "con", "int", "wis", "cha"};
  QStringList statNames = {"СИЛА", "ЛОВКОСТЬ", "ТЕЛО", "ИНТ", "МУД", "ХАР"};
  for (int i = 0; i < stats.size(); ++i) {
    col1->addWidget(panel(createStatBlock(
        statNames[i], stats[i],
        data["stats"].toObject()[stats[i]].toObject()["score"].toInt())));
  }
  col1->addStretch(1);

  col2->addWidget(panel(createSavingThrowsBlock(data)));
  col2->addWidget(panel(createSkillsBlock(data)));
  col2->addStretch(1);

  col3->addWidget(panel(createCombatStatsBlock(data)));
  col3->addWidget(panel(createHPBlock(data)));
  col3->addWidget(createAttacksBlock(data)); // сам задаёт objectName attacksFrame
  col3->addWidget(panel(createEquipmentBlock(data)));
  col3->addStretch(1);

  col4->addWidget(panel(createPersonalityBlock(data)));
  col4->addWidget(panel(createFeaturesBlock(data)));
  col4->addStretch(0);

  colsL->addLayout(col1);
  colsL->addLayout(col2);
  colsL->addLayout(col3, 1);
  colsL->addLayout(col4);
  l->addLayout(colsL);
}

QWidget *CharacterSheet::createHeaderBlock(const QJsonObject &data) {
  auto *f = new QFrame();
  auto *grid = new QGridLayout(f);
  grid->setContentsMargins(10, 10, 10, 10);
  grid->setSpacing(8);
  QJsonObject info = data["info"].toObject();
  nameEditField = new QLineEdit(data["name"].toObject()["value"].toString());
  nameEditField->setObjectName("charNameEdit");

  classEdit = new QLineEdit(info["charClass"].toObject()["value"].toString());
  subclassEdit =
      new QLineEdit(info["charSubclass"].toObject()["value"].toString());
  backgroundEdit =
      new QLineEdit(info["background"].toObject()["value"].toString());
  raceEdit = new QLineEdit(info["race"].toObject()["value"].toString());
  alignmentEdit =
      new QLineEdit(info["alignment"].toObject()["value"].toString());
  expEdit = new QLineEdit(info["experience"].toObject()["value"].toString());
  playerNameEdit =
      new QLineEdit(info["playerName"].toObject()["value"].toString());
  levelSpin = new QSpinBox();
  levelSpin->setRange(1, 40);
  levelSpin->setValue(info["level"].toObject()["value"].toInt());

  auto *saveBtn = new QPushButton("💾 СОХРАНИТЬ");
  saveBtn->setObjectName("saveBtn");
  saveBtn->setFixedSize(140, 40);
  connect(saveBtn, &QPushButton::clicked, this, &CharacterSheet::saveToFile);

  // Индикатор статуса автосейва: "Изменения…" → "Сохранено ✓" → "".
  saveStatusLabel = new QLabel("");
  saveStatusLabel->setAlignment(Qt::AlignCenter);
  saveStatusLabel->setStyleSheet(
      "color: palette(link); font-size: 0.7em;");

  auto addF = [&](const QString &label, QWidget *w, int r, int c, int rs = 1,
                  int cs = 1) {
    auto *v = new QVBoxLayout();
    v->setSpacing(1);
    v->addWidget(w);
    auto *l = new QLabel(label);
    l->setProperty("class", "sublabel");
    v->addWidget(l);
    grid->addLayout(v, r, c, rs, cs);
  };
  addF("ИМЯ ПЕРСОНАЖА", nameEditField, 0, 0, 2, 2);
  addF("КЛАСС", classEdit, 0, 2);
  addF("ПРЕДЫСТОРИЯ", backgroundEdit, 0, 3);
  addF("ИМЯ ИГРОКА", playerNameEdit, 0, 4);
  addF("РАСА", raceEdit, 1, 2);
  addF("МИРОВОЗЗРЕНИЕ", alignmentEdit, 1, 3);
  addF("ОПЫТ", expEdit, 1, 4);

  auto *lvlV = new QVBoxLayout();
  lvlV->setSpacing(1);
  lvlV->addWidget(levelSpin);
  auto *l = new QLabel("УРОВЕНЬ");
  l->setProperty("class", "sublabel");
  lvlV->addWidget(l);
  grid->addLayout(lvlV, 0, 5, 2, 1);

  // Колонка 6: кнопка сохранения + индикатор статуса под ней.
  auto *saveCol = new QVBoxLayout();
  saveCol->setSpacing(2);
  saveCol->addWidget(saveBtn);
  saveCol->addWidget(saveStatusLabel);
  grid->addLayout(saveCol, 0, 6, 2, 1, Qt::AlignCenter);

  // Автосейв: все поля шапки помечают лист как изменённый.
  connectDirtyField(nameEditField);
  connectDirtyField(classEdit);
  connectDirtyField(subclassEdit);
  connectDirtyField(backgroundEdit);
  connectDirtyField(raceEdit);
  connectDirtyField(alignmentEdit);
  connectDirtyField(expEdit);
  connectDirtyField(playerNameEdit);
  connectDirtySpin(levelSpin);

  grid->setColumnStretch(0, 2);
  grid->setColumnStretch(2, 1);
  return f;
}

QWidget *CharacterSheet::createStatBlock(const QString &label,
                                         const QString &statKey, int score) {
  auto *f = new QFrame();
  f->setFixedSize(90, 100);
  auto *l = new QVBoxLayout(f);
  l->setContentsMargins(5, 5, 5, 5);
  l->setSpacing(0);
  auto *t = new QLabel(label);
  t->setAlignment(Qt::AlignCenter);
  t->setProperty("class", "sublabel");
  auto *modVal = new QLineEdit("+0");
  modVal->setAlignment(Qt::AlignCenter);
  modVal->setObjectName("combatStatValue");
  modVal->setReadOnly(true);
  modLabels[statKey] = modVal;
  auto *sb = new QSpinBox();
  sb->setRange(1, 40);
  sb->setValue(score);
  sb->setAlignment(Qt::AlignCenter);
  sb->setButtonSymbols(QAbstractSpinBox::NoButtons);
  statSpins[statKey] = sb;
  connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &CharacterSheet::updateAllCalculations);
  connectDirtySpin(sb);
  l->addWidget(t);
  l->addWidget(modVal);
  l->addStretch();
  l->addWidget(sb);
  return f;
}

QWidget *CharacterSheet::createSavingThrowsBlock(const QJsonObject &data) {
  auto *f = new QFrame();
  auto *l = new QVBoxLayout(f);
  l->setContentsMargins(10, 10, 10, 10);
  l->setSpacing(4);
  globalProfSpin = new QSpinBox();
  globalProfSpin->setRange(0, 20);
  globalProfSpin->setValue(data["proficiency"].toInt());
  connect(globalProfSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &CharacterSheet::updateAllCalculations);
  auto *h1 = new QHBoxLayout();
  h1->addWidget(globalProfSpin);
  h1->addWidget(new QLabel("БОНУС МАСТЕРСТВА"));
  h1->addStretch();
  l->addLayout(h1);
  inspirationCheck = new QCheckBox("ВДОХНОВЕНИЕ");
  l->addWidget(inspirationCheck);
  l->addSpacing(5);
  l->addWidget(new QLabel("СПАСБРОСКИ"));
  QStringList stats = {"str", "dex", "con", "int", "wis", "cha"};
  for (const QString &k : stats) {
    savesState[k] = data["saves"].toObject()[k].toObject()["isProf"].toBool();
    auto *h = new QHBoxLayout();
    h->setSpacing(5);
    auto *btn = new QPushButton();
    btn->setFixedSize(20, 20);
    btn->setCheckable(true);
    btn->setChecked(savesState[k]);
    btn->setProperty("class", "indicator");
    saveProfBtns[k] = btn;
    connect(btn, &QPushButton::clicked,
            [this, k]() { toggleSaveProficiency(k); });
    auto *v = new QLineEdit("+0");
    v->setFixedWidth(35);
    v->setAlignment(Qt::AlignCenter);
    v->setReadOnly(true);
    saveBonusLabels[k] = v;
    h->addWidget(btn);
    h->addWidget(v);
    h->addWidget(new QLabel(k.toUpper()));
    h->addStretch();
    l->addLayout(h);
  }
  return f;
}

QWidget *CharacterSheet::createSkillsBlock(const QJsonObject &data) {
  auto *f = new QFrame();
  auto *l = new QVBoxLayout(f);
  l->setContentsMargins(10, 10, 10, 10);
  l->setSpacing(2);
  l->addWidget(new QLabel("НАВЫКИ"));
  QJsonObject sks = data["skills"].toObject();
  QStringList sorted = sks.keys();
  sorted.sort();
  for (const QString &k : sorted) {
    auto s = sks[k].toObject();
    int p = s["isProf"].toInt();
    skillsState[k] = {s["baseStat"].toString(), p};
    auto *h = new QHBoxLayout();
    h->setSpacing(4);
    auto *btn = new QPushButton();
    btn->setFixedSize(20, 20);
    btn->setProperty("class", "indicator");
    if (p == 1)
      btn->setObjectName("prof1");
    else if (p == 2)
      btn->setObjectName("prof2");
    skillProfBtns[k] = btn;
    connect(btn, &QPushButton::clicked,
            [this, k]() { toggleSkillProficiency(k); });
    auto *val = new QSpinBox();
    val->setRange(-20, 50);
    val->setButtonSymbols(QAbstractSpinBox::NoButtons);
    val->setFixedWidth(28);
    val->setAlignment(Qt::AlignCenter);
    skillSpins[k] = val;
    auto *lbl = new QLabel(s["label"].toString());
    lbl->setStyleSheet("font-size: 0.9em;");
    auto *cl = new QLabel("");
    cl->setProperty("class", "sublabel");
    skillCalcLabels[k] = cl;
    h->addWidget(btn);
    h->addWidget(val);
    h->addWidget(lbl);
    h->addStretch();
    h->addWidget(cl);
    l->addLayout(h);
  }
  passivePerceptionLabel = new QLineEdit("0");
  passivePerceptionLabel->setFixedWidth(35);
  passivePerceptionLabel->setAlignment(Qt::AlignCenter);
  auto *ppL = new QHBoxLayout();
  ppL->addWidget(new QLabel("ВНИМ. (П):"));
  ppL->addWidget(passivePerceptionLabel);
  ppL->addStretch();
  l->addLayout(ppL);
  return f;
}

QWidget *CharacterSheet::createCombatStatsBlock(const QJsonObject &data) {
  auto *f = new QFrame();
  auto *l = new QHBoxLayout(f);
  l->setContentsMargins(5, 5, 5, 5);
  l->setSpacing(10);
  auto makeB = [&](const QString &t, QWidget *w, bool shield = false,
                   bool smallLabel = false) {
    auto *b = new QFrame();
    b->setMinimumWidth(110);
    b->setMinimumHeight(80);
    if (shield)
      b->setObjectName("acShield");
    auto *v = new QVBoxLayout(b);
    v->setContentsMargins(5, 5, 5, 5);
    v->setSpacing(2);
    w->setObjectName("combatStatValue");
    auto *lbl = new QLabel(t);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setProperty("class", smallLabel ? "small-label" : "sublabel");
    v->addWidget(w);
    v->addWidget(lbl);
    return b;
  };
  acSpin = new QSpinBox();
  acSpin->setRange(0, 50);
  acSpin->setAlignment(Qt::AlignCenter);
  acSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
  acSpin->setValue(
      data["vitality"].toObject()["ac"].toObject()["value"].toInt());
  initEdit = new QLineEdit("+0");
  initEdit->setAlignment(Qt::AlignCenter);
  initEdit->setReadOnly(true);
  QString sVal =
      data["vitality"].toObject()["speed"].toObject()["value"].toString();
  if (sVal.isEmpty())
    sVal = "30";
  speedEdit = new QLineEdit(sVal);
  speedEdit->setAlignment(Qt::AlignCenter);
  l->addWidget(makeB("КД", acSpin, true));
  l->addWidget(makeB("ИНИЦИАТИВА", initEdit, false, true));
  l->addWidget(makeB("СКОРОСТЬ", speedEdit));
  return f;
}

QWidget *CharacterSheet::createHPBlock(const QJsonObject &data) {
  auto *f = new QFrame();
  auto *l = new QVBoxLayout(f);
  l->setContentsMargins(10, 10, 10, 10);
  l->setSpacing(5);
  QJsonObject v = data["vitality"].toObject();
  auto *top = new QHBoxLayout();
  top->addWidget(new QLabel("МАКСИМУМ ХИТОВ"));
  hpMaxSpin = new QSpinBox();
  hpMaxSpin->setRange(0, 999);
  hpMaxSpin->setValue(v["hp-max"].toObject()["value"].toInt());
  top->addWidget(hpMaxSpin);
  l->addLayout(top);
  hpCurrentSpin = new QSpinBox();
  hpCurrentSpin->setRange(-200, 999);
  hpCurrentSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
  hpCurrentSpin->setValue(v["hp-current"].toInt());
  hpCurrentSpin->setAlignment(Qt::AlignCenter);
  hpCurrentSpin->setObjectName("combatStatValue");
  l->addWidget(hpCurrentSpin);
  l->addWidget(new QLabel("ТЕКУЩИЕ ХИТЫ"), 0, Qt::AlignCenter);
  hpTempSpin = new QSpinBox();
  hpTempSpin->setRange(0, 500);
  hpTempSpin->setAlignment(Qt::AlignCenter);
  hpTempSpin->setValue(v["hp-temp"].toInt());
  l->addWidget(hpTempSpin);
  l->addWidget(new QLabel("ВРЕМЕННЫЕ ХИТЫ"), 0, Qt::AlignCenter);
  auto *bot = new QHBoxLayout();
  auto *hdF = new QFrame();
  auto *hdV = new QVBoxLayout(hdF);
  hdValueEdit = new QLineEdit(v["hit-die"].toObject()["value"].toString());
  hdValueEdit->setAlignment(Qt::AlignCenter);
  hdV->addWidget(new QLabel("Всего"), 0, Qt::AlignCenter);
  hdV->addWidget(hdValueEdit);
  hdV->addWidget(new QLabel("КОСТЬ ХИТОВ"), 0, Qt::AlignCenter);
  bot->addWidget(hdF);
  auto *dsF = new QFrame();
  auto *dsV = new QVBoxLayout(dsF);
  auto *succL = new QHBoxLayout();
  succL->addWidget(new QLabel("У:"));
  for (int i = 0; i < 3; ++i) {
    auto *btn = new QPushButton();
    btn->setFixedSize(20, 20);
    btn->setCheckable(true);
    btn->setProperty("class", "indicator");
    deathSavesSuccess.append(btn);
    succL->addWidget(btn);
  }
  dsV->addLayout(succL);
  auto *failL = new QHBoxLayout();
  failL->addWidget(new QLabel("П:"));
  for (int i = 0; i < 3; ++i) {
    auto *btn = new QPushButton();
    btn->setFixedSize(20, 20);
    btn->setCheckable(true);
    btn->setProperty("class", "indicator-fail");
    deathSavesFail.append(btn);
    failL->addWidget(btn);
  }
  dsV->addLayout(failL);
  dsV->addWidget(new QLabel("СМЕРТЬ"), 0, Qt::AlignCenter);
  bot->addWidget(dsF);
  l->addLayout(bot);

  // Восстанавливаем состояние спасбросков смерти из данных.
  QJsonObject deathSaves = data["deathSaves"].toObject();
  QJsonArray dsSucc = deathSaves["success"].toArray();
  for (int i = 0; i < deathSavesSuccess.size() && i < dsSucc.size(); ++i)
    deathSavesSuccess[i]->setChecked(dsSucc.at(i).toBool());
  QJsonArray dsFail = deathSaves["fail"].toArray();
  for (int i = 0; i < deathSavesFail.size() && i < dsFail.size(); ++i)
    deathSavesFail[i]->setChecked(dsFail.at(i).toBool());

  return f;
}

QWidget *CharacterSheet::createAttacksBlock(const QJsonObject &data) {
  attacksBlockFrame = new QFrame();
  attacksBlockFrame->setObjectName("attacksFrame");
  attacksBlockFrame->setFixedHeight(350);
  auto *l = new QVBoxLayout(attacksBlockFrame);
  l->setContentsMargins(10, 5, 10, 5);
  l->setSpacing(5);

  // Header with Labels and Add Button
  auto *headerLayout = new QHBoxLayout();
  auto *addBtnTop = new QPushButton("+");
  addBtnTop->setObjectName("smallControl");
  connect(addBtnTop, &QPushButton::clicked, this,
          &CharacterSheet::addNewWeapon);
  headerLayout->addWidget(addBtnTop);

  auto *labelsH = new QHBoxLayout();
  labelsH->setContentsMargins(10, 0, 10, 0);
  labelsH->setSpacing(10);
  auto *l1 = new QLabel("НАЗВАНИЕ");
  l1->setProperty("class", "sublabel");
  l1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  auto *l2 = new QLabel("БОНУС");
  l2->setProperty("class", "sublabel");
  l2->setAlignment(Qt::AlignCenter);
  auto *l3 = new QLabel("УРОН / ВИД");
  l3->setProperty("class", "sublabel");
  l3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  labelsH->addWidget(l1, 3);
  labelsH->addWidget(l2, 1);
  labelsH->addWidget(l3, 2);
  headerLayout->addLayout(labelsH, 1);
  l->addLayout(headerLayout);

  auto *scroll = new QScrollArea();
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);
  auto *cont = new QWidget();
  weaponListLayout = new QVBoxLayout(cont);
  weaponListLayout->setContentsMargins(0, 5, 0, 5);
  weaponListLayout->setSpacing(8);

  weaponsData.clear();
  QJsonArray wps = data["weaponsList"].toArray();
  for (auto v : wps) {
    QJsonObject w = v.toObject();
    weaponsData.append({w["name"].toObject()["value"].toString(),
                        w["ability"].toString(), w["isProf"].toBool(),
                        w["modBonus"].toObject()["value"].toInt(),
                        w["dmg"].toObject()["value"].toString(), ""});
  }

  scroll->setWidget(cont);
  l->addWidget(scroll);

  // Footer with Sizing Controls
  auto *footer = new QHBoxLayout();
  auto *title = new QLabel("АТАКИ И ЗАКЛИНАНИЯ");
  title->setProperty("class", "sublabel");
  footer->addStretch();
  footer->addWidget(title);
  footer->addStretch();

  auto *sizeControls = new QFrame();
  sizeControls->setStyleSheet("background: palette(window); border-radius: "
                              "6px; border: 1px solid palette(text);");
  auto *scL = new QHBoxLayout(sizeControls);
  scL->setContentsMargins(4, 4, 4, 4);
  scL->setSpacing(4);
  auto *plus = new QPushButton("+");
  plus->setObjectName("smallControl");
  connect(plus, &QPushButton::clicked, this,
          &CharacterSheet::expandAttacksBlock);
  auto *minus = new QPushButton("-");
  minus->setObjectName("smallControl");
  connect(minus, &QPushButton::clicked, this,
          &CharacterSheet::shrinkAttacksBlock);
  scL->addWidget(plus);
  scL->addWidget(minus);
  footer->addWidget(sizeControls);
  l->addLayout(footer);

  refreshWeaponList();
  return attacksBlockFrame;
}

void CharacterSheet::refreshWeaponList() {
  while (auto *it = weaponListLayout->takeAt(0)) {
    if (it->widget())
      delete it->widget();
    delete it;
  }
  int pbVal = globalProfSpin ? globalProfSpin->value() : 2;
  QMap<QString, int> mods;
  for (auto k : statSpins.keys())
    mods[k] = std::floor((statSpins[k]->value() - 10) / 2.0);

  for (int i = 0; i < weaponsData.size(); ++i) {
    auto &w = weaponsData[i];
    auto *card = new QPushButton();
    card->setObjectName("weaponCard");
    card->setContentsMargins(5, 2, 5, 2);
    auto *cl = new QHBoxLayout(card);
    cl->setContentsMargins(0, 0, 0, 0);
    cl->setSpacing(10);

    int statMod = (w.ability == "none") ? 0 : mods.value(w.ability, 0);
    int totalHit = statMod + (w.isProf ? pbVal : 0) + w.magicBonus;

    auto *nameB = new QLabel(w.name);
    nameB->setProperty("class", "weapon-blob");
    nameB->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    auto *hitB =
        new QLabel(QString("%1%2").arg(totalHit >= 0 ? "+" : "").arg(totalHit));
    hitB->setProperty("class", "weapon-blob");
    hitB->setAlignment(Qt::AlignCenter);
    auto *dmgB = new QLabel(w.damage);
    dmgB->setProperty("class", "weapon-blob");
    dmgB->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    cl->addWidget(nameB, 3);
    cl->addWidget(hitB, 1);
    cl->addWidget(dmgB, 2);
    connect(card, &QPushButton::clicked, this, [this, i]() { editWeapon(i); });
    weaponListLayout->addWidget(card);
  }
  weaponListLayout->addStretch(1);
}

void CharacterSheet::expandAttacksBlock() {
  if (attacksBlockFrame)
    attacksBlockFrame->setFixedHeight(attacksBlockFrame->height() + 50);
}

void CharacterSheet::shrinkAttacksBlock() {
  if (attacksBlockFrame && attacksBlockFrame->height() > 200)
    attacksBlockFrame->setFixedHeight(attacksBlockFrame->height() - 50);
}

void CharacterSheet::addNewWeapon() {
  weaponsData.append({"Новое оружие", "str", true, 0, "1к8", ""});
  refreshWeaponList();
  markDirty();
}

void CharacterSheet::editWeapon(int index) {
  if (index < 0 || index >= weaponsData.size())
    return;
  int pbVal = globalProfSpin ? globalProfSpin->value() : 2;
  QMap<QString, int> mods;
  for (auto k : statSpins.keys())
    mods[k] = std::floor((statSpins[k]->value() - 10) / 2.0);

  WeaponEditorDialog dlg(weaponsData[index], pbVal, mods, this);
  if (dlg.exec() == QDialog::Accepted) {
    if (dlg.isDeleted())
      weaponsData.removeAt(index);
    else
      weaponsData[index] = dlg.getWeaponData();
    refreshWeaponList();
    markDirty();
  }
}

QWidget *CharacterSheet::createPersonalityBlock(const QJsonObject &data) {
  auto *f = new QFrame();
  auto *l = new QVBoxLayout(f);
  l->setContentsMargins(10, 10, 10, 10);
  l->setSpacing(5);
  auto makeS = [&](const QString &t, QTextEdit *&ed, const QString &k) {
    auto *b = new QFrame();
    b->setObjectName("textBlock");
    auto *v = new QVBoxLayout(b);
    v->setContentsMargins(5, 5, 5, 5);
    v->setSpacing(2);
    ed = new QTextEdit();
    ed->setPlainText(tipTapToPlain(
        data["text"].toObject()[k].toObject()["value"].toObject()));
    ed->setMinimumHeight(60);
    v->addWidget(ed);
    auto *lbl = new QLabel(t);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setProperty("class", "sublabel");
    v->addWidget(lbl);
    return b;
  };
  l->addWidget(makeS("ЧЕРТЫ ХАРАКТЕРА", traitsEdit, "personality"));
  l->addWidget(makeS("ИДЕАЛЫ", idealsEdit, "ideals"));
  l->addWidget(makeS("ПРИВЯЗАННОСТИ", bondsEdit, "bonds"));
  l->addWidget(makeS("СЛАБОСТИ", flawsEdit, "flaws"));
  return f;
}

QWidget *CharacterSheet::createFeaturesBlock(const QJsonObject &data) {
  auto *f = new QFrame();
  auto *l = new QVBoxLayout(f);
  l->setContentsMargins(10, 10, 10, 10);
  l->addWidget(new QLabel("УМЕНИЯ И ОСОБЕННОСТИ"));
  featuresEdit = new QTextEdit();
  featuresEdit->setPlainText(tipTapToPlain(
      data["text"].toObject()["traits"].toObject()["value"].toObject()));
  l->addWidget(featuresEdit);
  return f;
}

QWidget *CharacterSheet::createEquipmentBlock(const QJsonObject &data) {
  auto *f = new QFrame();
  auto *l = new QVBoxLayout(f);
  l->setContentsMargins(10, 10, 10, 10);
  inventoryEdit = new QTextEdit();
  inventoryEdit->setPlainText(tipTapToPlain(
      data["text"].toObject()["equipment"].toObject()["value"].toObject()));
  l->addWidget(inventoryEdit);
  l->addWidget(new QLabel("СНАРЯЖЕНИЕ"), 0, Qt::AlignCenter);
  return f;
}

void CharacterSheet::setupMagicTab(const QJsonObject &data) {
  auto *tab = new QWidget();
  auto *l = new QVBoxLayout(tab);
  l->setContentsMargins(15, 15, 15, 15);
  auto *h = new QHBoxLayout();
  spellClassEdit =
      new QLineEdit(data["casterClass"].toObject()["value"].toString());
  spellAbilityCombo = new QComboBox();
  spellAbilityCombo->addItem("Интеллект", "int");
  spellAbilityCombo->addItem("Мудрость", "wis");
  spellAbilityCombo->addItem("Харизма", "cha");
  spellSaveDCEdit = new QLineEdit();
  spellSaveDCEdit->setFixedWidth(40);
  spellSaveDCEdit->setAlignment(Qt::AlignCenter);
  spellSaveDCEdit->setReadOnly(true);
  spellAttackBonusEdit = new QLineEdit();
  spellAttackBonusEdit->setFixedWidth(40);
  spellAttackBonusEdit->setAlignment(Qt::AlignCenter);
  spellAttackBonusEdit->setReadOnly(true);
  h->addWidget(new QLabel("КЛАСС:"));
  h->addWidget(spellClassEdit, 2);
  h->addWidget(new QLabel("ХАР-КА:"));
  h->addWidget(spellAbilityCombo, 1);
  h->addWidget(new QLabel("DC:"));
  h->addWidget(spellSaveDCEdit);
  h->addWidget(new QLabel("БОНУС:"));
  h->addWidget(spellAttackBonusEdit);
  l->addLayout(h);
  auto *scroll = new QScrollArea();
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);
  auto *cont = new QWidget();
  auto *grid = new QGridLayout(cont);
  grid->setSpacing(10);
  QJsonObject txts = data["text"].toObject();
  QJsonObject spellsSlotsData = data["spells"].toObject();
  for (int i = 0; i <= 9; ++i) {
    QString key = QString("spells-level-%1").arg(i);
    QString slotKey = QString("slots-%1").arg(i);
    auto *box = new QFrame();
    box->setObjectName("magicBox");
    auto *bl = new QVBoxLayout(box);
    auto *header = new QHBoxLayout();
    header->addWidget(new QLabel(QString("УРОВЕНЬ %1").arg(i)));
    header->addStretch();
    if (i > 0) {
      auto *sb = new QSpinBox();
      sb->setRange(0, 20);
      sb->setValue(spellsSlotsData[slotKey].toObject()["value"].toInt());
      sb->setFixedWidth(40);
      spellSlotSpins[i] = sb;
      auto *circles = new QWidget();
      auto *cL = new QHBoxLayout(circles);
      cL->setContentsMargins(0, 0, 0, 0);
      cL->setSpacing(4);
      spellCirclesLayouts[i] = cL;
      updateSpellSlots(i, sb->value());
      connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this,
              [this, i](int v) { updateSpellSlots(i, v); });
      header->addWidget(new QLabel("Яч:"));
      header->addWidget(sb);
      header->addWidget(circles);

      // Восстанавливаем сохранённое состояние pips (потраченные ячейки).
      // Формат spells.expendedSlots: массив уровней, каждый — массив флагов.
      QJsonArray expended =
          spellsSlotsData["expendedSlots"].toArray();
      if (expended.size() > i) {
        QJsonArray levelPips = expended.at(i).toArray();
        const auto &pips = spellSlotPips.value(i);
        for (int p = 0; p < pips.size() && p < levelPips.size(); ++p) {
          pips.at(p)->setChecked(levelPips.at(p).toBool());
        }
      }
    }
    bl->addLayout(header);
    auto *ed = new QTextEdit();
    ed->setPlainText(tipTapToPlain(txts[key].toObject()["value"].toObject()));
    ed->setMinimumHeight(100);
    magicEdits[key] = ed;
    bl->addWidget(ed);
    grid->addWidget(box, i / 2, i % 2);
  }
  scroll->setWidget(cont);
  l->addWidget(scroll);
  tabs->addTab(tab, "МАГИЯ");
  connect(spellAbilityCombo,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &CharacterSheet::updateSpellCalculations);
}

void CharacterSheet::updateAllCalculations(int) {
  int pbVal = globalProfSpin ? globalProfSpin->value() : 2;
  QMap<QString, int> mods;
  for (auto k : statSpins.keys()) {
    if (!statSpins[k])
      continue;
    int m = std::floor((statSpins[k]->value() - 10) / 2.0);
    mods[k] = m;
    if (modLabels.contains(k) && modLabels[k])
      modLabels[k]->setText(QString("%1%2").arg(m >= 0 ? "+" : "").arg(m));
    int s = m + (savesState[k] ? pbVal : 0);
    if (saveBonusLabels.contains(k) && saveBonusLabels[k])
      saveBonusLabels[k]->setText(
          QString("%1%2").arg(s >= 0 ? "+" : "").arg(s));
  }
  for (auto k : skillSpins.keys()) {
    if (!skillSpins[k] || !skillsState.contains(k))
      continue;
    int m = mods.value(skillsState[k].baseStat, 0);
    int t = m + (skillsState[k].profLevel * pbVal);
    skillSpins[k]->setValue(t);
    if (skillCalcLabels.contains(k) && skillCalcLabels[k])
      skillCalcLabels[k]->setText(QString("(%1)").arg(t >= 0 ? "+" : "") +
                                  QString::number(t));
  }
  if (passivePerceptionLabel) {
    int m = mods.value("wis", 0);
    int t = 10 + m + (skillsState.value("perception").profLevel * pbVal);
    passivePerceptionLabel->setText(QString::number(t));
  }
  if (initEdit) {
    int m = mods.value("dex", 0);
    initEdit->setText(QString("%1%2").arg(m >= 0 ? "+" : "").arg(m));
  }
  refreshWeaponList();
  updateSpellCalculations();
}

void CharacterSheet::updateSpellCalculations() {
  if (!spellAbilityCombo || !spellSaveDCEdit || !spellAttackBonusEdit)
    return;
  QString ac = spellAbilityCombo->currentData().toString();
  if (ac.isEmpty() || !statSpins.contains(ac) || !statSpins[ac])
    return;
  int m = std::floor((statSpins[ac]->value() - 10) / 2.0);
  int pbVal = globalProfSpin ? globalProfSpin->value() : 2;
  spellSaveDCEdit->setText(QString::number(8 + pbVal + m));
  int b = pbVal + m;
  spellAttackBonusEdit->setText((b >= 0 ? "+" : "") + QString::number(b));
}

void CharacterSheet::updateSpellSlots(int l, int c) {
  if (!spellCirclesLayouts.contains(l))
    return;
  auto *lay = spellCirclesLayouts[l];
  while (auto *it = lay->takeAt(0)) {
    if (it->widget())
      delete it->widget();
    delete it;
  }
  spellSlotPips[l].clear(); // старые pip удалены вместе с виджетами
  for (int i = 0; i < c; ++i) {
    auto *btn = new QPushButton();
    btn->setFixedSize(20, 20);
    btn->setCheckable(true);
    btn->setProperty("class", "indicator");
    connectDirtyBtn(btn);
    lay->addWidget(btn);
    spellSlotPips[l].append(btn); // запоминаем для сохранения состояния
  }
}

void CharacterSheet::markDirty() {
  if (!m_loaded)
    return;
  m_autosaveTimer->start(); // перезапуск 3-секундного отсчёта
  m_statusTimer->stop();
  saveStatusLabel->setText("Изменения…");
}

void CharacterSheet::flushSave() {
  m_autosaveTimer->stop();
  saveToFile();
}

// Helper'ы подключения виджетов к markDirty. Используются после создания
// каждого редактируемого поля, чтобы автосейв реагировал на любые правки.
void CharacterSheet::connectDirtyField(QLineEdit *f) {
  connect(f, &QLineEdit::textChanged, this, &CharacterSheet::markDirty);
}
void CharacterSheet::connectDirtySpin(QSpinBox *s) {
  connect(s, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &CharacterSheet::markDirty);
}
void CharacterSheet::connectDirtyEdit(QTextEdit *e) {
  connect(e, &QTextEdit::textChanged, this, &CharacterSheet::markDirty);
}
void CharacterSheet::connectDirtyCheck(QCheckBox *c) {
  connect(c, &QCheckBox::toggled, this, &CharacterSheet::markDirty);
}
void CharacterSheet::connectDirtyBtn(QPushButton *b) {
  connect(b, &QPushButton::clicked, this, &CharacterSheet::markDirty);
}

void CharacterSheet::applyThemeStyle() {
  // Scoped-стили только для конкретных классов/objectName. Важно: НЕ стилизуем
  // generic QFrame — иначе получим "рамки в рамках". Все цвета — роли палитры,
  // поэтому светлая/тёмная тема ОС подхватывается автоматически.
  setStyleSheet(
      // Плавающие панели блоков на palette(base) с тонкой границей palette(mid).
      "QFrame#cardPanel, QFrame#attacksFrame, QFrame#magicBox { "
      "background-color: palette(base); border: 1px solid palette(mid); "
      "border-radius: 8px; }"
      // Боевые крупные значения (КД, текущие хиты) — без рамки, акцент шрифтом.
      "QLineEdit#combatStatValue, QSpinBox#combatStatValue { font-size: 1.5em; "
      "font-weight: bold; border: none; background: transparent; }"
      // Кнопка сохранения — акцентная, на palette(highlight).
      "QPushButton#saveBtn { background-color: palette(highlight); "
      "color: palette(highlighted-text); font-weight: bold; border-radius: 4px; }"
      // Вторичные подписи.
      "QLabel.sublabel { font-size: 0.7em; color: palette(link); }"
      "QLabel.small-label { font-size: 0.6em; color: palette(link); }"
      // Круглые индикаторы: спасброски, владение навыками, ячейки заклинаний.
      "QPushButton.indicator { border: 1px solid palette(mid); border-radius: "
      "10px; background: transparent; }"
      "QPushButton.indicator:checked { background-color: palette(text); }"
      // Индикаторы провала спасбросков смерти — акцент palette(highlight).
      "QPushButton.indicator-fail { border: 1px solid palette(mid); "
      "border-radius: 10px; background: transparent; }"
      "QPushButton.indicator-fail:checked { background-color: palette(highlight); "
      "border-color: palette(highlight); }"
      // Владение навыком: 1 — palette(text), 2 — palette(highlight).
      "QPushButton#prof1 { background-color: palette(text); }"
      "QPushButton#prof2 { background-color: palette(highlight); "
      "border-color: palette(highlight); }"
      // Карточка оружия в списке — ненавязчивая.
      "QPushButton#weaponCard { background: transparent; border: none; "
      "border-bottom: 1px solid palette(mid); padding: 2px; border-radius: 4px; "
      "text-align: left; }"
      "QPushButton#weaponCard:hover { background: palette(midlight); }"
      "QLabel.weapon-blob { background-color: palette(alternate-base); "
      "border-radius: 4px; padding: 2px 8px; color: palette(text); "
      "font-size: 0.9em; }"
      // Мелкие кнопки управления размером блока атак.
      "QPushButton#smallControl { background: palette(button); "
      "border: 1px solid palette(mid); border-radius: 4px; font-size: 12px; "
      "padding: 2px; min-width: 20px; }");
}

QString CharacterSheet::tipTapToPlain(const QJsonObject &o) {
  QJsonArray c;
  if (o.contains("data"))
    c = o["data"].toObject()["content"].toArray();
  else
    c = o["content"].toArray();
  QString t;
  for (auto v : c) {
    auto p = v.toObject();
    if (p["type"].toString() == "paragraph") {
      QJsonArray contentArray = p["content"].toArray();
      if (contentArray.isEmpty())
        t += "\n";
      for (auto cv : contentArray)
        t += cv.toObject()["text"].toString();
      t += "\n";
    }
  }
  return t;
}

QJsonObject CharacterSheet::plainToTipTap(const QString &text) {
  QJsonArray c;
  for (const QString &l : text.split("\n")) {
    QJsonObject t;
    t["type"] = "text";
    t["text"] = l;
    QJsonObject p;
    p["type"] = "paragraph";
    if (!l.isEmpty())
      p["content"] = QJsonArray{t};
    else
      p["content"] = QJsonArray();
    c.append(p);
  }
  QJsonObject d;
  d["type"] = "doc";
  d["content"] = c;
  QJsonObject r;
  r["data"] = d;
  return r;
}

void CharacterSheet::saveToFile() {
  QString path = m_document->getFilePath();
  if (path.isEmpty()) {
    path = QFileDialog::getSaveFileName(this, "Сохранить чарник",
                                        Storage::charactersDir(), "*.json");
    if (path.isEmpty())
      return;
    m_document->setFilePath(path);
  }

  QJsonObject d = m_document->getData();
  d["name"] = QJsonObject{{"value", nameEditField->text()}};
  d["proficiency"] = globalProfSpin->value();
  d["casterClass"] = QJsonObject{{"value", spellClassEdit->text()}};
  QJsonObject i = d["info"].toObject();
  i["charClass"] = QJsonObject{{"value", classEdit->text()}};
  i["charSubclass"] = QJsonObject{{"value", subclassEdit->text()}};
  i["background"] = QJsonObject{{"value", backgroundEdit->text()}};
  i["race"] = QJsonObject{{"value", raceEdit->text()}};
  i["alignment"] = QJsonObject{{"value", alignmentEdit->text()}};
  i["experience"] = QJsonObject{{"value", expEdit->text()}};
  i["playerName"] = QJsonObject{{"value", playerNameEdit->text()}};
  i["level"] = QJsonObject{{"value", levelSpin->value()}};
  d["info"] = i;

  // Вдохновение (inspiration) — раньше не сохранялось (аудит: несохраняемые поля).
  d["inspiration"] = inspirationCheck->isChecked();

  QJsonObject st = d["stats"].toObject();
  for (auto k : statSpins.keys()) {
    QJsonObject s = st[k].toObject();
    s["score"] = statSpins[k]->value();
    st[k] = s;
  }
  d["stats"] = st;
  QJsonObject svs = d["saves"].toObject();
  for (auto k : saveProfBtns.keys()) {
    QJsonObject s = svs[k].toObject();
    s["isProf"] = saveProfBtns[k]->isChecked();
    svs[k] = s;
  }
  d["saves"] = svs;
  QJsonObject sks = d["skills"].toObject();
  for (auto k : skillsState.keys()) {
    QJsonObject s = sks[k].toObject();
    s["isProf"] = skillsState[k].profLevel;
    sks[k] = s;
  }
  d["skills"] = sks;
  QJsonObject v = d["vitality"].toObject();
  v["ac"] = QJsonObject{{"value", acSpin->value()}};
  v["hp-max"] = QJsonObject{{"value", hpMaxSpin->value()}};
  // Фикс #1 (критический): hp-current/hp-temp сохранялись как голый int,
  // а загрузка (CharacterCard::loadLssJson) ожидает объект {"value":X}.
  // После пересохранения HP всегда грузился как 0. Теперь консистентно.
  v["hp-current"] = QJsonObject{{"value", hpCurrentSpin->value()}};
  v["hp-temp"] = QJsonObject{{"value", hpTempSpin->value()}};
  v["initiative"] = QJsonObject{{"value", initEdit->text()}};
  v["speed"] = QJsonObject{{"value", speedEdit->text()}};
  v["hit-die"] = QJsonObject{{"value", hdValueEdit->text()}};
  d["vitality"] = v;

  // Спасброски смерти — раньше не сохранялись (аудит: несохраняемые поля).
  QJsonArray deathSucc;
  for (auto *btn : deathSavesSuccess)
    deathSucc.append(btn->isChecked());
  QJsonArray deathFail;
  for (auto *btn : deathSavesFail)
    deathFail.append(btn->isChecked());
  d["deathSaves"] = QJsonObject{{"success", deathSucc}, {"fail", deathFail}};

  QJsonObject t = d["text"].toObject();
  t["personality"] = plainToTipTap(traitsEdit->toPlainText());
  t["ideals"] = plainToTipTap(idealsEdit->toPlainText());
  t["bonds"] = plainToTipTap(bondsEdit->toPlainText());
  t["flaws"] = plainToTipTap(flawsEdit->toPlainText());
  t["traits"] = plainToTipTap(featuresEdit->toPlainText());
  t["equipment"] = plainToTipTap(inventoryEdit->toPlainText());
  for (auto k : magicEdits.keys())
    t[k] = plainToTipTap(magicEdits[k]->toPlainText());
  d["text"] = t;

  QJsonArray curWeps;
  for (const auto &wd : weaponsData) {
    QJsonObject wo;
    wo["name"] = QJsonObject{{"value", wd.name}};
    wo["dmg"] = QJsonObject{{"value", wd.damage}};
    wo["ability"] = wd.ability;
    wo["isProf"] = wd.isProf;
    wo["modBonus"] = QJsonObject{{"value", wd.magicBonus}};
    curWeps.append(wo);
  }
  d["weaponsList"] = curWeps;

  QJsonObject sps = d["spells"].toObject();
  for (auto k : spellSlotSpins.keys()) {
    QJsonObject s = sps[QString("slots-%1").arg(k)].toObject();
    s["value"] = spellSlotSpins[k]->value();
    sps[QString("slots-%1").arg(k)] = s;
  }
  // Потраченные ячейки заклинаний (pips) — раньше не сохранялись.
  // Формат: expendedSlots — массив уровней, каждый уровень — массив флагов.
  QJsonArray expendedSlots;
  for (int lvl = 0; lvl <= 9; ++lvl) {
    QJsonArray levelPips;
    const auto &pips = spellSlotPips.value(lvl);
    for (auto *btn : pips)
      levelPips.append(btn->isChecked());
    expendedSlots.append(levelPips);
  }
  sps["expendedSlots"] = expendedSlots;
  d["spells"] = sps;

  m_document->updateFullData(d);
  if (m_document->save()) {
    emit saved(path);       // сообщаем MainWindow перезагрузить карточки
    // Индикатор автосейва: сброс таймеров, краткая надпись "Сохранено ✓".
    m_autosaveTimer->stop();
    saveStatusLabel->setText("Сохранено ✓");
    m_statusTimer->start(); // через 2с индикатор очистится
  } else {
    QMessageBox::warning(this, "Ошибка сохранения",
                         "Не удалось записать файл:\n" + path);
  }
}

void CharacterSheet::toggleSkillProficiency(const QString &k) {
  int &lvl = skillsState[k].profLevel;
  lvl = (lvl + 1) % 3;
  if (lvl == 1)
    skillProfBtns[k]->setObjectName("prof1");
  else if (lvl == 2)
    skillProfBtns[k]->setObjectName("prof2");
  else
    skillProfBtns[k]->setObjectName("");
  skillProfBtns[k]->style()->unpolish(skillProfBtns[k]);
  skillProfBtns[k]->style()->polish(skillProfBtns[k]);
  updateAllCalculations();
}

void CharacterSheet::toggleSaveProficiency(const QString &k) {
  savesState[k] = !savesState[k];
  saveProfBtns[k]->setChecked(savesState[k]);
  updateAllCalculations();
}

CharacterSheet::~CharacterSheet() {}