#include "ClassicSheetStore.h"
#include "CharacterDocument.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QStringList>

namespace {
QString namedValue(const QJsonObject &object) {
    static const QStringList keys = {
        QStringLiteral("spellName"),
        QStringLiteral("name"),
        QStringLiteral("label"),
        QStringLiteral("title"),
        QStringLiteral("text"),
        QStringLiteral("value")
    };

    for (const QString &key : keys) {
        const QJsonValue value = object.value(key);
        if (value.isString() && !value.toString().trimmed().isEmpty())
            return value.toString();
    }
    return {};
}

QString nodeText(const QJsonValue &value) {
    if (value.isString())
        return value.toString();

    if (value.isArray()) {
        QString text;
        for (const QJsonValue &child : value.toArray())
            text += nodeText(child);
        return text;
    }

    if (!value.isObject())
        return {};

    const QJsonObject node = value.toObject();
    if (node.value(QStringLiteral("type")).toString() == QStringLiteral("text"))
        return node.value(QStringLiteral("text")).toString();

    QString text;
    for (const QJsonValue &child : node.value(QStringLiteral("content")).toArray())
        text += nodeText(child);
    if (!text.trimmed().isEmpty())
        return text;

    // LSS may keep interactive/rich nodes as attrs instead of ordinary
    // TipTap text children. Prefer human-readable spell-ish fields and never
    // expose opaque ids as visible sheet text.
    const QString fromAttrs = namedValue(node.value(QStringLiteral("attrs")).toObject());
    if (!fromAttrs.isEmpty())
        return fromAttrs;

    return namedValue(node);
}

QString richTextToPlain(QJsonValue value) {
    QJsonValue current = value;

    // LSS text blocks are commonly wrapped as value -> data -> doc, but some
    // exports use a plain string or omit one of those wrappers.
    for (;;) {
        if (!current.isObject())
            break;
        const QJsonObject object = current.toObject();
        if (object.contains(QStringLiteral("value"))) {
            current = object.value(QStringLiteral("value"));
            continue;
        }
        if (object.contains(QStringLiteral("data"))) {
            current = object.value(QStringLiteral("data"));
            continue;
        }
        break;
    }

    if (current.isString())
        return current.toString();

    QStringList lines;
    if (current.isArray()) {
        for (const QJsonValue &node : current.toArray()) {
            const QString line = nodeText(node).trimmed();
            if (!line.isEmpty())
                lines.append(line);
        }
        return lines.join('\n');
    }

    if (!current.isObject())
        return {};

    const QJsonObject document = current.toObject();
    const QJsonArray content = document.value(QStringLiteral("content")).toArray();
    if (content.isEmpty())
        return nodeText(document).trimmed();

    for (const QJsonValue &node : content) {
        const QString line = nodeText(node).trimmed();
        if (!line.isEmpty())
            lines.append(line);
    }
    return lines.join('\n');
}

QString spellKey(int level) {
    return QStringLiteral("spells-level-%1").arg(level);
}

QJsonObject collectSpellBackup(const QJsonObject &data) {
    const QJsonObject text = data.value(QStringLiteral("text")).toObject();
    QJsonObject backup;
    for (int level = 0; level <= 9; ++level) {
        const QString key = spellKey(level);
        if (text.contains(key))
            backup.insert(key, text.value(key));
    }
    return backup;
}
}

QJsonObject ClassicSheetStore::spellBackup(const QString &filePath) const {
    const auto cached = m_spellBackups.constFind(filePath);
    if (cached != m_spellBackups.constEnd())
        return cached.value();

    CharacterDocument document;
    if (!document.load(filePath))
        return {};

    const QJsonObject backup = collectSpellBackup(document.getData());
    m_spellBackups.insert(filePath, backup);
    return backup;
}

QVariantMap ClassicSheetStore::load(const QString &filePath) const {
    CharacterDocument document;
    if (!document.load(filePath))
        return {};

    const QJsonObject data = document.getData();
    m_spellBackups.insert(filePath, collectSpellBackup(data));
    m_editedSpellLevels.remove(filePath);
    return data.value(QStringLiteral("dndTrackerClassic")).toObject().toVariantMap();
}

QString ClassicSheetStore::spellText(const QString &filePath, int level) const {
    if (level < 0 || level > 9)
        return {};
    return richTextToPlain(spellBackup(filePath).value(spellKey(level)));
}

QString ClassicSheetStore::spellLine(const QString &filePath, int level, int index) const {
    if (index < 0)
        return {};
    const QStringList lines = spellText(filePath, level).split('\n', Qt::KeepEmptyParts);
    return index < lines.size() ? lines.at(index) : QString{};
}

void ClassicSheetStore::markSpellEdited(const QString &filePath, int level) {
    if (level < 0 || level > 9)
        return;
    m_editedSpellLevels[filePath].insert(level);
}

bool ClassicSheetStore::isSpellLevelEdited(const QString &filePath, int level) const {
    return m_editedSpellLevels.value(filePath).contains(level);
}

bool ClassicSheetStore::save(const QString &filePath, const QVariantMap &values) const {
    CharacterDocument document;
    if (!document.load(filePath))
        return false;

    QJsonObject data = document.getData();
    QJsonObject extras = data.value(QStringLiteral("dndTrackerClassic")).toObject();
    const QJsonObject incoming = QJsonObject::fromVariantMap(values);
    for (auto it = incoming.constBegin(); it != incoming.constEnd(); ++it)
        extras.insert(it.key(), it.value());
    data.insert(QStringLiteral("dndTrackerClassic"), extras);

    // App.saveCharacterBasics writes the plain-text spell representation for
    // every level. Restore the untouched original LSS nodes so merely toggling
    // prepared spells or editing biography fields cannot destroy richer spell
    // blocks. A level is flattened only after the user actually edits a spell
    // line in classic view.
    const QJsonObject backup = spellBackup(filePath);
    const QSet<int> edited = m_editedSpellLevels.value(filePath);
    QJsonObject text = data.value(QStringLiteral("text")).toObject();
    for (int level = 0; level <= 9; ++level) {
        const QString key = spellKey(level);
        if (!edited.contains(level) && backup.contains(key))
            text.insert(key, backup.value(key));
    }
    data.insert(QStringLiteral("text"), text);

    document.updateFullData(data);
    const bool ok = document.save();
    if (ok) {
        m_spellBackups.insert(filePath, collectSpellBackup(data));
        m_editedSpellLevels.remove(filePath);
    }
    return ok;
}