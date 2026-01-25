#include "MainWindow.h"
#include <QApplication>

// Точка входа в приложение
int main(int argc, char *argv[]) {
  // Инициализация графического приложения Qt
  QApplication a(argc, argv);

  // Создание и отображение главного окна
  MainWindow w;
  w.show();

  // Запуск цикла обработки событий
  return a.exec();
}