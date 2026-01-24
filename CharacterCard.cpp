#include "CharacterCard.h"
#include "CharacterSheet.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QJsonDocument>
#include <QRandomGenerator>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QMessageBox>

CharacterCard::CharacterCard(QWidget *parent) : QFrame(parent) {
    // ПРОБЛЕМА: setAttribute() очень дорогая операция. Лучше сделать это один раз в стиле или нарисовать вручную
    // Делаем саму основу прозрачной, чтобы border-radius не давал черных углов
    setAttribute(Qt::WA_TranslucentBackground);
    setObjectName("CharacterCard");
    
    // ПРОБЛЕМА: Многократное применение setStyleSheet для отдельных виджетов переперспарсивает CSS каждый раз
    // РЕКОМЕНДАЦИЯ: Вынести стили в QSS-файл или централизовать в QApplication::setStyleSheet()
    // СТИЛИ: Белая карточка, черные рамки, черный текст
    setStyleSheet(
        "QFrame#CharacterCard { background: transparent; border: none; }"
        "QWidget { color: black; }"
        "QLineEdit { border: 1px solid black; border-radius: 4px; background: white; font-weight: bold; padding: 2px; }"
        "QSpinBox { border: 1px solid black; border-radius: 4px; background: white; padding: 2px; font-weight: bold; }"
        "QPushButton { background: #eee; border: 1px solid black; border-radius: 4px; font-weight: bold; color: black; }"
    );
    setFixedHeight(185);

    // ПРОБЛЕМА: QGraphicsOpacityEffect очень дорога для производительности! Используется GPU в некоторых системах
    // РЕКОМЕНДАЦИЯ: Для простой анимации прозрачности использовать QPainter или просто убрать эффект
    opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityEffect);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(5, 5, 5, 5);

    // ВНУТРЕННИЙ КОРПУС (те самые рамки)
    QFrame *body = new QFrame(this);
    body->setObjectName("cardBody");
    body->setStyleSheet("QFrame#cardBody { background-color: white; border: 2px solid black; border-radius: 12px; }");
    
    auto *mainLayout = new QVBoxLayout(body);
    mainLayout->setContentsMargins(12, 10, 12, 10);
    mainLayout->setSpacing(8);

    // --- ВЕРХНИЙ РЯД: Аватар, Инициатива, Имя, LSS ---
    auto *topLayout = new QHBoxLayout();
    
    // ПРОБЛЕМА: avatarBtn не хранится как member-переменная, но позже нельзя будет изменить цвет
    QPushButton *avatarBtn = new QPushButton();
    avatarBtn->setFixedSize(32, 32);
    setRandomColor(avatarBtn); // Рандомный цвет кружка
    connect(avatarBtn, &QPushButton::clicked, this, &CharacterCard::showFullSheet);
    
    initSpin = new QSpinBox();
    initSpin->setRange(-20, 99);
    initSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    initSpin->setFixedWidth(35);
    initSpin->setToolTip("Инициатива");

    nameEdit = new QLineEdit("Персонаж");

    QPushButton *lssBtn = new QPushButton("LSS");
    lssBtn->setFixedSize(32, 22);
    lssBtn->setStyleSheet("font-size: 9px; background: #ffffcc; border: 1px solid black;");
    lssBtn->setToolTip("Импортировать JSON из LSS");
    connect(lssBtn, &QPushButton::clicked, this, &CharacterCard::openLssFile);

    auto *delBtn = new QPushButton("×");
    delBtn->setFixedSize(22, 22);
    delBtn->setStyleSheet("border: none; font-size: 18px; font-weight: bold; background: transparent;");
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
    acLabel->setStyleSheet("font-weight: bold; border: 1px solid black; border-radius: 4px; padding: 2px 4px; background: #f0f0f0;");

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
    QString path = QFileDialog::getOpenFileName(this, "Выбрать чарник LSS", "", "JSON Files (*.json)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        loadLssJson(file.readAll());
    }
}

void CharacterCard::loadLssJson(const QByteArray &rawData) {
    // ПРОБЛЕМА: Нет обработки ошибок парсинга JSON - просто молча возвращаемся
    // РЕКОМЕНДАЦИЯ: Показать пользователю сообщение об ошибке
    QJsonDocument doc = QJsonDocument::fromJson(rawData);
    if (doc.isNull()) return;

    rootLssJson = doc.object(); // Сохраняем весь файл целиком для будущего экспорта
    
    // Двойной парсинг LSS: вытаскиваем строку data и парсим её как объект
    // Это необходимо для совместимости с форматом LSS, где персональные данные хранятся как JSON-строка
    QString innerJsonStr = rootLssJson["data"].toString();
    QJsonDocument innerDoc = QJsonDocument::fromJson(innerJsonStr.toUtf8());
    if (innerDoc.isNull()) return;

    characterData = innerDoc.object();

    // Заполняем поля карточки
    nameEdit->setText(characterData["name"].toObject()["value"].toString());
    
    QJsonObject vitality = characterData["vitality"].toObject();
    hpSpin->setValue(vitality["hp-current"].toObject()["value"].toInt());
    
    int ac = vitality["ac"].toObject()["value"].toInt();
    acLabel->setText(QString("КД: %1").arg(ac));
    
    int init = vitality["initiative"].toObject()["value"].toInt();
    if (init != 0) initSpin->setValue(init);

    QMessageBox::information(this, "Успех", "Персонаж " + nameEdit->text() + " импортирован!");
}

void CharacterCard::showFullSheet() {
    if (characterData.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Сначала импортируйте JSON файл персонажа!");
        return;
    }
    // Вызываем чарник, передавая и корень файла (для сейва) и данные персонажа
    CharacterSheet *sheet = new CharacterSheet(rootLssJson, characterData, this);
    sheet->show();
}// ПРОБЛЕМА: setStyleSheet дорогая операция, лучше использовать setPalette() для цвета
    // ПРОБЛЕМА: Пользователь не может повторно сгенерировать цвет - нет связи с UI
    

void CharacterCard::setRandomColor(QWidget* widget) {
    int r = QRandomGenerator::global()->bounded(160, 256);
    int g = QRandomGenerator::global()->bounded(160, 256);
    // ПРОБЛЕМА: QPropertyAnimation на каждый виджет создает новый объект, не переиспользует
    // ПРОБЛЕМА: DeleteWhenStopped не гарантирует удаление при краше (утечка памяти в редких случаях)
    int b = QRandomGenerator::global()->bounded(160, 256);
    widget->setStyleSheet(QString("border: 2px solid black; border-radius: 16px; background-color: rgb(%1,%2,%3);").arg(r).arg(g).arg(b));
}

void CharacterCard::animateAppearance() {
    QPropertyAnimation *anim = new QPropertyAnimation(opacityEffect, "opacity");
    anim->setDuration(300);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

int CharacterCard::getInitiative() const { return initSpin->value(); }

void CharacterCard::applyDamage() {
    hpSpin->setValue(hpSpin->value() - modSpin->value());
    modSpin->setValue(0);
}

voiПРОБЛЕМА: mousePressEvent объявлена, но не используется для реального Drag&Drop
// РЕКОМЕНДАЦИЯ: Либо реализовать его (QDrag + setMimeData), либо удалить заглушку
    hpSpin->setValue(hpSpin->value() + modSpin->value());
    modSpin->setValue(0);
}

// Заглушка для мыши (нужна для корректной работы Drag&Drop в будущем)
void CharacterCard::mousePressEvent(QMouseEvent *event) {
    QFrame::mousePressEvent(event);
}

CharacterCard::~CharacterCard() {}