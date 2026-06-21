#include "MainWindow.h"
#include "Storage.h"
#include <QApplication>
#include <QMessageBox>

// Точка входа в приложение
int main(int argc, char *argv[]) {
  // Имя приложения/организации — обязательно до создания виджетов:
  // от него зависит QStandardPaths::AppDataLocation (фикс #6 аудита).
  QApplication::setApplicationName("dnd_tracker");
  QApplication::setOrganizationName("dnd_tracker");

  // Инициализация графического приложения Qt
  QApplication a(argc, argv);

  // Гарантируем, что папки хранения существуют.
  if (!Storage::ensureDirs()) {
    QMessageBox::critical(
        nullptr, "Ошибка запуска",
        "Не удалось создать папку данных приложения:\n" + Storage::appDataDir() +
            "\n\nПроверьте права доступа.");
  }

  // Нативный Look & Feel: никаких внешних .qss. Приложение использует системную
  // палитру Qt (QPalette) и автоматически подстраивается под светлую/тёмную
  // тему ОС. Минимальные палитровые стили (border-radius и т.п.) применяются
  // локально на отдельных виджетах.

  // Создание и отображение главного окна
  MainWindow w;
  w.show();

  // Запуск цикла обработки событий
  return a.exec();
}
