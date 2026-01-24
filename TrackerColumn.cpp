#include "TrackerColumn.h"
#include "CharacterCard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QDragEnterEvent>
#include <QMimeData>
#include <algorithm>
#include <vector>

TrackerColumn::TrackerColumn(const QString &title, QWidget *parent) : QFrame(parent) {
    setFixedWidth(360);
    setAcceptDrops(true);
    // ПРОБЛЕМА: setStyleSheet на каждый объект колонки - неоптимально
    // ПРОБЛЕМА: Селектор "TrackerColumn" не срабатывает, нужен #objectName
    // РЕКОМЕНДАЦИЯ: Использовать setObjectName() и централизованные стили
    // Фон колонки серый, чтобы БЕЛЫЕ карточки выделялись
    setStyleSheet("TrackerColumn { background-color: #dcdcdc; border: 2px solid black; border-radius: 10px; }");
    
    auto *l = new QVBoxLayout(this);
    l->setContentsMargins(10, 10, 10, 10);

    auto *header = new QHBoxLayout();
    
    auto *sortBtn = new QPushButton("⇅");
    sortBtn->setFixedSize(28, 28);
    sortBtn->setStyleSheet("background: white; border: 1px solid black; font-weight: bold; color: black;");
    connect(sortBtn, &QPushButton::clicked, this, &TrackerColumn::sortInitiative);

    titleEdit = new QLineEdit(title);
    titleEdit->setAlignment(Qt::AlignCenter);
    titleEdit->setStyleSheet("QLineEdit { color: black; font-size: 18px; font-weight: bold; border: none; background: transparent; }");
    
    auto *delCol = new QPushButton("×");
    delCol->setFixedSize(28, 28);
    delCol->setStyleSheet("border: none; color: black; font-size: 20px; font-weight: bold; background: transparent;");
    connect(delCol, &QPushButton::clicked, this, &TrackerColumn::deleteLater);

    header->addWidget(sortBtn);
    header->addWidget(titleEdit);
    header->addWidget(delCol);
    l->addLayout(header);

    auto *btn = new QPushButton("+ Персонаж");
    btn->setStyleSheet("color: black; background: white; border: 1px solid black; font-weight: bold; padding: 8px;");
    connect(btn, &QPushButton::clicked, this, &TrackerColumn::addCharacter);
    
    auto *s = new QScrollArea();
    s->setWidgetResizable(true);
    s->setFrameShape(QFrame::NoFrame);
    s->setStyleSheet("background: transparent;");

    QWidget *c = new QWidget();
    listLayout = new QVBoxLayout(c);
    listLayout->setAlignment(Qt::AlignTop);
    listLayout->setSpacing(10);
    listLayout->setContentsMargins(5, 5, 5, 5);
    s->setWidget(c);

    l->addWidget(btn);
    l->addWidget(s);
    addCharacter();
}

void TrackerColumn::sortInitiative() {
    std::vector<CharacterCard*> cards;
    for (int i = 0; i < listLayout->count(); ++i) {
        if (auto *card = qobject_cast<CharacterCard*>(listLayout->itemAt(i)->widget()))
            cards.push_back(card);
    }
    std::sort(cards.begin(), cards.end(), [](CharacterCard* a, CharacterCard* b) {
        return a->getInitiative() > b->getInitiative();
    });
    // ПРОБЛЕМА: removeWidget() + addWidget() для каждой карточки - O(n²) операция с лейаутом
    // РЕКОМЕНДАЦИЯ: Использовать insertWidget() или пересоздать лейаут один раз
    // ПРОБЛЕМА: Анимация на каждый элемент - перегруз GPU при большом количестве
    for (auto *card : cards) {
        listLayout->removeWidget(card);
        listLayout->addWidget(card);
        card->animateAppearance();
    }
}

void TrackerColumn::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat("application/x-charactercard")) event->acceptProposedAction();
}
void TrackerColumn::dragMoveEvent(QDragMoveEvent *event) { event->acceptProposedAction(); }
void TrackerColumn::dropEvent(QDropEvent *event) {
    // ПРОБЛЕМА: Использование указателя через MIME-данные КРАЙНЕ ОПАСНО!
    // ПРОБЛЕМА: После удаления карточки в другом месте - получим dangling pointer и краш
    // РЕКОМЕНДАЦИЯ: Использовать QVariant::fromValue() или сохранять ID вместо указателя
    qintptr ptr = event->mimeData()->data("application/x-charactercard").toLongLong();
    CharacterCard *card = reinterpret_cast<CharacterCard*>(ptr);
    if (card) {
        // ПРОБЛЕМА: Линейный поиск O(n) для каждого drop - неэффективно
        int index = 0;
        for (int i = 0; i < listLayout->count(); ++i) {
            QWidget *w = listLayout->itemAt(i)->widget();
            // ПРОБЛЕМА: geometry().center().y() может быть невалидна, если виджет не отрисован
            if (w && event->position().y() > w->geometry().center().y()) index = i + 1;
        }
        listLayout->insertWidget(index, card);
        card->animateAppearance();
        event->acceptProposedAction();
    }
}

void TrackerColumn::addCharacter() { listLayout->addWidget(new CharacterCard(this)); }
TrackerColumn::~TrackerColumn() {}