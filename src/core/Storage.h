#ifndef STORAGE_H
#define STORAGE_H

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QStandardPaths>

namespace Storage {

inline QString appDataDir() {
#ifdef PORTABLE_BUILD
    return QFileInfo(QCoreApplication::applicationFilePath()).absolutePath();
#else
    static QString cached;
    if (!cached.isEmpty())
        return cached;

    const QString overridePath = qEnvironmentVariable("DND_TRACKER_DATA_DIR");
    if (!overridePath.trimmed().isEmpty()) {
        cached = QDir::cleanPath(overridePath);
        return cached;
    }

    cached = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (cached.isEmpty())
        cached = QDir::homePath() + "/.dnd_tracker";
    return cached;
#endif
}

inline QString charactersDir() { return appDataDir() + "/characters"; }
inline QString notesDir() { return appDataDir() + "/notes"; }
inline QString stateFilePath() { return appDataDir() + "/initiative_state.json"; }

inline QString findNoteRecursively(const QString &name) {
    QDirIterator it(notesDir(), {name + ".md"}, QDir::Files,
                    QDirIterator::Subdirectories);
    if (!it.hasNext())
        return {};

    it.next();
    QDir notes(notesDir());
    QString relative = notes.relativeFilePath(it.filePath());
    if (relative.endsWith(".md", Qt::CaseInsensitive))
        relative.chop(3);
    return relative;
}

inline bool ensureDirs() {
    QDir root(appDataDir());
    if (!root.exists() && !root.mkpath("."))
        return false;

    for (const QString &subdir : {charactersDir(), notesDir()}) {
        QDir dir(subdir);
        if (!dir.exists() && !dir.mkpath("."))
            return false;
    }
    return true;
}

} // namespace Storage

#endif // STORAGE_H
