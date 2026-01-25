#include "TrackerColumn.h"
#include "CharacterCard.h"
#include <QDragEnterEvent>
#include <QFile>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMimeData>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <algorithm>
#include <vector>

TrackerColumn::TrackerColumn(const QString &title, QWidget *parent)
    : QFrame(parent) {
  setFixedWidth(360);
  setAcceptDrops(true); // Разрешаем сброс карточек в эту область

  // Настройка внешнего вида колонки (серый фон, темные границы)
  // Используется setObjectName для стилизации конкретного экземпляра через id
  setObjectName("TrackerColumn");
  setStyleSheet("QFrame#TrackerColumn { background-color: palette(window); "
                "border: 2px solid palette(mid); border-radius: 10px; color: "
                "palette(window-text); }");

  auto *l = new QVBoxLayout(this);
  l->setContentsMargins(10, 10, 10, 10);

  auto *header = new QHBoxLayout();

  auto *sortBtn = new QPushButton("⇅");
  sortBtn->setFixedSize(28, 28);
  sortBtn->setStyleSheet(
      "border: 1px solid palette(mid); font-weight: bold; color: "
      "palette(button-text); background: palette(button);");
  connect(sortBtn, &QPushButton::clicked, this, &TrackerColumn::sortInitiative);

  titleEdit = new QLineEdit(title);
  titleEdit->setAlignment(Qt::AlignCenter);
  titleEdit->setStyleSheet(
      "QLineEdit { font-size: 18px; font-weight: bold; "
      "border: none; background: transparent; color: palette(window-text); }");

  auto *delCol = new QPushButton("×");
  delCol->setFixedSize(28, 28);
  delCol->setStyleSheet(
      "border: none; font-size: 20px; font-weight: bold; "
      "background: transparent; color: palette(window-text);");
  connect(delCol, &QPushButton::clicked, this, &TrackerColumn::deleteLater);

  header->addWidget(sortBtn);
  header->addWidget(titleEdit);
  header->addWidget(delCol);
  l->addLayout(header);

  auto *btn = new QPushButton("+ Персонаж");
  btn->setStyleSheet(
      "border: 1px solid palette(mid); font-weight: bold; padding: 8px; color: "
      "palette(button-text); background: palette(button);");
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
  std::vector<CharacterCard *> cards;
  for (int i = 0; i < listLayout->count(); ++i) {
    if (auto *card =
            qobject_cast<CharacterCard *>(listLayout->itemAt(i)->widget()))
      cards.push_back(card);
  }
  std::sort(cards.begin(), cards.end(), [](CharacterCard *a, CharacterCard *b) {
    return a->getInitiative() > b->getInitiative();
  });

  // Переупорядочивание виджетов в layout путем их удаления и повторного
  // добавления
  for (auto *card : cards) {
    listLayout->removeWidget(card);
    listLayout->addWidget(card);
    card->animateAppearance(); // Анимация для визуального подтверждения
                               // сортировки
  }
}

void TrackerColumn::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->hasFormat("application/x-charactercard"))
    event->acceptProposedAction();
}
void TrackerColumn::dragMoveEvent(QDragMoveEvent *event) {
  event->acceptProposedAction();
}
// Обработка события "броска" карточки (Drop)
void TrackerColumn::dropEvent(QDropEvent *event) {
  // Получение указателя на карточку из MIME-данных
  qintptr ptr =
      event->mimeData()->data("application/x-charactercard").toLongLong();
  CharacterCard *card = reinterpret_cast<CharacterCard *>(ptr);

  if (card) {
    // Определение позиции вставки на основе координаты Y курсора
    int index = 0;
    for (int i = 0; i < listLayout->count(); ++i) {
      QWidget *w = listLayout->itemAt(i)->widget();
      if (w && event->position().y() > w->geometry().center().y())
        index = i + 1;
    }

    // Вставка карточки в новую позицию
    listLayout->insertWidget(index, card);
    card->animateAppearance();
    event->acceptProposedAction();
  }
}

void TrackerColumn::addCharacter() {
  listLayout->addWidget(new CharacterCard(this));
}

void TrackerColumn::loadCharacter(const QString &filePath) {
  auto *card = new CharacterCard(this);
  listLayout->addWidget(card);

  QFile file(filePath);
  if (file.open(QIODevice::ReadOnly)) {
    card->loadLssJson(file.readAll());
    card->setFilePath(filePath);
  }
}
TrackerColumn::~TrackerColumn() {}