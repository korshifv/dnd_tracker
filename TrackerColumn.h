#ifndef TRACKERCOLUMN_H
#define TRACKERCOLUMN_H
#include <QFrame>

class QVBoxLayout;
class QLineEdit;

// Виджет колонки трекера
// Отвечает за отобажение группы персонажей (например, Игроки или Враги) и
// управление ими
class TrackerColumn : public QFrame {
  Q_OBJECT
public:
  explicit TrackerColumn(const QString &title, QWidget *parent = nullptr);
  ~TrackerColumn();

protected:
  // События Drag & Drop для перетаскивания карточек между колонками
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dragMoveEvent(QDragMoveEvent *event) override;
  void dropEvent(QDropEvent *event) override;

public slots:
  // Добавить новую пустую карточку персонажа в колонку
  void addCharacter();
  // Загрузить персонажа из файла (используется при восстановлении сессии)
  void loadCharacter(const QString &filePath);
  // Сортировать карточки по значению инициативы (от большего к меньшему)
  void sortInitiative();

private:
  QVBoxLayout *listLayout; // Вертикальный список карточек
  QLineEdit *titleEdit;    // Редактируемый заголовок колонки
};
#endif