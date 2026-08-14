#include "NotesModel.h"
#include "Storage.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>
#include <utility>

NotesModel::NotesModel(QObject *parent) : QAbstractListModel(parent) { refresh(); }

int NotesModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_entries.size();
}

QVariant NotesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size()) return {};
    const Entry &entry = m_entries.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case TitleRole: return entry.title;
    case RelativePathRole: return entry.relativePath;
    case IsFolderRole: return entry.isFolder;
    case DepthRole: return entry.depth;
    default: return {};
    }
}

QHash<int, QByteArray> NotesModel::roleNames() const {
    return {{TitleRole, "title"}, {RelativePathRole, "relativePath"},
            {IsFolderRole, "isFolder"}, {DepthRole, "depth"}};
}

void NotesModel::refresh() {
    QVector<Entry> next;
    scanDirectory(Storage::notesDir(), QString(), 0, next);
    beginResetModel();
    m_entries = std::move(next);
    endResetModel();
}

QString NotesModel::loadText(const QString &relativePath) const {
    if (relativePath.isEmpty()) return {};
    QFile file(absolutePath(relativePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return {};
    return QString::fromUtf8(file.readAll());
}

bool NotesModel::saveText(const QString &relativePath, const QString &text) {
    if (relativePath.isEmpty()) return false;
    QSaveFile file(absolutePath(relativePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit operationFailed(tr("Не удалось сохранить заметку"));
        return false;
    }
    file.write(text.toUtf8());
    if (!file.commit()) {
        emit operationFailed(tr("Не удалось атомарно записать заметку"));
        return false;
    }
    return true;
}

bool NotesModel::createNote(const QString &parentPath, const QString &name) {
    const QString safe = sanitizeName(name);
    if (safe.isEmpty()) {
        emit operationFailed(tr("Недопустимое имя заметки"));
        return false;
    }
    QString parent = absolutePath(parentPath);
    if (!QDir(parent).exists()) parent = Storage::notesDir();
    const QString path = QDir(parent).filePath(safe + ".md");
    if (QFileInfo::exists(path)) {
        emit operationFailed(tr("Заметка с таким именем уже существует"));
        return false;
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit operationFailed(tr("Не удалось создать заметку"));
        return false;
    }
    file.close();
    refresh();
    return true;
}

bool NotesModel::createFolder(const QString &parentPath, const QString &name) {
    const QString safe = sanitizeName(name);
    if (safe.isEmpty()) {
        emit operationFailed(tr("Недопустимое имя папки"));
        return false;
    }
    QString parent = absolutePath(parentPath);
    if (!QDir(parent).exists()) parent = Storage::notesDir();
    if (!QDir(parent).mkdir(safe)) {
        emit operationFailed(tr("Не удалось создать папку"));
        return false;
    }
    refresh();
    return true;
}

bool NotesModel::renameAt(int row, const QString &newName) {
    if (row < 0 || row >= m_entries.size()) return false;
    const QString safe = sanitizeName(newName);
    if (safe.isEmpty()) {
        emit operationFailed(tr("Недопустимое имя"));
        return false;
    }
    const Entry entry = m_entries.at(row);
    const QString oldPath = absolutePath(entry.relativePath);
    QFileInfo info(oldPath);
    const QString fileName = safe + (entry.isFolder ? QString() : QStringLiteral(".md"));
    const QString newPath = info.dir().filePath(fileName);
    if (QFileInfo::exists(newPath)) {
        emit operationFailed(tr("Объект с таким именем уже существует"));
        return false;
    }
    if (!QDir().rename(oldPath, newPath)) {
        emit operationFailed(tr("Не удалось переименовать"));
        return false;
    }
    refresh();
    return true;
}

bool NotesModel::moveAt(int row, const QString &targetFolder) {
    if (row < 0 || row >= m_entries.size()) return false;

    const Entry entry = m_entries.at(row);
    const QString sourcePath = QDir::fromNativeSeparators(
        QDir::cleanPath(absolutePath(entry.relativePath)));
    const QString targetDir = QDir::fromNativeSeparators(
        QDir::cleanPath(absolutePath(targetFolder)));
    if (!QFileInfo(targetDir).isDir()) {
        emit operationFailed(tr("Папка назначения не существует"));
        return false;
    }

    if (entry.isFolder) {
        const QString prefix = sourcePath + '/';
        if (targetDir == sourcePath || targetDir.startsWith(prefix)) {
            emit operationFailed(tr("Нельзя переместить папку внутрь самой себя"));
            return false;
        }
    }

    const QString destination = QDir::fromNativeSeparators(
        QDir::cleanPath(QDir(targetDir).filePath(QFileInfo(sourcePath).fileName())));
    if (destination == sourcePath)
        return true;
    if (QFileInfo::exists(destination)) {
        emit operationFailed(tr("В папке назначения уже есть объект с таким именем"));
        return false;
    }
    if (!QDir().rename(sourcePath, destination)) {
        emit operationFailed(tr("Не удалось переместить %1").arg(entry.title));
        return false;
    }

    refresh();
    return true;
}

bool NotesModel::removeAt(int row) {
    if (row < 0 || row >= m_entries.size()) return false;
    const Entry entry = m_entries.at(row);
    const QString path = absolutePath(entry.relativePath);
    const bool ok = entry.isFolder ? QDir(path).removeRecursively() : QFile::remove(path);
    if (!ok) {
        emit operationFailed(tr("Не удалось удалить %1").arg(entry.title));
        return false;
    }
    refresh();
    return true;
}

QString NotesModel::pathByTitle(const QString &title) const {
    const QString wanted = title.trimmed();
    if (wanted.isEmpty()) return {};
    for (const Entry &entry : m_entries) {
        if (!entry.isFolder && entry.title.compare(wanted, Qt::CaseInsensitive) == 0)
            return entry.relativePath;
    }
    return {};
}

QString NotesModel::absolutePath(const QString &relativePath) const {
    if (relativePath.isEmpty()) return Storage::notesDir();
    const QString clean = QDir::cleanPath(relativePath);
    if (QDir::isAbsolutePath(clean) || clean == ".." || clean.startsWith("../"))
        return Storage::notesDir();
    return QDir(Storage::notesDir()).absoluteFilePath(clean);
}

QString NotesModel::sanitizeName(const QString &name) const {
    QString out = name.trimmed();
    out.replace(QRegularExpression(QStringLiteral(R"([<>:"/\\|?*\x00-\x1F])")), QStringLiteral("_"));
    while (out.endsWith('.') || out.endsWith(' '))
        out.chop(1);
    if (out == "." || out == "..")
        return {};

    static const QRegularExpression reserved(
        QStringLiteral(R"(^(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])$)"),
        QRegularExpression::CaseInsensitiveOption);
    if (reserved.match(out).hasMatch())
        out.prepend('_');
    return out.trimmed();
}

void NotesModel::scanDirectory(const QString &absoluteDir, const QString &relativeDir,
                               int depth, QVector<Entry> &target) const {
    QDir dir(absoluteDir);
    for (const QFileInfo &folder : dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name)) {
        const QString rel = relativeDir.isEmpty() ? folder.fileName() : relativeDir + "/" + folder.fileName();
        target.push_back({folder.fileName(), rel, true, depth});
        scanDirectory(folder.absoluteFilePath(), rel, depth + 1, target);
    }
    for (const QFileInfo &note : dir.entryInfoList({"*.md"}, QDir::Files | QDir::Readable, QDir::Name)) {
        const QString rel = relativeDir.isEmpty() ? note.fileName() : relativeDir + "/" + note.fileName();
        target.push_back({note.completeBaseName(), rel, false, depth});
    }
}
