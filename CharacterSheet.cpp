#include "CharacterSheet.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <cmath>

CharacterSheet::CharacterSheet(const QJsonObject &root, const QJsonObject &data,
                               QWidget *parent)
    : QDialog(parent), originalRoot(root), originalData(data) {

  setWindowTitle("Редактор персонажа");
  resize(1000, 950);
  setStyleSheet("font-family: 'Segoe UI', sans-serif;");

  auto *mainLayout = new QVBoxLayout(this);

  // ВЕРХНЯЯ ПАНЕЛЬ (Имя и Мастерство)
  auto *topPanel = new QHBoxLayout();

  nameEditField = new QLineEdit(data["name"].toObject()["value"].toString());
  nameEditField->setStyleSheet("font-size: 20px; font-weight: bold; border: "
                               "2px solid palette(mid); padding: 5px; color: "
                               "palette(text); background: palette(base);");
  topPanel->addWidget(new QLabel("<b>ИМЯ:</b>"));
  topPanel->addWidget(nameEditField, 1);

  topPanel->addWidget(new QLabel("<b>БОНУС МАСТЕРСТВА:</b>"));
  globalProfSpin = new QSpinBox();
  globalProfSpin->setRange(1, 12);
  globalProfSpin->setValue(data["proficiency"].toInt());
  globalProfSpin->setPrefix("+");
  globalProfSpin->setFixedWidth(80);
  globalProfSpin->setStyleSheet(
      "font-size: 18px; font-weight: bold; border: 2px solid palette(mid); "
      "border-radius: 5px; padding: 5px; color: palette(text); background: "
      "palette(base);");
  connect(globalProfSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &CharacterSheet::updateAllCalculations);
  topPanel->addWidget(globalProfSpin);

  auto *saveBtn = new QPushButton("💾 СОХРАНИТЬ");
  saveBtn->setStyleSheet("border: 2px solid palette(mid); "
                         "font-weight: bold; padding: 10px; color: "
                         "palette(button-text); background: palette(button);");
  connect(saveBtn, &QPushButton::clicked, this, &CharacterSheet::saveToFile);
  topPanel->addWidget(saveBtn);

  mainLayout->addLayout(topPanel);

  tabs = new QTabWidget(this);
  // Настройка стилей вкладок через таблицу стилей
  tabs->setStyleSheet(
      "QTabWidget::pane { border: 2px solid palette(mid); }"
      "QTabBar::tab { border: 2px solid palette(mid); "
      "border-bottom: none; padding: "
      "12px; font-weight: bold; min-width: "
      "100px; color: palette(window-text); background: palette(window); }");

  setupGeneralTab(data);
  setupSkillsTab(data);
  setupCombatTab(data);
  setupMagicTab(data);
  setupNotesTab(data);

  mainLayout->addWidget(tabs);

  // Первый запуск расчетов
  updateAllCalculations();
}

// Конвертация формата TipTap (JSON документ) в простой текст
// Извлекает текст из параграфов, игнорируя сложное форматирование
QString CharacterSheet::tipTapToPlain(const QJsonObject &obj) {
  QString t;
  QJsonArray c;

  // Вариант 1: obj содержит ключ "data", внутри которого "content"
  if (obj.contains("data") && obj["data"].toObject().contains("content")) {
    c = obj["data"].toObject()["content"].toArray();
  }
  // Вариант 2: obj сам является документом и содержит "content"
  else if (obj.contains("content")) {
    c = obj["content"].toArray();
  }
  // Вариант 3: если это строка? (нет, входной параметр QJsonObject)
  // Если ничего не нашли - пустой массив

  for (auto bV : c) {
    auto b = bV.toObject();
    if (b["type"].toString() == "paragraph") {
      bool first = true;
      for (auto cV : b["content"].toArray()) {
        t += cV.toObject()["text"].toString();
        first = false;
      }
      t += "\n";
    }
  }
  return t;
}

// Расчет модификатора характеристики по правилам D&D 5e
// Формула: (Значение - 10) / 2, с округлением вниз
int CharacterSheet::calculateMod(int score) {
  return std::floor((score - 10) / 2.0);
}

// Обновление всех вычисляемых полей при изменении характеристик или мастерства
void CharacterSheet::updateAllCalculations(int) {
  int pb = globalProfSpin->value(); // Бонус мастерства
  QMap<QString, int> currentMods;

  // 1. Считаем модификаторы характеристик и Спасброски
  for (auto it = statSpins.begin(); it != statSpins.end(); ++it) {
    int score = it.value()->value();
    int mod = std::floor((score - 10) / 2.0);
    currentMods[it.key()] = mod;

    // Обновляем текст модификатора (+5)
    modLabels[it.key()]->setText(
        QString("%1%2").arg(mod >= 0 ? "+" : "").arg(mod));

    // Обновляем итоговый бонус спасброска
    int saveTotal = mod + (savesState[it.key()] ? pb : 0);
    saveBonusLabels[it.key()]->setText(
        QString("Спас: %1%2").arg(saveTotal >= 0 ? "+" : "").arg(saveTotal));
  }

  // 2. Считаем навыки (Мод стата + PB * Уровень владения)
  for (auto it = skillSpins.begin(); it != skillSpins.end(); ++it) {
    QString key = it.key();
    int statMod = currentMods[skillsState[key].baseStat];
    int masteryPart = skillsState[key].profLevel * pb;
    int total = statMod + masteryPart;

    it.value()->setValue(total);

    if (skillCalcLabels.contains(key)) {
      skillCalcLabels[key]->setText(
          QString("<font color='#777'>(%1%2 ст, %3%4 мст)</font>")
              .arg(statMod >= 0 ? "+" : "")
              .arg(statMod)
              .arg(masteryPart >= 0 ? "+" : "")
              .arg(masteryPart));
    }
  }

  // 3. Считаем попадание оружия
  for (auto &w : weaponsUI) {
    int hit = currentMods[w.ability] + (w.isProf ? pb : 0) + w.magicBonus;
    w.hitLabel->setText(QString("+%1").arg(hit));
  }

  // 4. Считаем магию (DC, Bonus)
  updateSpellCalculations();
}

void CharacterSheet::toggleSkillProficiency(const QString &key) {
  int &lvl = skillsState[key].profLevel;
  lvl = (lvl + 1) % 3; // Цикл 0 -> 1 -> 2
  skillProfBtns[key]->setText((lvl == 2) ? "●●" : (lvl == 1 ? "●" : "○"));
  updateAllCalculations();
}

void CharacterSheet::toggleSaveProficiency(const QString &key) {
  savesState[key] = !savesState[key];
  saveProfBtns[key]->setText(savesState[key] ? "● ВЛАДЕЮ" : "○ СПАС");
  updateAllCalculations();
}

QWidget *CharacterSheet::createStatBox(const QString &label, int score,
                                       bool isProfSave,
                                       const QString &statKey) {
  auto *box = new QFrame();
  box->setStyleSheet("border: 2px solid palette(mid); border-radius: 12px; "
                     "background: palette(window);");
  auto *bl = new QVBoxLayout(box);
  bl->setSpacing(2);

  auto *nameLbl = new QLabel(label.toUpper());
  nameLbl->setAlignment(Qt::AlignCenter);
  nameLbl->setStyleSheet("border: none; font-size: 10px; font-weight: bold; "
                         "color: palette(window-text);");

  auto *sb = new QSpinBox();
  sb->setRange(1, 30);
  sb->setValue(score);
  sb->setButtonSymbols(QAbstractSpinBox::NoButtons);
  sb->setAlignment(Qt::AlignCenter);
  sb->setStyleSheet("border: none; font-size: 26px; font-weight: bold; "
                    "background: transparent; color: palette(text);");
  statSpins[statKey] = sb;
  connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &CharacterSheet::updateAllCalculations);

  auto *ml = new QLabel("+0");
  ml->setAlignment(Qt::AlignCenter);
  ml->setStyleSheet("border: none; font-size: 14px; font-weight: bold; color: "
                    "palette(text);");
  modLabels[statKey] = ml;

  auto *saveVal = new QLabel("Спас: +0");
  saveVal->setAlignment(Qt::AlignCenter);
  saveVal->setStyleSheet(
      "border: none; font-size: 12px; color: #d00; font-weight: bold;");
  saveBonusLabels[statKey] = saveVal;

  auto *saveBtn = new QPushButton(isProfSave ? "● ВЛАДЕЮ" : "○ СПАС");
  saveBtn->setStyleSheet(
      "border: 1px solid palette(mid); border-radius: 4px; font-size: 10px; "
      "font-weight: bold; background: palette(button); color: "
      "palette(button-text); padding: 3px;");
  saveProfBtns[statKey] = saveBtn;
  savesState[statKey] = isProfSave;
  connect(saveBtn, &QPushButton::clicked,
          [this, statKey]() { toggleSaveProficiency(statKey); });

  bl->addWidget(nameLbl);
  bl->addWidget(sb);
  bl->addWidget(ml);
  bl->addWidget(saveVal);
  bl->addWidget(saveBtn);
  return box;
}

void CharacterSheet::setupGeneralTab(const QJsonObject &data) {
  auto *tab = new QWidget();
  auto *l = new QVBoxLayout(tab);
  auto *statsLayout = new QHBoxLayout();

  // Загрузка характеристик и спасбросков из JSON
  QJsonObject stats = data["stats"].toObject();
  QJsonObject saves = data["saves"].toObject();

  // Список ключей характеристик в нужном порядке
  QStringList keys = {"str", "dex", "con", "int", "wis", "cha"};

  for (const QString &k : keys) {
    statsLayout->addWidget(
        createStatBox(stats[k].toObject()["label"].toString(),
                      stats[k].toObject()["score"].toInt(),
                      saves[k].toObject()["isProf"].toBool(), k));
  }
  l->addLayout(statsLayout);

  l->addWidget(new QLabel("<b>ОСОБЕННОСТИ И УМЕНИЯ:</b>"));
  traitsEdit = new QTextEdit();
  traitsEdit->setPlainText(tipTapToPlain(
      data["text"].toObject()["traits"].toObject()["value"].toObject()));
  traitsEdit->setStyleSheet(
      "border: 2px solid black; padding: 10px; font-size: 14px;");
  l->addWidget(traitsEdit);

  tabs->addTab(tab, "Основа");
}

void CharacterSheet::setupSkillsTab(const QJsonObject &data) {
  auto *tab = new QWidget();
  auto *l = new QVBoxLayout(tab);
  auto *scroll = new QScrollArea();
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);

  auto *content = new QWidget();
  auto *grid = new QGridLayout(content);
  grid->setColumnStretch(2, 1);

  // Получение списка навыков и их сортировка по алфавиту
  QJsonObject skills = data["skills"].toObject();
  QStringList sorted = skills.keys();
  sorted.sort();
  int row = 0;

  for (const QString &k : sorted) {
    auto s = skills[k].toObject();
    int profLevel = s["isProf"].toInt();
    skillsState[k] = {s["baseStat"].toString(), profLevel};

    auto *btn =
        new QPushButton(profLevel == 2 ? "●●" : (profLevel == 1 ? "●" : "○"));
    btn->setFixedSize(40, 30);
    btn->setStyleSheet(
        "border: 1px solid palette(mid); font-weight: bold; "
        "background: palette(button); color: palette(button-text);");
    skillProfBtns[k] = btn;
    connect(btn, &QPushButton::clicked,
            [this, k]() { toggleSkillProficiency(k); });

    auto *calc = new QLabel();
    skillCalcLabels[k] = calc;

    auto *sb = new QSpinBox();
    sb->setRange(-20, 50);
    sb->setButtonSymbols(QAbstractSpinBox::NoButtons);
    sb->setFixedWidth(50);
    sb->setStyleSheet(
        "border: 1px solid palette(mid); border-radius: 4px; font-weight: "
        "bold; color: palette(text); background: palette(base);");
    skillSpins[k] = sb;

    grid->addWidget(btn, row, 0);
    grid->addWidget(new QLabel(s["label"].toString()), row, 1);
    grid->addWidget(calc, row, 2);
    grid->addWidget(sb, row, 3);
    row++;
  }
  scroll->setWidget(content);
  l->addWidget(scroll);
  tabs->addTab(tab, "Навыки");
}

void CharacterSheet::setupCombatTab(const QJsonObject &data) {
  auto *tab = new QWidget();
  auto *l = new QVBoxLayout(tab);
  auto *scroll = new QScrollArea();
  scroll->setWidgetResizable(true);
  auto *container = new QWidget();
  auto *v = new QVBoxLayout(container);

  v->addWidget(new QLabel("<br><h3>Оружие и атаки</h3>"));
  QJsonArray weapons = data["weaponsList"].toArray();
  for (auto wV : weapons) {
    auto w = wV.toObject();
    auto *f = new QFrame();
    f->setStyleSheet("border: 2px solid palette(mid); border-radius: 20px; "
                     "background: palette(window); margin: 3px;");
    auto *fl = new QHBoxLayout(f);

    auto *nameInp = new QLineEdit(w["name"].toObject()["value"].toString());
    nameInp->setStyleSheet(
        "border: 1px solid palette(mid); border-radius: 12px; "
        "padding: 3px 12px; font-weight: bold; color: palette(text); "
        "background: palette(base);");

    auto *hitLbl = new QLabel("+0");
    hitLbl->setStyleSheet(
        "border: 1px solid palette(mid); border-radius: 10px; "
        "padding: 2px 10px; color: #ff5555; font-weight: bold;");

    auto *dmgInp = new QLineEdit(w["dmg"].toObject()["value"].toString());
    dmgInp->setFixedWidth(100);
    dmgInp->setStyleSheet(
        "border: 1px solid palette(mid); border-radius: 10px; "
        "padding: 2px; color: #ffa500; font-weight: bold; background: "
        "palette(base);");

    weaponsUI.append({nameInp, hitLbl, dmgInp, w["ability"].toString(),
                      w["isProf"].toBool(),
                      w["modBonus"].toObject()["value"].toInt()});

    fl->addWidget(nameInp);
    fl->addStretch();
    fl->addWidget(new QLabel("🎯"));
    fl->addWidget(hitLbl);
    fl->addWidget(new QLabel("💥"));
    fl->addWidget(dmgInp);
    v->addWidget(f);
  }
  scroll->setWidget(container);
  l->addWidget(scroll);
  tabs->addTab(tab, "Бой");
}

// Вспомогательная функция для создания поля шапки с комбобоксом
QWidget *createMagicHeaderCombo(const QString &title, QComboBox *&combo,
                                const QString &currentCode) {
  auto *f = new QFrame();
  f->setStyleSheet("border: 2px solid palette(text); border-radius: 0px; "
                   "background: palette(window);");
  auto *l = new QVBoxLayout(f);
  l->setSpacing(0);
  l->setContentsMargins(5, 2, 5, 2);

  combo = new QComboBox();
  combo->addItem("Интеллект", "int");
  combo->addItem("Мудрость", "wis");
  combo->addItem("Харизма", "cha");

  // Установка текущего значения
  int idx = combo->findData(currentCode);
  if (idx >= 0)
    combo->setCurrentIndex(idx);

  combo->setStyleSheet("border: none; background: transparent; font-size: "
                       "14px; font-weight: bold; color: palette(text);");

  auto *lbl = new QLabel(title);
  lbl->setAlignment(Qt::AlignCenter);
  lbl->setStyleSheet(
      "border: none; font-size: 9px; font-weight: bold; color: "
      "palette(window-text); border-top: 1px solid palette(text);");

  l->addWidget(combo);
  l->addWidget(lbl);
  return f;
}

// Вспомогательная функция для создания поля шапки (обычное текстовое поле)
QWidget *createMagicHeaderBox(const QString &title, QLineEdit *&edit,
                              const QString &val) {
  auto *f = new QFrame();
  f->setStyleSheet("border: 2px solid palette(text); border-radius: 0px; "
                   "background: palette(window);");
  auto *l = new QVBoxLayout(f);
  l->setSpacing(0);
  l->setContentsMargins(5, 2, 5, 2);

  edit = new QLineEdit(val);
  edit->setAlignment(Qt::AlignCenter);
  edit->setStyleSheet("border: none; background: transparent; font-size: 16px; "
                      "font-weight: bold; color: palette(text);");

  auto *lbl = new QLabel(title);
  lbl->setAlignment(Qt::AlignCenter);
  lbl->setStyleSheet(
      "border: none; font-size: 9px; font-weight: bold; color: "
      "palette(window-text); border-top: 1px solid palette(text);");

  l->addWidget(edit);
  l->addWidget(lbl);
  return f;
}

void CharacterSheet::setupMagicTab(const QJsonObject &data) {
  auto *tab = new QWidget();
  auto *mainL = new QVBoxLayout(tab);

  // --- ШАПКА ---
  auto *header = new QHBoxLayout();

  // Данные для шапки
  // 1. Попытка взять из root (originalRoot доступен как член класса)
  // но лучше брать из распаршенного data, так как там лежат поля
  QString cls = data["casterClass"].toObject()["value"].toString();

  // 2. Фолбек на подкласс (для трикстеров и т.д.)
  if (cls.isEmpty()) {
    cls =
        data["info"].toObject()["charSubclass"].toObject()["value"].toString();
  }

  // 3. Фолбек на класс
  if (cls.isEmpty()) {
    cls = data["info"].toObject()["charClass"].toObject()["value"].toString();
  }

  if (cls.isEmpty())
    cls = "Класс заклинателя";

  QJsonObject sInfo = data["spellsInfo"].toObject();
  QString abilityCode = sInfo["base"].toObject()["code"].toString();
  if (abilityCode.isEmpty())
    abilityCode = "int"; // По умолчанию Интеллект

  // DC и Bonus считаем заново или берем сохраненные
  // Но так как у нас теперь авторасчет, мы можем их инициализировать нулями,
  // а потом вызвать updateSpellCalculations() в конце
  QString saveDC = sInfo["save"].toObject()["value"].toString();
  QString bonus = sInfo["mod"].toObject()["value"].toString();

  header->addWidget(
      createMagicHeaderBox("КЛАСС ЗАКЛИНАТЕЛЯ", spellClassEdit, cls), 2);
  header->addSpacing(20);
  header->addWidget(
      createMagicHeaderCombo("БАЗОВАЯ ХАР-КА", spellAbilityCombo, abilityCode),
      1);
  header->addSpacing(10);
  header->addWidget(
      createMagicHeaderBox("СЛ СПАСБРОСКА", spellSaveDCEdit, saveDC), 1);
  spellSaveDCEdit->setReadOnly(true); // Теперь только чтение (авторасчет)
  header->addSpacing(10);
  header->addWidget(
      createMagicHeaderBox("БОНУС АТАКИ", spellAttackBonusEdit, bonus), 1);
  spellAttackBonusEdit->setReadOnly(true); // Только чтение

  // Подключение сигналов авторасчета
  connect(spellAbilityCombo,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &CharacterSheet::updateSpellCalculations);

  mainL->addLayout(header);
  mainL->addSpacing(15);

  // --- ПОЛОТНО ЗАКЛИНАНИЙ (3 КОЛОНКИ) ---
  auto *scroll = new QScrollArea();
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);

  auto *container = new QWidget();
  auto *colsL = new QHBoxLayout(container);
  colsL->setSpacing(15);
  colsL->setAlignment(Qt::AlignTop);

  auto *col1 = new QVBoxLayout();
  col1->setAlignment(Qt::AlignTop);
  auto *col2 = new QVBoxLayout();
  col2->setAlignment(Qt::AlignTop);
  auto *col3 = new QVBoxLayout();
  col3->setAlignment(Qt::AlignTop);

  QJsonObject textData = data["text"].toObject();
  QJsonObject spellsSlotsData = data["spells"].toObject();

  for (int i = 0; i <= 9; ++i) {
    QString key = QString("spells-level-%1").arg(i);
    QString slotKey = QString("slots-%1").arg(i);

    auto *group = new QFrame();
    auto *gl = new QVBoxLayout(group);
    gl->setContentsMargins(0, 0, 0, 10);
    gl->setSpacing(5);

    // Заголовок уровня
    auto *headFrame = new QFrame();
    // Убрали "ублюдочные кружки" (radius 15 -> 4), сделали более строгий стиль
    headFrame->setStyleSheet("border: 1px solid palette(text); border-radius: "
                             "4px; background: palette(window);");
    headFrame->setFixedHeight(36);
    auto *hl = new QHBoxLayout(headFrame);
    hl->setContentsMargins(5, 0, 10, 0);
    hl->setAlignment(Qt::AlignVCenter); // Центрирование всего содержимого

    auto *lvlCircle = new QLabel(QString::number(i));
    lvlCircle->setFixedSize(24, 24);
    lvlCircle->setAlignment(Qt::AlignCenter);
    lvlCircle->setStyleSheet("border: 1px solid palette(text); border-radius: "
                             "12px; font-weight: bold; font-size: 14px;");

    QString title = (i == 0) ? "ЗАГОВОРЫ" : "";
    auto *titleLbl = new QLabel(title);
    titleLbl->setStyleSheet(
        "border: none; font-weight: bold; font-size: 12px;");

    hl->addWidget(lvlCircle);
    hl->addWidget(titleLbl);
    hl->addStretch();

    // Ячейки заклинаний (только для 1-9 уровней)
    if (i > 0) {
      auto *lblTotal = new QLabel("ВСЕГО ЯЧЕЕК");
      lblTotal->setStyleSheet("border: none; font-size: 10px; font-weight: "
                              "bold; margin-right: 5px;");
      hl->addWidget(lblTotal);

      auto *sb = new QSpinBox();
      sb->setRange(0, 99);
      sb->setButtonSymbols(QAbstractSpinBox::NoButtons);
      sb->setFixedSize(30, 24); // Уменьшили и отцентровали
      sb->setAlignment(Qt::AlignCenter);
      sb->setStyleSheet("border: 1px solid palette(text); border-radius: 4px; "
                        "font-weight: bold; font-size: 14px; background: "
                        "palette(base); color: palette(text);");

      // Загрузка значения ячеек
      int maxVal = 0;
      if (spellsSlotsData.contains(slotKey)) {
        maxVal = spellsSlotsData[slotKey].toObject()["value"].toInt();
      }
      sb->setValue(maxVal);
      spellSlotSpins[i] = sb;

      hl->addWidget(sb);

      auto *lblExpended = new QLabel("ПОТРАЧЕНО");
      lblExpended->setStyleSheet("border: none; font-size: 10px; font-weight: "
                                 "bold; margin-left: 10px; margin-right: 5px;");
      hl->addWidget(lblExpended);

      // Контейнер для кружочков
      auto *circlesWidget = new QWidget();
      auto *circlesLayout = new QHBoxLayout(circlesWidget);
      circlesLayout->setContentsMargins(0, 0, 0, 0);
      circlesLayout->setSpacing(2);
      spellCirclesLayouts[i] = circlesLayout;

      // Первоначальное заполнение
      updateSpellSlots(i, maxVal);

      // Подключение обновления
      connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this,
              [this, i](int val) { updateSpellSlots(i, val); });
      hl->addWidget(circlesWidget);
    }

    // Текст заклинаний
    QString spellsText = "";
    if (textData.contains(key)) {
      spellsText = tipTapToPlain(textData[key].toObject()["value"].toObject());
    }
    auto *txt = new QTextEdit();
    txt->setPlainText(spellsText);

    // Вычисляем высоту на основе строк, но не меньше 60
    int lines = spellsText.count('\n') + 1;
    int h = qMax(60, lines * 20 + 20);
    if (h > 400)
      h = 400;
    txt->setFixedHeight(h);

    group->setObjectName("spellGroup");
    headFrame->setObjectName("spellHead");
    txt->setObjectName("spellEdit");

    // Стилизация с использованием селекторов для предотвращения наследования
    group->setStyleSheet(
        "#spellGroup { border: 1px solid palette(mid); border-radius: 4px; "
        "background: palette(window); margin-bottom: 10px; }");
    headFrame->setStyleSheet("#spellHead { border: none; border-bottom: 2px "
                             "solid palette(text); background: transparent; }");
    txt->setStyleSheet("#spellEdit { border: none; font-size: 12px; color: "
                       "palette(text); background: palette(base); }");

    gl->addWidget(headFrame);
    gl->addWidget(txt);

    // Распределение по колонкам
    if (i <= 2)
      col1->addWidget(group);
    else if (i <= 5)
      col2->addWidget(group);
    else
      col3->addWidget(group);
  }

  col1->addStretch();
  col2->addStretch();
  col3->addStretch();

  colsL->addLayout(col1, 1);
  colsL->addLayout(col2, 1);
  colsL->addLayout(col3, 1);

  scroll->setWidget(container);
  mainL->addWidget(scroll);

  tabs->addTab(tab, "Магия");
}

void CharacterSheet::setupNotesTab(const QJsonObject &data) {
  auto *tab = new QWidget();
  auto *l = new QVBoxLayout(tab);

  l->addWidget(new QLabel("<b>ИНВЕНТАРЬ И СНАРЯЖЕНИЕ:</b>"));
  inventoryEdit = new QTextEdit();
  inventoryEdit->setPlainText(
      tipTapToPlain(data["equipment"].toObject()["value"].toObject()));
  inventoryEdit->setStyleSheet("border: 2px solid black; padding: 10px;");
  l->addWidget(inventoryEdit);

  l->addWidget(new QLabel("<br><b>ЗАМЕТКИ ГЕРОЯ:</b>"));
  notesEdit = new QTextEdit();
  notesEdit->setPlainText(
      tipTapToPlain(data["notes-1"].toObject()["value"].toObject()));
  notesEdit->setStyleSheet("border: 2px solid black; padding: 10px;");
  l->addWidget(notesEdit);

  tabs->addTab(tab, "Инфо");
}

// Конвертация простого текста в структуру TipTap (JSON)
// Создает простую структуру paragraphs с text, чтобы LSS мог прочитать данные
QJsonObject CharacterSheet::plainToTipTap(const QString &text) {
  QJsonArray content;
  for (const QString &line : text.split("\n")) {
    if (line.isEmpty())
      continue;
    QJsonObject txt;
    txt["type"] = "text";
    txt["text"] = line;
    QJsonObject p;
    p["type"] = "paragraph";
    p["content"] = QJsonArray{txt};
    content.append(p);
  }
  QJsonObject doc;
  doc["type"] = "doc";
  doc["content"] = content;
  QJsonObject r;
  r["data"] = doc;
  return r;
}

// СОХРАНЕНИЕ
void CharacterSheet::saveToFile() {
  QString path = QFileDialog::getSaveFileName(this, "Экспорт в LSS JSON", "",
                                              "JSON (*.json)");
  if (path.isEmpty())
    return;

  QJsonObject data = originalData;

  // Имя и мастерство
  QJsonObject n = data["name"].toObject();
  n["value"] = nameEditField->text();
  data["name"] = n;
  data["proficiency"] = globalProfSpin->value();

  // Характеристики
  QJsonObject s = data["stats"].toObject();
  for (auto k : statSpins.keys()) {
    QJsonObject st = s[k].toObject();
    st["score"] = statSpins[k]->value();
    s[k] = st;
  }
  data["stats"] = s;

  // Текстовые поля (Особенности, Инвентарь, Заметки)
  QJsonObject t = data["text"].toObject();
  QJsonObject tr = t["traits"].toObject();
  tr["value"] = plainToTipTap(traitsEdit->toPlainText());
  t["traits"] = tr;
  QJsonObject eq = t["equipment"].toObject();
  eq["value"] = plainToTipTap(inventoryEdit->toPlainText());
  t["equipment"] = eq;
  QJsonObject nt = t["notes-1"].toObject();
  nt["value"] = plainToTipTap(notesEdit->toPlainText());
  t["notes-1"] = nt;

  // Сохранение заклинаний
  for (auto it = magicEdits.begin(); it != magicEdits.end(); ++it) {
    QString key = it.key();
    if (!t.contains(key))
      t[key] = QJsonObject();
    QJsonObject spellObj = t[key].toObject();
    spellObj["value"] = plainToTipTap(it.value()->toPlainText());
    t[key] = spellObj;
  }
  data["text"] = t;

  // Сохранение шапки магии
  QJsonObject info = data["info"].toObject();
  if (!info.contains("charClass"))
    info["charClass"] = QJsonObject();
  QJsonObject cc = info["charClass"].toObject();
  cc["value"] = spellClassEdit->text();
  info["charClass"] = cc;
  data["info"] = info;

  QJsonObject sInfo = data["spellsInfo"].toObject();

  if (!sInfo.contains("base"))
    sInfo["base"] = QJsonObject();
  QJsonObject base = sInfo["base"].toObject();
  // Сохраняем код характеристики (int/wis/cha)
  base["code"] = spellAbilityCombo->currentData().toString();
  base["label"] = spellAbilityCombo->currentText();
  sInfo["base"] = base;

  // DC и Bonus считаются автоматически, но сохраняем их как значения
  if (!sInfo.contains("save"))
    sInfo["save"] = QJsonObject();
  QJsonObject save = sInfo["save"].toObject();
  save["value"] = spellSaveDCEdit->text();
  sInfo["save"] = save;

  if (!sInfo.contains("mod"))
    sInfo["mod"] = QJsonObject();
  QJsonObject mod = sInfo["mod"].toObject();
  mod["value"] = spellAttackBonusEdit->text();
  sInfo["mod"] = mod;

  data["spellsInfo"] = sInfo;

  // Сохранение ячеек заклинаний
  QJsonObject spellsObj = data["spells"].toObject();
  for (auto it = spellSlotSpins.begin(); it != spellSlotSpins.end(); ++it) {
    QString key = QString("slots-%1").arg(it.key());
    if (!spellsObj.contains(key))
      spellsObj[key] = QJsonObject();
    QJsonObject slotObj = spellsObj[key].toObject();
    // LSS формат: value - это макс. кол-во ячеек
    // Если мы хотим сохранять текущее состояние потраченных, то нужно поле
    // filled. Но пока сохраняем только максимум, чтобы соответствовать LSS
    // структуре, либо если пользователь меняет, то это Max. В UI у нас
    // отображается Max (так как suffix / N). Давайте сохранять значение как
    // Макс.
    slotObj["value"] = it.value()->value();
    spellsObj[key] = slotObj;
  }
  data["spells"] = spellsObj;

  // Упаковка данных обратно в строковое поле "data" JSON структуры LSS
  QJsonDocument innerDoc(data);
  QJsonObject finalRoot = originalRoot;
  finalRoot["data"] =
      QString::fromUtf8(innerDoc.toJson(QJsonDocument::Compact));

  QFile f(path);
  if (f.open(QIODevice::WriteOnly)) {
    f.write(QJsonDocument(finalRoot).toJson());
    f.close();
  }
}

void CharacterSheet::updateSpellCalculations() {
  // Получаем базовую характеристику (int/wis/cha)
  QString abilityCode = spellAbilityCombo->currentData().toString();
  int score = 10;
  if (statSpins.contains(abilityCode)) {
    score = statSpins[abilityCode]->value();
  }

  // Вычисляем модификатор: (Score - 10) / 2
  int mod = std::floor((score - 10) / 2.0);

  // Бонус мастерства
  int prof = globalProfSpin->value();

  // DC = 8 + Prof + Mod
  int dc = 8 + prof + mod;
  spellSaveDCEdit->setText(QString::number(dc));

  // Attack Bonus = Prof + Mod
  int bonus = prof + mod;
  QString bonusStr = (bonus >= 0 ? "+" : "") + QString::number(bonus);
  spellAttackBonusEdit->setText(bonusStr);
}

void CharacterSheet::updateSpellSlots(int level, int count) {
  if (!spellCirclesLayouts.contains(level))
    return;
  auto *l = spellCirclesLayouts[level];

  // Очистка старых (удаляем виджеты)
  QLayoutItem *child;
  while ((child = l->takeAt(0)) != 0) {
    if (child->widget())
      delete child->widget();
    delete child;
  }

  // Добавление новых (чекбоксы)
  for (int j = 0; j < count; ++j) {
    auto *cb = new QCheckBox();
    cb->setFixedSize(16, 16);
    // Стилизация под кружок (можно доработать, но стандартный чекбокс понятен)
    // В CSS Qt чекбокс indicator можно стилизовать
    cb->setStyleSheet(
        "QCheckBox::indicator { width: 14px; height: 14px; border: 1px solid "
        "palette(text); border-radius: 7px; background: transparent; }"
        "QCheckBox::indicator:checked { background: palette(text); }");
    l->addWidget(cb);
  }
}

CharacterSheet::~CharacterSheet() {}