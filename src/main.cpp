#include "MainWindow.h"
#include "Storage.h"
#include "MobileTheme.h"
#include <QApplication>
#include <QMessageBox>

// Точка входа в приложение
int main(int argc, char *argv[]) {
  // Имя приложения/организации — обязательно до создания виджетов:
  QApplication::setApplicationName("dnd_tracker");
  QApplication::setOrganizationName("dnd_tracker");

  // Инициализация графического приложения Qt
  QApplication a(argc, argv);

  // Применяем современную тёмную тему и стили для мобилок и ПК
  a.setStyleSheet(MobileTheme::getStylesheet());

  // Гарантируем, что папки хранения существуют.
  if (!Storage::ensureDirs()) {
    QMessageBox::critical(
        nullptr, "Ошибка запуска",
        "Не удалось создать папку данных приложения:\n" + Storage::appDataDir() +
            "\n\nПроверьте права доступа.");
  }

  // Создание и отображение главного окна
  MainWindow w;
  w.show();

  // Запуск цикла обработки событий
  return a.exec();
}
