#include "TrackerColumn.h"
#include "CharacterCard.h"
#include <QDragEnterEvent>
#include <QFile>
#include <QFont>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMimeData>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <algorithm>
#include <vector>

TrackerColumn::TrackerColumn(const QString &title, QWidget *parent)
    : QFrame(parent) {
  setAcceptDrops(true); // Разрешаем сброс карточек и файлов в эту область

  // Адаптивная ширина: никакого setFixedWidth. Колонка подстраивается под
  // содержимое (самую широкую карточку), но не уже комфортного минимума.
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  setMinimumWidth(280);
  setFrameShape(QFrame::NoFrame);
  setAutoFillBackground(true);

  auto *l = new QVBoxLayout(this);
  l->setContentsMargins(8, 8, 8, 8);
  l->setSpacing(8);

  auto *header = new QHBoxLayout();
  header->setSpacing(6);

  auto *sortBtn = new QPushButton("⇅");
  sortBtn->setFixedSize(28, 28);
  connect(sortBtn, &QPushButton::clicked, this, &TrackerColumn::sortInitiative);

  titleEdit = new QLineEdit(title);
  titleEdit->setAlignment(Qt::AlignCenter);
  QFont titleFont = titleEdit->font();
  titleFont.setPointSize(14);
  titleFont.setBold(true);
  titleEdit->setFont(titleFont);
  titleEdit->setFrame(false);

  auto *delCol = new QPushButton("×");
  delCol->setFixedSize(28, 28);
  QFont delFont = delCol->font();
  delFont.setPointSize(16);
  delFont.setBold(true);
  delCol->setFont(delFont);
  connect(delCol, &QPushButton::clicked, this, &TrackerColumn::deleteLater);

  header->addWidget(sortBtn);
  header->addWidget(titleEdit);
  header->addWidget(delCol);
  l->addLayout(header);

  auto *btn = new QPushButton("+ Персонаж");
  QFont addFont = btn->font();
  addFont.setBold(true);
  btn->setFont(addFont);
  connect(btn, &QPushButton::clicked, this, &TrackerColumn::addCharacter);

  scrollArea = new QScrollArea();
  scrollArea->setWidgetResizable(true);
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setStyleSheet("background: transparent;");

  QWidget *c = new QWidget();
  listLayout = new QVBoxLayout(c);
  listLayout->setAlignment(Qt::AlignTop);
  listLayout->setSpacing(10);
  listLayout->setContentsMargins(5, 5, 5, 5);
  scrollArea->setWidget(c);

  l->addWidget(btn);
  l->addWidget(scrollArea);
  addCharacter();
}

// Настройка сигналов карточки: проброс запроса чарника и уведомление об
// изменении привязки.
void TrackerColumn::setupCard(CharacterCard *card) {
  connect(card, &CharacterCard::sheetRequested, this,
          &TrackerColumn::sheetRequested);
  connect(card, &CharacterCard::documentRequested, this,
          &TrackerColumn::documentRequested);
  connect(card, &CharacterCard::bindingChanged, this,
          &TrackerColumn::contentsChanged);
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
  emit contentsChanged();
}

void TrackerColumn::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->hasFormat("application/x-charactercard") ||
      event->mimeData()->hasFormat("application/x-character-filepath"))
    event->acceptProposedAction();
}

void TrackerColumn::dragMoveEvent(QDragMoveEvent *event) {
  event->acceptProposedAction();
}

// Обработка события "броска" карточки (Drop).
// Поддерживает два MIME-типа:
//  1) application/x-charactercard — перемещение существующей карточки (ptr)
//  2) application/x-character-filepath — новый персонаж из хранилища (filePath)
void TrackerColumn::dropEvent(QDropEvent *event) {
  const QMimeData *mime = event->mimeData();

  // --- Случай 1: перемещение существующей карточки ---
  if (mime->hasFormat("application/x-charactercard")) {
    qintptr ptr =
        mime->data("application/x-charactercard").toLongLong();
    auto *card = reinterpret_cast<CharacterCard *>(ptr);

    // Фикс #5 (UAF-risk): проверяем, что это вообще QObject и при том CharacterCard.
    if (!card) {
      event->ignore();
      return;
    }
    if (!qobject_cast<CharacterCard *>(card)) {
      event->ignore();
      return;
    }

    // Конвертируем позицию в систему координат контента скролла.
    const QPoint posInContent =
        scrollArea->widget()->mapFrom(this, event->position().toPoint());

    int index = 0;
    for (int i = 0; i < listLayout->count(); ++i) {
      QWidget *w = listLayout->itemAt(i)->widget();
      if (w && posInContent.y() > w->geometry().center().y())
        index = i + 1;
    }

    listLayout->insertWidget(index, card);
    card->animateAppearance();
    event->acceptProposedAction();
    emit contentsChanged();
    return;
  }

  // --- Случай 2: новый персонаж из хранилища (drag из CharacterRepository) ---
  if (mime->hasFormat("application/x-character-filepath")) {
    const QString filePath =
        QString::fromUtf8(mime->data("application/x-character-filepath"));

    const QPoint posInContent =
        scrollArea->widget()->mapFrom(this, event->position().toPoint());

    int index = 0;
    for (int i = 0; i < listLayout->count(); ++i) {
      QWidget *w = listLayout->itemAt(i)->widget();
      if (w && posInContent.y() > w->geometry().center().y())
        index = i + 1;
    }

    insertCharacter(index, filePath);
    event->acceptProposedAction();
    return;
  }

  event->ignore();
}

void TrackerColumn::addCharacter() {
  auto *card = new CharacterCard(this);
  setupCard(card);
  listLayout->addWidget(card);
  emit contentsChanged();
}

void TrackerColumn::insertCharacter(int index, const QString &filePath, const QJsonObject &ephemeralState) {
  auto *card = new CharacterCard(this);
  setupCard(card);

  if (index >= 0 && index < listLayout->count())
    listLayout->insertWidget(index, card);
  else
    listLayout->addWidget(card);

  if (!filePath.isEmpty()) {
    emit documentRequested(card, filePath);
  }
  
  if (!ephemeralState.isEmpty()) {
    card->setEphemeralState(ephemeralState);
  }

  emit contentsChanged();
}

void TrackerColumn::loadCharacter(const QString &filePath, const QJsonObject &ephemeralState) {
  insertCharacter(-1, filePath, ephemeralState);
}

QString TrackerColumn::getTitle() const { return titleEdit->text(); }

QList<CharacterCard *> TrackerColumn::getCards() const {
  QList<CharacterCard *> cards;
  for (int i = 0; i < listLayout->count(); ++i) {
    if (auto *card =
            qobject_cast<CharacterCard *>(listLayout->itemAt(i)->widget()))
      cards.append(card);
  }
  return cards;
}

TrackerColumn::~TrackerColumn() {}
