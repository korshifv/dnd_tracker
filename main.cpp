#include <QApplication>
#include "MainWindow.h"

// Точка входа в приложение
int main(int argc, char *argv[]) {
    // Инициализация графического приложения Qt
    QApplication a(argc, argv);
    
    // Установка глобальных стилей: черный цвет текста для всех виджетов по умолчанию
    a.setStyleSheet("QWidget { color: black; }"); 
    
    // Создание и отображение главного окна
    MainWindow w;
    w.show();
    
    // Запуск цикла обработки событий
    return a.exec();
}