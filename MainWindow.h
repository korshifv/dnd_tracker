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
private slots:
    // Слот добавления новой колонки (Группы)
    void addColumn();
private:
    QHBoxLayout *columnsLayout; // Лейаут для горизонтального размещения колонок
};
#endif