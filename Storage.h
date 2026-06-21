#ifndef STORAGE_H
#define STORAGE_H

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QStandardPaths>

// Хелпер путей хранения приложения.
// Раньше все файлы (состояние, чарники) лежали относительно CWD — это дыра в
// безопасности (#6 аудита): запуск из чужой директории подхватывал подменённые
// файлы. Теперь всё хранится в стандартном AppDataLocation.
namespace Storage {

// Корневая папка данных приложения (QStandardPaths::AppDataLocation).
// На Linux: ~/.local/share/dnd_tracker (или при PORTABLE_BUILD — рядом с exe).
inline QString appDataDir() {
#ifdef PORTABLE_BUILD
  // Переносимая сборка: всё рядом с исполняемым файлом.
  return QFileInfo(QCoreApplication::applicationFilePath()).absolutePath();
#else
  static QString cached;
  if (cached.isEmpty()) {
    cached = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    // Если путь недоступен (тестовые окружения и т.п.) — откат к домашней папке.
    if (cached.isEmpty())
      cached = QDir::homePath() + "/.dnd_tracker";
  }
  return cached;
#endif
}

// Папка с файлами персонажей.
inline QString charactersDir() { return appDataDir() + "/characters"; }

// Папка с заметками (Markdown, .md).
inline QString notesDir() { return appDataDir() + "/notes"; }

// Путь к файлу состояния трекера инициативы.
inline QString stateFilePath() { return appDataDir() + "/initiative_state.json"; }

// Рекурсивный поиск заметки по имени (без расширения) внутри notesDir.
// Возвращает относительный путь от notesDir (например "folder/Note")
// или пустую строку, если не найдено.
inline QString findNoteRecursively(const QString &name) {
  QDirIterator it(notesDir(), {name + ".md"}, QDir::Files, QDirIterator::Subdirectories);
  if (it.hasNext()) {
    it.next();
    QString absPath = it.filePath();
    QDir notesD(notesDir());
    QString relPath = notesD.relativeFilePath(absPath);
    if (relPath.endsWith(".md")) {
      relPath.chop(3);
    }
    return relPath;
  }
  return QString();
}

  // Создаёт нужные папки при первом запуске. Возвращает true при успехе.
  inline bool ensureDirs() {
    QDir d(appDataDir());
    if (!d.exists() && !d.mkpath("."))
      return false;
    for (const auto &sub : {charactersDir(), notesDir()}) {
      QDir c(sub);
      if (!c.exists() && !c.mkpath("."))
        return false;
    }
    return true;
  }

} // namespace Storage

#endif // STORAGE_H
