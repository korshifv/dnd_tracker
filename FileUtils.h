#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QDir>
#include <QFileInfo>
#include <QString>

namespace FileUtils {
inline QString copyToData(const QString &sourcePath) {
  QDir dir("data");
  if (!dir.exists())
    dir.mkpath(".");

  QFileInfo fi(sourcePath);
  QString newPath = dir.filePath(fi.fileName());

  if (QFileInfo(sourcePath).absoluteFilePath() !=
      QFileInfo(newPath).absoluteFilePath()) {
    if (QFile::exists(newPath))
      QFile::remove(newPath);
    QFile::copy(sourcePath, newPath);
  }
  return newPath;
}
} // namespace FileUtils

#endif // FILEUTILS_H
