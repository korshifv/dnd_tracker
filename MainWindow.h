#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QWidget>

class QHBoxLayout;

class MainWindow : public QWidget {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void addColumn();
private:
    QHBoxLayout *columnsLayout;
};
#endif