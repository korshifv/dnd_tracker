#ifndef FILEUTILS_H
#define FILEUTILS_H

#include "Storage.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>

// Утилиты для импорта файлов персонажей в хранилище приложения.
namespace FileUtils {

// Копирует файл персонажа в хранилище (Storage::charactersDir()).
// При совпадении имени файла генерирует уникальное ("Имя_1.json", "Имя_2.json"),
// а НЕ перезаписывает существующий — раньше это молча уничтожало персонажа (#7).
// Возвращает путь к локальной копии либо пустую строку при ошибке копирования.
inline QString copyToData(const QString &sourcePath) {
  QDir dir(Storage::charactersDir());
  if (!dir.exists())
    dir.mkpath(".");

  QFileInfo fi(sourcePath);
  QString newPath = dir.filePath(fi.fileName());

  // Если импортируют файл, который уже лежит в хранилище — ничего не делаем.
  if (QFileInfo(sourcePath).absoluteFilePath() ==
      QFileInfo(newPath).absoluteFilePath()) {
    return newPath;
  }

  // Разрешение коллизии имён: ищем первый свободный вариант.
  if (QFile::exists(newPath)) {
    const QString base = fi.completeBaseName(); // имя без расширения
    const QString ext = fi.suffix();            // расширение без точки
    int suffix = 1;
    do {
      newPath = dir.filePath(QString("%1_%2.%3").arg(base).arg(suffix).arg(ext));
      ++suffix;
    } while (QFile::exists(newPath));
  }

  // Проверяем результат копирования — раньше отказ игнорировался (#7),
  // и карточка открывала несуществующий файл без всякой ошибки.
  if (!QFile::copy(sourcePath, newPath)) {
    return {};
  }
  return newPath;
}

} // namespace FileUtils

#endif // FILEUTILS_H
