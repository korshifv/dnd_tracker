#include "CharacterSheet.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QTabWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileDialog>
#include <QScrollArea>
#include <QFrame>
#include <QSpinBox>
#include <QPushButton>
#include <cmath>

CharacterSheet::CharacterSheet(const QJsonObject &root, const QJsonObject &data, QWidget *parent) 
    : QDialog(parent), originalRoot(root), originalData(data) {
    
    setWindowTitle("Редактор персонажа");
    resize(1000, 950);
    setStyleSheet("background-color: white; color: black; font-family: 'Segoe UI', sans-serif;");

    auto *mainLayout = new QVBoxLayout(this);

    // ВЕРХНЯЯ ПАНЕЛЬ (Имя и Мастерство)
    auto *topPanel = new QHBoxLayout();
    
    nameEditField = new QLineEdit(data["name"].toObject()["value"].toString());
    nameEditField->setStyleSheet("font-size: 20px; font-weight: bold; border: 2px solid black; padding: 5px;");
    topPanel->addWidget(new QLabel("<b>ИМЯ:</b>"));
    topPanel->addWidget(nameEditField, 1);

    topPanel->addWidget(new QLabel("<b>БОНУС МАСТЕРСТВА:</b>"));
    globalProfSpin = new QSpinBox();
    globalProfSpin->setRange(1, 12);
    globalProfSpin->setValue(data["proficiency"].toInt());
    globalProfSpin->setPrefix("+");
    globalProfSpin->setFixedWidth(80);
    globalProfSpin->setStyleSheet("font-size: 18px; font-weight: bold; border: 2px solid black; border-radius: 5px; padding: 5px;");
    connect(globalProfSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &CharacterSheet::updateAllCalculations);
    topPanel->addWidget(globalProfSpin);

    auto *saveBtn = new QPushButton("💾 СОХРАНИТЬ");
    saveBtn->setStyleSheet("background: #ccffcc; border: 2px solid black; font-weight: bold; padding: 10px;");
    connect(saveBtn, &QPushButton::clicked, this, &CharacterSheet::saveToFile);
    topPanel->addWidget(saveBtn);

    mainLayout->addLayout(topPanel);

    tabs = new QTabWidget(this);
    // ПРОБЛЕМА: Сложные CSS селекторы в setStyleSheet - лучше в QSS-файле для переиспользования
    // РЕКОМЕНДАЦИЯ: Вынести стили в отдельный stylesheet файл
    tabs->setStyleSheet(
        "QTabWidget::pane { border: 2px solid black; background: white; }"
        "QTabBar::tab { border: 2px solid black; border-bottom: none; padding: 12px; font-weight: bold; color: black; background: #eee; min-width: 100px; }"
        "QTabBar::tab:selected { background: white; }"
    );

    setupGeneralTab(data);
    setupSkillsTab(data);
    setupCombatTab(data);
    setupNotesTab(data);

    mainLayout->addWidget(tabs);

    // Первый запуск расчетов
    updateAllCalculations();
}

int CharacterSheet::calculateMod(int score) {
    // ПРОБЛЕМА: Использование std::floor для целых чисел - лишний вызов, достаточно целочисленного деления
    // РЕКОМЕНДАЦИЯ: return (score - 10) / 2;  // работает корректнее для отрицательных
    return std::floor((score - 10) / 2.0);
}

void CharacterSheet::updateAllCalculations() {
    // ПРОБЛЕМА: updateAllCalculations() вызывается часто (на каждое изменение), лучше добавить дебаунс
    // ПРОБЛЕМА: Полный пересчет всех навыков каждый раз - можно оптимизировать
    int pb = globalProfSpin->value();
    QMap<QString, int> currentMods;

    // 1. Считаем модификаторы характеристик и Спасброски
    for (auto it = statSpins.begin(); it != statSpins.end(); ++it) {
        int mod = calculateMod(it.value()->value());
        currentMods[it.key()] = mod;
        
        // Обновляем текст модификатора (+5)
        modLabels[it.key()]->setText(QString("%1%2").arg(mod >= 0 ? "+" : "").arg(mod));
        
        // Обновляем итоговый бонус спасброска
        int saveTotal = mod + (savesState[it.key()] ? pb : 0);
        saveBonusLabels[it.key()]->setText(QString("Спас: %1%2").arg(saveTotal >= 0 ? "+" : "").arg(saveTotal));
    }

    // 2. Считаем навыки (Мод стата + PB * Уровень владения)
    for (auto it = skillSpins.begin(); it != skillSpins.end(); ++it) {
        QString key = it.key();
        int statMod = currentMods[skillsState[key].baseStat];
        int masteryPart = skillsState[key].profLevel * pb;
        int total = statMod + masteryPart;
        
        it.value()->setValue(total);

        if (skillCalcLabels.contains(key)) {
            skillCalcLabels[key]->setText(QString("<font color='#777'>(%1%2 ст, %3%4 мст)</font>")
                .arg(statMod >= 0 ? "+" : "").arg(statMod)
                .arg(masteryPart >= 0 ? "+" : "").arg(masteryPart));
        }
    }

    // 3. Считаем попадание оружия
    for (auto &w : weaponsUI) {
        int hit = currentMods[w.ability] + (w.isProf ? pb : 0) + w.magicBonus;
        w.hitLabel->setText(QString("+%1").arg(hit));
    }
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

QWidget* CharacterSheet::createStatBox(const QString &label, int score, bool isProfSave, const QString &statKey) {
    auto *box = new QFrame();
    box->setStyleSheet("border: 2px solid black; border-radius: 12px; background: #fdfdfd;");
    auto *bl = new QVBoxLayout(box);
    bl->setSpacing(2);
    
    auto *nameLbl = new QLabel(label.toUpper());
    nameLbl->setAlignment(Qt::AlignCenter);
    nameLbl->setStyleSheet("border: none; font-size: 10px; font-weight: bold;");

    auto *sb = new QSpinBox();
    sb->setRange(1, 30); sb->setValue(score);
    sb->setButtonSymbols(QAbstractSpinBox::NoButtons);
    sb->setAlignment(Qt::AlignCenter);
    sb->setStyleSheet("border: none; font-size: 26px; font-weight: bold; background: transparent;");
    statSpins[statKey] = sb;
    connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this, &CharacterSheet::updateAllCalculations);

    auto *ml = new QLabel("+0");
    ml->setAlignment(Qt::AlignCenter);
    ml->setStyleSheet("border: none; font-size: 14px; font-weight: bold; color: #444;");
    modLabels[statKey] = ml;

    auto *saveVal = new QLabel("Спас: +0");
    saveVal->setAlignment(Qt::AlignCenter);
    saveVal->setStyleSheet("border: none; font-size: 12px; color: #d00; font-weight: bold;");
    saveBonusLabels[statKey] = saveVal;

    auto *saveBtn = new QPushButton(isProfSave ? "● ВЛАДЕЮ" : "○ СПАС");
    saveBtn->setStyleSheet("border: 1px solid #000; border-radius: 4px; font-size: 10px; font-weight: bold; background: #eee; color: black; padding: 3px;");
    saveProfBtns[statKey] = saveBtn;
    savesState[statKey] = isProfSave;
    connect(saveBtn, &QPushButton::clicked, [this, statKey](){ toggleSaveProficiency(statKey); });

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
    
    // ПРОБЛЕМА: Hardcoded список ключей - хрупко при изменении формата
    // РЕКОМЕНДАЦИЯ: Использовать stats.keys() или константы
    QJsonObject stats = data["stats"].toObject();
    QJsonObject saves = data["saves"].toObject();
    QStringList keys = {"str", "dex", "con", "int", "wis", "cha"};
    
    for(const QString &k : keys) {
        statsLayout->addWidget(createStatBox(stats[k].toObject()["label"].toString(), 
                                            stats[k].toObject()["score"].toInt(),
                                            saves[k].toObject()["isProf"].toBool(), k));
    }
    l->addLayout(statsLayout);

    l->addWidget(new QLabel("<b>ОСОБЕННОСТИ И УМЕНИЯ:</b>"));
    traitsEdit = new QTextEdit();
    traitsEdit->setPlainText(tipTapToPlain(data["text"].toObject()["traits"].toObject()["value"].toObject()));
    traitsEdit->setStyleSheet("border: 2px solid black; padding: 10px; font-size: 14px;");
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
    
    // ПРОБЛЕМА: Создание 18+ виджетов в цикле без кэша - медленно при пересчетах
    QJsonObject skills = data["skills"].toObject();
    QStringList sorted = skills.keys(); sorted.sort();
    int row = 0;
    
    for(const QString &k : sorted) {
        auto s = skills[k].toObject();
        int profLevel = s["isProf"].toInt();
        skillsState[k] = {s["baseStat"].toString(), profLevel};

        auto *btn = new QPushButton(profLevel == 2 ? "●●" : (profLevel == 1 ? "●" : "○"));
        btn->setFixedSize(40, 30);
        btn->setStyleSheet("border: 1px solid black; font-weight: bold; background: #eee; color: black;");
        skillProfBtns[k] = btn;
        connect(btn, &QPushButton::clicked, [this, k](){ toggleSkillProficiency(k); });

        auto *calc = new QLabel();
        skillCalcLabels[k] = calc;

        auto *sb = new QSpinBox();
        sb->setRange(-20, 50); sb->setButtonSymbols(QAbstractSpinBox::NoButtons);
        sb->setFixedWidth(50); sb->setStyleSheet("border: 1px solid black; border-radius: 4px; font-weight: bold;");
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

    v->addWidget(new QLabel("<h3>Ячейки заклинаний</h3>"));
    auto *slotsLayout = new QHBoxLayout();
    QJsonObject spells = data["spells"].toObject();
    for(int i = 1; i <= 3; ++i) {
        QString key = QString("slots-%1").arg(i);
        if(spells.contains(key)) {
            int maxVal = spells[key].toObject()["value"].toInt();
            auto *f = new QFrame();
            f->setStyleSheet("border: 2px solid black; border-radius: 8px; background: #eee;");
            auto *fl = new QVBoxLayout(f);
            fl->addWidget(new QLabel(QString("Круг %1").arg(i)));
            auto *sb = new QSpinBox(); 
            sb->setRange(0, maxVal); sb->setValue(maxVal); // Всегда полные при открытии
            sb->setSuffix(QString(" / %1").arg(maxVal));
            fl->addWidget(sb);
            slotsLayout->addWidget(f);
        }
    }
    v->addLayout(slotsLayout);

    v->addWidget(new QLabel("<br><h3>Оружие и атаки</h3>"));
    QJsonArray weapons = data["weaponsList"].toArray();
    for(auto wV : weapons) {
        auto w = wV.toObject();
        auto *f = new QFrame();
        f->setStyleSheet("border: 2px solid black; border-radius: 20px; background: white; margin: 3px;");
        auto *fl = new QHBoxLayout(f);
        
        auto *nameInp = new QLineEdit(w["name"].toObject()["value"].toString());
        nameInp->setStyleSheet("border: 1px solid black; border-radius: 12px; padding: 3px 12px; font-weight: bold;");
        
        auto *hitLbl = new QLabel("+0");
        hitLbl->setStyleSheet("border: 1px solid black; border-radius: 10px; padding: 2px 10px; color: red; font-weight: bold;");
        
        auto *dmgInp = new QLineEdit(w["dmg"].toObject()["value"].toString());
        dmgInp->setFixedWidth(100);
        dmgInp->setStyleSheet("border: 1px solid black; border-radius: 10px; padding: 2px; color: orange; font-weight: bold;");

        weaponsUI.append({nameInp, hitLbl, dmgInp, w["ability"].toString(), w["isProf"].toBool(), w["modBonus"].toObject()["value"].toInt()});
        
        fl->addWidget(nameInp);
        fl->addStretch();
        fl->addWidget(new QLabel("🎯")); fl->addWidget(hitLbl);
        fl->addWidget(new QLabel("💥")); fl->addWidget(dmgInp);
        v->addWidget(f);
    }
    scroll->setWidget(container);
    l->addWidget(scroll);
    tabs->addTab(tab, "Бой");
}

void CharacterSheet::setupNotesTab(const QJsonObject &data) {
    auto *tab = new QWidget();
    auto *l = new QVBoxLayout(tab);
    
    l->addWidget(new QLabel("<b>ИНВЕНТАРЬ И СНАРЯЖЕНИЕ:</b>"));
    inventoryEdit = new QTextEdit();
    inventoryEdit->setPlainText(tipTapToPlain(data["equipment"].toObject()["value"].toObject()));
    inventoryEdit->setStyleSheet("border: 2px solid black; padding: 10px;");
    l->addWidget(inventoryEdit);

    l->addWidget(new QLabel("<br><b>ЗАМЕТКИ ГЕРОЯ:</b>"));
    notesEdit = new QTextEdit();
    notesEdit->setPlainText(tipTapToPlain(data["notes-1"].toObject()["value"].toObject()));
    notesEdit->setStyleSheet("border: 2px solid black; padding: 10px;");
    l->addWidget(notesEdit);
    
    tabs->addTab(tab, "Инфо");
}
// ПРОБЛЕМА: tipTapToPlain неправильно парсит - не обрабатывает разметку текста (bold, italic и т.д.)
    // РЕКОМЕНДАЦИЯ: Использовать QTextHtmlParser или сохранять как plain-text в LSS
    
// ПАРСИНГ LSS ТЕКСТА
QString CharacterSheet::tipTapToPlain(const QJsonObject &obj) {
    QString t; if(!obj.contains("data")) return "";
    QJsonArray c = obj["data"].toObject()["content"].toArray();
    for(auto bV : c) {
        auto b = bV.toObject();
        if(b["type"].toString() == "paragraph") {
            for(auto cV : b["content"].toArray()) t += cV.toObject()["text"].toString();
            t += "\n";
        }
    }
    // ПРОБЛЕМА: plainToTipTap теряет информацию о форматировании (bold, italic, списки)
    // ПРОБЛЕМА: Пользователь потеряет форматирование при экспорте-импорте
    // РЕКОМЕНДАЦИЯ: Сохранить как plain-text в LSS или полноценно парсить разметку
    return t;
}

// КОНВЕРТАЦИЯ В LSS ТЕКСТ
QJsonObject CharacterSheet::plainToTipTap(const QString &text) {
    QJsonArray content;
    for(const QString &line : text.split("\n")) {
        if(line.isEmpty()) continue;
        QJsonObject txt; txt["type"] = "text"; txt["text"] = line;
        QJsonObject p; p["type"] = "paragraph"; p["content"] = QJsonArray{txt};
        content.append(p);
    // ПРОБЛЕМА: Нет обработки ошибок при сохранении файла
    // ПРОБЛЕМА: Пользователь не узнает, успешно ли сохранилось
    // РЕКОМЕНДАЦИЯ: Добавить QMessageBox с результатом
    }
    QJsonObject doc; doc["type"] = "doc"; doc["content"] = content;
    QJsonObject r; r["data"] = doc;
    return r;
}

// СОХРАНЕНИЕ
void CharacterSheet::saveToFile() {
    QString path = QFileDialog::getSaveFileName(this, "Экспорт в LSS JSON", "", "JSON (*.json)");
    if(path.isEmpty()) return;

    QJsonObject data = originalData;
    
    // Имя и мастерство
    QJsonObject n = data["name"].toObject(); n["value"] = nameEditField->text(); data["name"] = n;
    data["proficiency"] = globalProfSpin->value();

    // Характеристики
    QJsonObject s = data["stats"].toObject();
    for(auto k : statSpins.keys()) { 
        QJsonObject st = s[k].toObject(); 
        st["score"] = statSpins[k]->value(); 
        s[k] = st; 
    }
    data["stats"] = s;

    // Текстовые поля (Особенности, Инвентарь, Заметки)
    QJsonObject t = data["text"].toObject();
    QJsonObject tr = t["traits"].toObject(); tr["value"] = plainToTipTap(traitsEdit->toPlainText()); t["traits"] = tr;
    QJsonObject eq = t["equipment"].toObject(); eq["value"] = plainToTipTap(inventoryEdit->toPlainText()); t["equipment"] = eq;
    QJsonObject nt = t["notes-1"].toObject(); nt["value"] = plainToTipTap(notesEdit->toPlainText()); t["notes-1"] = nt;
    data["text"] = t;

        // ПРОБЛЕМА: Нет проверки f.error() после close()
        // ПРОБЛЕМА: Нет сообщения пользователю об успехе
        // РЕКОМЕНДАЦИЯ: Добавить проверку и QMessageBox::information()
    // Пакем обратно в строку "data"
    QJsonDocument innerDoc(data);
    QJsonObject finalRoot = originalRoot;
    finalRoot["data"] = QString::fromUtf8(innerDoc.toJson(QJsonDocument::Compact));

    QFile f(path); 
    if(f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(finalRoot).toJson());
        f.close();
    }
}

CharacterSheet::~CharacterSheet() {}