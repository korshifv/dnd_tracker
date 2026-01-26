#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QWidget>

class QHBoxLayout;

// Класс главного окна приложения
// Отвечает за хранение и отображение колонок с трекерами инициативы
class MainWindow : public QWidget {
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

protected:
  // Перехват события закрытия окна для сохранения состояния
  void closeEvent(QCloseEvent *event) override;

private:
  // Сохранение текущего состояния (колонки, открытые персонажи) в JSON
  void saveState();
  // Восстановление состояния из JSON при запуске
  void loadState();
private slots:
  // Слот добавления новой колонки (Группы)
  void addColumn();
  // Очистка всех данных и состояния приложения с подтверждением
  void clearAllData();

private:
  QHBoxLayout *columnsLayout; // Лейаут для горизонтального размещения колонок
};
#endif