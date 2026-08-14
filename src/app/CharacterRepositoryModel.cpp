#include "CharacterRepositoryModel.h"
#include "CharacterDocument.h"
#include "JsonUtils.h"
#include "Storage.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <utility>

CharacterRepositoryModel::CharacterRepositoryModel(QObject *parent)
    : QAbstractListModel(parent) {
    refresh();
}

int CharacterRepositoryModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_entries.size();
}

QVariant CharacterRepositoryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
        return {};
    const Entry &entry = m_entries.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case NameRole: return entry.name;
    case FilePathRole: return entry.filePath;
    case ClassRole: return entry.charClass;
    case LevelRole: return entry.level;
    case HpRole: return entry.hp;
    case HpMaxRole: return entry.hpMax;
    case ArmorClassRole: return entry.armorClass;
    case InitiativeRole: return entry.initiative;
    default: return {};
    }
}

QHash<int, QByteArray> CharacterRepositoryModel::roleNames() const {
    return {
        {FilePathRole, "filePath"}, {NameRole, "name"},
        {ClassRole, "charClass"}, {LevelRole, "level"},
        {HpRole, "hp"}, {HpMaxRole, "hpMax"},
        {ArmorClassRole, "armorClass"}, {InitiativeRole, "initiative"}
    };
}

void CharacterRepositoryModel::refresh() {
    QVector<Entry> next;
    QDir dir(Storage::charactersDir());
    const QStringList files = dir.entryList({"*.json"}, QDir::Files | QDir::Readable, QDir::Name);
    next.reserve(files.size());
    for (const QString &fileName : files)
        next.push_back(readEntry(dir.absoluteFilePath(fileName)));
    beginResetModel();
    m_entries = std::move(next);
    endResetModel();
}

QString CharacterRepositoryModel::pathAt(int row) const {
    return row >= 0 && row < m_entries.size() ? m_entries.at(row).filePath : QString();
}

bool CharacterRepositoryModel::removeAt(int row) {
    if (row < 0 || row >= m_entries.size()) return false;
    const QString path = m_entries.at(row).filePath;
    if (!QFile::remove(path)) {
        emit operationFailed(tr("Не удалось удалить %1").arg(QFileInfo(path).fileName()));
        return false;
    }
    beginRemoveRows(QModelIndex(), row, row);
    m_entries.removeAt(row);
    endRemoveRows();
    return true;
}

CharacterRepositoryModel::Entry CharacterRepositoryModel::readEntry(const QString &filePath) const {
    Entry entry;
    entry.filePath = filePath;
    entry.name = QFileInfo(filePath).completeBaseName();
    CharacterDocument document;
    if (!document.load(filePath)) return entry;
    const QJsonObject data = document.getData();
    if (!document.getName().isEmpty()) entry.name = document.getName();
    entry.charClass = JsonUtils::safeGetString(data, {"info", "charClass"});
    entry.level = JsonUtils::safeGetInt(data, {"info", "level"});
    entry.hp = document.getHp();
    entry.hpMax = document.getHpMax();
    entry.armorClass = document.getArmorClass();
    entry.initiative = document.getInitiative();
    return entry;
}
