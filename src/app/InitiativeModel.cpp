#include "InitiativeModel.h"

#include "CharacterDocument.h"
#include "Storage.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QSaveFile>
#include <QUuid>
#include <algorithm>

namespace {
QString randomPastel() {
    auto *rng = QRandomGenerator::global();
    return QString("#%1%2%3")
        .arg(rng->bounded(160, 256), 2, 16, QLatin1Char('0'))
        .arg(rng->bounded(160, 256), 2, 16, QLatin1Char('0'))
        .arg(rng->bounded(160, 256), 2, 16, QLatin1Char('0'))
        .toUpper();
}
}

InitiativeModel::InitiativeModel(QObject *parent)
    : QAbstractListModel(parent) {
    m_saveTimer.setSingleShot(true);
    m_saveTimer.setInterval(250);
    connect(&m_saveTimer, &QTimer::timeout, this, &InitiativeModel::saveState);
    loadState();
}

int InitiativeModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_items.size();
}

QVariant InitiativeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return {};

    const Combatant &item = m_items.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case NameRole:
        return item.name;
    case IdRole:
        return item.id;
    case HpRole:
        return item.hp;
    case ArmorClassRole:
        return item.armorClass;
    case InitiativeRole:
        return item.initiative;
    case StatusRole:
        return item.status;
    case FilePathRole:
        return item.filePath;
    case GroupRole:
        return item.group;
    case AvatarColorRole:
        return item.avatarColor;
    case ActiveRole:
        return !m_currentTurnId.isEmpty() && item.id == m_currentTurnId;
    default:
        return {};
    }
}

QHash<int, QByteArray> InitiativeModel::roleNames() const {
    return {
        {IdRole, "combatantId"},
        {NameRole, "name"},
        {HpRole, "hp"},
        {ArmorClassRole, "armorClass"},
        {InitiativeRole, "initiative"},
        {StatusRole, "status"},
        {FilePathRole, "filePath"},
        {GroupRole, "groupName"},
        {AvatarColorRole, "avatarColor"},
        {ActiveRole, "activeTurn"},
    };
}

QString InitiativeModel::currentTurnName() const {
    const int index = indexOfId(m_currentTurnId);
    return index >= 0 ? m_items.at(index).name : QString();
}

void InitiativeModel::addBlank(const QString &group) {
    Combatant item;
    item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    item.name = tr("Безымянный");
    item.group = group.trimmed().isEmpty() ? tr("Основная группа") : group.trimmed();
    item.avatarColor = randomPastel();

    const int row = m_items.size();
    beginInsertRows(QModelIndex(), row, row);
    m_items.push_back(item);
    endInsertRows();

    emit countChanged();
    scheduleSave();
}

bool InitiativeModel::addCharacter(const QString &filePath, const QString &group) {
    CharacterDocument document;
    if (!document.load(filePath)) {
        emit operationFailed(tr("Не удалось открыть персонажа: %1").arg(filePath));
        return false;
    }

    Combatant item;
    item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    item.name = document.getName();
    if (item.name.isEmpty())
        item.name = QFileInfo(filePath).completeBaseName();
    item.hp = document.getHp();
    item.armorClass = document.getArmorClass();
    item.initiative = document.getInitiative();
    item.filePath = filePath;
    item.group = group.trimmed().isEmpty() ? tr("Основная группа") : group.trimmed();
    item.avatarColor = randomPastel();

    const int row = m_items.size();
    beginInsertRows(QModelIndex(), row, row);
    m_items.push_back(item);
    endInsertRows();

    emit countChanged();
    scheduleSave();
    return true;
}

void InitiativeModel::removeAt(int row) {
    if (row < 0 || row >= m_items.size())
        return;

    const bool removedCurrent = m_items.at(row).id == m_currentTurnId;
    beginRemoveRows(QModelIndex(), row, row);
    m_items.removeAt(row);
    endRemoveRows();

    if (removedCurrent) {
        m_currentTurnId.clear();
        emit currentTurnChanged();
    }
    emit countChanged();
    scheduleSave();
}

void InitiativeModel::setName(int row, const QString &name) {
    if (row < 0 || row >= m_items.size())
        return;
    m_items[row].name = name;
    emitRowChanged(row, {NameRole});
    if (m_items[row].id == m_currentTurnId)
        emit currentTurnChanged();
    scheduleSave();
}

void InitiativeModel::setHp(int row, int value) {
    if (row < 0 || row >= m_items.size())
        return;
    if (m_items[row].hp == value)
        return;
    m_items[row].hp = value;
    syncHpToCharacter(m_items[row]);
    emitRowChanged(row, {HpRole});
    scheduleSave();
}

void InitiativeModel::setArmorClass(int row, int value) {
    if (row < 0 || row >= m_items.size())
        return;
    if (m_items[row].armorClass == value)
        return;
    m_items[row].armorClass = value;
    syncArmorClassToCharacter(m_items[row]);
    emitRowChanged(row, {ArmorClassRole});
    scheduleSave();
}

void InitiativeModel::setInitiative(int row, int value) {
    if (row < 0 || row >= m_items.size())
        return;
    m_items[row].initiative = value;
    emitRowChanged(row, {InitiativeRole});
    scheduleSave();
}

void InitiativeModel::setStatus(int row, const QString &status) {
    if (row < 0 || row >= m_items.size())
        return;
    m_items[row].status = status;
    emitRowChanged(row, {StatusRole});
    scheduleSave();
}

void InitiativeModel::setGroup(int row, const QString &group) {
    if (row < 0 || row >= m_items.size())
        return;
    m_items[row].group = group.trimmed();
    emitRowChanged(row, {GroupRole});
    scheduleSave();
}

void InitiativeModel::setAvatarColor(int row, const QString &color) {
    if (row < 0 || row >= m_items.size())
        return;
    m_items[row].avatarColor = color;
    emitRowChanged(row, {AvatarColorRole});
    scheduleSave();
}

void InitiativeModel::applyDamage(int row, int amount) {
    if (row < 0 || row >= m_items.size() || amount < 0)
        return;
    m_items[row].hp -= amount;
    syncHpToCharacter(m_items[row]);
    emitRowChanged(row, {HpRole});
    scheduleSave();
}

void InitiativeModel::applyHeal(int row, int amount) {
    if (row < 0 || row >= m_items.size() || amount < 0)
        return;
    m_items[row].hp += amount;
    syncHpToCharacter(m_items[row]);
    emitRowChanged(row, {HpRole});
    scheduleSave();
}

void InitiativeModel::sortByInitiative() {
    if (m_items.size() < 2)
        return;

    beginResetModel();
    std::stable_sort(m_items.begin(), m_items.end(),
                     [](const Combatant &a, const Combatant &b) {
                         return a.initiative > b.initiative;
                     });
    endResetModel();
    scheduleSave();
}

void InitiativeModel::nextTurn() {
    if (m_items.isEmpty())
        return;

    QVector<int> order;
    order.reserve(m_items.size());
    for (int i = 0; i < m_items.size(); ++i)
        order.push_back(i);

    std::stable_sort(order.begin(), order.end(), [this](int a, int b) {
        return m_items[a].initiative > m_items[b].initiative;
    });

    if (m_currentTurnId.isEmpty()) {
        m_currentTurnId = m_items.at(order.first()).id;
    } else {
        int currentPos = -1;
        for (int i = 0; i < order.size(); ++i) {
            if (m_items.at(order.at(i)).id == m_currentTurnId) {
                currentPos = i;
                break;
            }
        }

        if (currentPos < 0 || currentPos + 1 >= order.size()) {
            m_currentTurnId = m_items.at(order.first()).id;
            ++m_round;
            emit roundChanged();
        } else {
            m_currentTurnId = m_items.at(order.at(currentPos + 1)).id;
        }
    }

    emit dataChanged(index(0, 0), index(m_items.size() - 1, 0), {ActiveRole});
    emit currentTurnChanged();
    scheduleSave();
}

void InitiativeModel::resetCombat() {
    const bool roundWasDifferent = m_round != 1;
    m_round = 1;
    m_currentTurnId.clear();

    if (roundWasDifferent)
        emit roundChanged();
    if (!m_items.isEmpty())
        emit dataChanged(index(0, 0), index(m_items.size() - 1, 0), {ActiveRole});
    emit currentTurnChanged();
    scheduleSave();
}

void InitiativeModel::clearAll() {
    if (m_items.isEmpty()) {
        resetCombat();
        return;
    }

    beginResetModel();
    m_items.clear();
    endResetModel();
    m_round = 1;
    m_currentTurnId.clear();

    emit roundChanged();
    emit currentTurnChanged();
    emit countChanged();
    scheduleSave();
}

void InitiativeModel::flush() {
    m_saveTimer.stop();
    saveState();
}

void InitiativeModel::scheduleSave() {
    m_saveTimer.start();
}

void InitiativeModel::saveState() {
    QJsonObject root;
    root["version"] = 2;
    root["round"] = m_round;
    root["currentTurnId"] = m_currentTurnId;

    QJsonArray combatants;
    for (const Combatant &item : m_items) {
        QJsonObject obj;
        obj["id"] = item.id;
        obj["name"] = item.name;
        obj["hp"] = item.hp;
        obj["armorClass"] = item.armorClass;
        obj["initiative"] = item.initiative;
        obj["status"] = item.status;
        obj["characterRef"] = portableCharacterRef(item.filePath);
        obj["group"] = item.group;
        obj["avatarColor"] = item.avatarColor;
        combatants.append(obj);
    }
    root["combatants"] = combatants;

    QSaveFile file(Storage::stateFilePath());
    if (!file.open(QIODevice::WriteOnly)) {
        emit operationFailed(tr("Не удалось сохранить состояние инициативы"));
        return;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    if (!file.commit())
        emit operationFailed(tr("Не удалось атомарно записать состояние инициативы"));
}

void InitiativeModel::loadState() {
    QFile file(Storage::stateFilePath());
    if (!file.open(QIODevice::ReadOnly))
        return;

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        emit operationFailed(tr("Файл состояния инициативы повреждён; он не был загружен"));
        return;
    }

    const QJsonObject root = document.object();
    beginResetModel();
    m_items.clear();
    const bool loaded = root.value("version").toInt() >= 2
        ? loadV2(root)
        : loadLegacy(root);
    endResetModel();

    if (loaded && !m_items.isEmpty())
        emit countChanged();
}

bool InitiativeModel::loadV2(const QJsonObject &root) {
    m_round = std::max(1, root.value("round").toInt(1));
    m_currentTurnId = root.value("currentTurnId").toString();

    const QJsonArray combatants = root.value("combatants").toArray();
    for (const QJsonValue &value : combatants) {
        const QJsonObject obj = value.toObject();
        Combatant item;
        item.id = obj.value("id").toString();
        if (item.id.isEmpty())
            item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        item.name = obj.value("name").toString(tr("Безымянный"));
        item.hp = obj.value("hp").toInt(10);
        item.armorClass = obj.value("armorClass").toInt();
        item.initiative = obj.value("initiative").toInt();
        item.status = obj.value("status").toString();
        item.filePath = resolveCharacterRef(obj.value("characterRef").toString());
        item.group = obj.value("group").toString(tr("Основная группа"));
        item.avatarColor = obj.value("avatarColor").toString();
        if (item.avatarColor.isEmpty())
            item.avatarColor = randomPastel();
        m_items.push_back(item);
    }

    if (indexOfId(m_currentTurnId) < 0)
        m_currentTurnId.clear();
    return true;
}

bool InitiativeModel::loadLegacy(const QJsonObject &root) {
    m_round = std::max(1, root.value("roundCount").toInt(1));
    const int legacyTurnIndex = root.value("currentTurnIndex").toInt(-1);

    const QJsonArray columns = root.value("columns").toArray();
    for (const QJsonValue &columnValue : columns) {
        const QJsonObject column = columnValue.toObject();
        const QString group = column.value("title").toString(tr("Основная группа"));

        for (const QJsonValue &cardValue : column.value("cards").toArray()) {
            const QJsonObject card = cardValue.toObject();
            const QJsonObject state = card.value("state").toObject();

            Combatant item;
            item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
            item.filePath = resolveCharacterRef(card.value("path").toString());
            item.name = state.value("name").toString();
            item.hp = state.value("hp").toInt(10);
            item.initiative = state.value("initiative").toInt();
            item.status = state.value("status").toString();
            item.group = group;

            const QString rgb = state.value("avatarColor").toString();
            if (!rgb.isEmpty()) {
                const QStringList parts = rgb.split(',');
                if (parts.size() == 3) {
                    item.avatarColor = QString("#%1%2%3")
                        .arg(parts[0].toInt(), 2, 16, QLatin1Char('0'))
                        .arg(parts[1].toInt(), 2, 16, QLatin1Char('0'))
                        .arg(parts[2].toInt(), 2, 16, QLatin1Char('0'))
                        .toUpper();
                }
            }
            if (item.avatarColor.isEmpty())
                item.avatarColor = randomPastel();

            if (!item.filePath.isEmpty()) {
                CharacterDocument document;
                if (document.load(item.filePath)) {
                    if (item.name.isEmpty())
                        item.name = document.getName();
                    item.armorClass = document.getArmorClass();
                }
            }
            if (item.name.isEmpty())
                item.name = tr("Безымянный");

            m_items.push_back(item);
        }
    }

    if (legacyTurnIndex >= 0 && !m_items.isEmpty()) {
        QVector<int> order;
        for (int i = 0; i < m_items.size(); ++i)
            order.push_back(i);
        std::stable_sort(order.begin(), order.end(), [this](int a, int b) {
            return m_items[a].initiative > m_items[b].initiative;
        });
        if (legacyTurnIndex < order.size())
            m_currentTurnId = m_items.at(order.at(legacyTurnIndex)).id;
    }

    // Следующая запись автоматически мигрирует legacy state в v2.
    scheduleSave();
    return true;
}

QString InitiativeModel::portableCharacterRef(const QString &filePath) const {
    if (filePath.isEmpty())
        return {};

    const QFileInfo file(filePath);
    const QDir characters(Storage::charactersDir());
    const QString relative = characters.relativeFilePath(file.absoluteFilePath());
    if (!relative.startsWith("../") && relative != "..")
        return relative;

    return file.absoluteFilePath();
}

QString InitiativeModel::resolveCharacterRef(const QString &ref) const {
    if (ref.isEmpty())
        return {};

    QFileInfo direct(ref);
    if (direct.isAbsolute() && direct.exists())
        return direct.absoluteFilePath();

    const QString insideStore = QDir(Storage::charactersDir()).absoluteFilePath(ref);
    if (QFileInfo::exists(insideStore))
        return insideStore;

    // Legacy state from another OS often contains an absolute path. Fall back
    // to the basename inside the current platform's characters directory.
    QString portableName = ref;
    portableName.replace('\\', '/');
    portableName = portableName.section('/', -1);

    const QString byName =
        QDir(Storage::charactersDir()).absoluteFilePath(portableName);
    return QFileInfo::exists(byName) ? byName : QString();
}

void InitiativeModel::syncHpToCharacter(const Combatant &item) {
    if (item.filePath.isEmpty())
        return;

    CharacterDocument document;
    if (!document.load(item.filePath))
        return;

    document.setHp(item.hp);
    if (!document.save())
        emit operationFailed(tr("HP изменён в бою, но файл персонажа не удалось сохранить"));
}

void InitiativeModel::syncArmorClassToCharacter(const Combatant &item) {
    if (item.filePath.isEmpty())
        return;

    CharacterDocument document;
    if (!document.load(item.filePath))
        return;

    document.setArmorClass(item.armorClass);
    if (!document.save())
        emit operationFailed(tr("КД изменён в бою, но файл персонажа не удалось сохранить"));
}

int InitiativeModel::indexOfId(const QString &id) const {
    if (id.isEmpty())
        return -1;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).id == id)
            return i;
    }
    return -1;
}

void InitiativeModel::emitRowChanged(int row, const QList<int> &roles) {
    if (row < 0 || row >= m_items.size())
        return;
    emit dataChanged(index(row, 0), index(row, 0), roles);
}
