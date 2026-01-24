#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setStyleSheet("QWidget { color: black; }"); 
    MainWindow w;
    w.show();
    return a.exec();
}