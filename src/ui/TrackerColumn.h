#ifndef TRACKERCOLUMN_H
#define TRACKERCOLUMN_H
#include <QFrame>
#include <QJsonObject>

class QVBoxLayout;
class QLineEdit;
class QScrollArea;
class CharacterCard;

// Виджет колонки трекера
// Отвечает за отобажение группы персонажей (например, Игроки или Враги) и
// управление ими
class TrackerColumn : public QFrame {
  Q_OBJECT
public:
  explicit TrackerColumn(const QString &title, QWidget *parent = nullptr);
  ~TrackerColumn();

signals:
  // Проброс запроса открытия чарника от карточки выше к InitiativeTracker.
  void sheetRequested(const QString &filePath);
  // Проброс запроса на привязку документа от карточки
  void documentRequested(CharacterCard* card, const QString &filePath);
  // Сигнал об изменении содержимого колонки (добавление/удаление/перемещение
  // карточек, привязка персонажа). InitiativeTracker debounce-сохраняет
  // состояние.
  void contentsChanged();

protected:
  // События Drag & Drop для перетаскивания карточек между колонками
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dragMoveEvent(QDragMoveEvent *event) override;
  void dropEvent(QDropEvent *event) override;

public slots:
  // Добавить новую пустую карточку персонажа в колонку
  void addCharacter();
  // Загрузить персонажа из файла (используется при восстановлении сессии).
  // Если index >= 0 — вставляет в указанную позицию, иначе — в конец.
  void insertCharacter(int index, const QString &filePath, const QJsonObject &ephemeralState = QJsonObject());
  // Загрузить персонажа в конец (обёртка для совместимости).
  void loadCharacter(const QString &filePath, const QJsonObject &ephemeralState = QJsonObject());
  // Сортировать карточки по значению инициативы (от большего к меньшему)
  void sortInitiative();

  // Геттеры для данных (для сохранения состояния)
  QString getTitle() const;
  QList<CharacterCard *> getCards() const;

private:
  QVBoxLayout *listLayout; // Вертикальный список карточек
  QLineEdit *titleEdit;    // Редактируемый заголовок колонки
  QScrollArea *scrollArea; // Скролл-область списка (для конвертации координат)

  // Инициализировать соединения карточки (sheetRequested, bindingChanged).
  void setupCard(CharacterCard *card);
};
#endif
