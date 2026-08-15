#include "AppController.h"
#include "CharacterDocument.h"
#include "CharacterRepositoryModel.h"
#include "InitiativeModel.h"
#include "JsonUtils.h"
#include "NotesModel.h"
#include "Storage.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

namespace {
QJsonObject wrapped(const QVariant &value) {
    return QJsonObject{{"value", QJsonValue::fromVariant(value)}};
}

QString tipTapNodeText(const QJsonValue &value) {
    if (!value.isObject())
        return {};
    const QJsonObject node = value.toObject();
    if (node.value("type").toString() == "text")
        return node.value("text").toString();

    QString text;
    const QJsonArray content = node.value("content").toArray();
    for (const QJsonValue &child : content)
        text += tipTapNodeText(child);
    return text;
}

QString tipTapToPlain(const QJsonValue &value) {
    QJsonValue current = value;
    if (current.isObject() && current.toObject().contains("value"))
        current = current.toObject().value("value");
    if (current.isObject() && current.toObject().contains("data"))
        current = current.toObject().value("data");

    const QJsonObject document = current.toObject();
    QStringList lines;
    for (const QJsonValue &node : document.value("content").toArray())
        lines.append(tipTapNodeText(node));
    return lines.join('\n');
}

QJsonObject plainToTipTapValue(const QString &text) {
    QJsonArray content;
    const QStringList lines = text.split('\n');
    for (const QString &line : lines) {
        QJsonObject paragraph{{"type", "paragraph"}};
        if (!line.isEmpty())
            paragraph["content"] = QJsonArray{QJsonObject{{"type", "text"}, {"text", line}}};
        else
            paragraph["content"] = QJsonArray{};
        content.append(paragraph);
    }

    const QJsonObject document{{"type", "doc"}, {"content", content}};
    return QJsonObject{{"value", QJsonObject{{"data", document}}}};
}

QVariantList jsonBoolArrayToVariant(const QJsonArray &array, int minimumSize = 0) {
    QVariantList out;
    for (const QJsonValue &value : array)
        out.append(value.toBool());
    while (out.size() < minimumSize)
        out.append(false);
    return out;
}

QJsonArray variantBoolArray(const QVariant &value, int minimumSize = 0) {
    QJsonArray out;
    const QVariantList list = value.toList();
    for (const QVariant &item : list)
        out.append(item.toBool());
    while (out.size() < minimumSize)
        out.append(false);
    return out;
}

QString wrappedString(const QJsonObject &obj, const QString &key) {
    const QJsonValue value = obj.value(key);
    if (value.isString())
        return value.toString();
    if (value.isObject())
        return value.toObject().value("value").toString();
    return {};
}
}

AppController::AppController(CharacterRepositoryModel *characters,
                             InitiativeModel *initiative,
                             NotesModel *notes,
                             QObject *parent)
    : QObject(parent), m_characters(characters), m_initiative(initiative), m_notes(notes) {
    connect(m_characters, &CharacterRepositoryModel::operationFailed,
            this, [this](const QString &m) { setError(m); });
    connect(m_initiative, &InitiativeModel::operationFailed,
            this, [this](const QString &m) { setError(m); });
    connect(m_notes, &NotesModel::operationFailed,
            this, [this](const QString &m) { setError(m); });
}

bool AppController::importCharacter(const QUrl &url) {
    clearError();
    const QString sourcePath = url.isLocalFile()
        ? url.toLocalFile()
        : url.toString(QUrl::FullyEncoded);
    QFile source(sourcePath);
    if (sourcePath.isEmpty() || !source.open(QIODevice::ReadOnly)) {
        setError(tr("Не удалось открыть выбранный файл"));
        return false;
    }
    const QByteArray bytes = source.readAll();
    QJsonParseError error;
    if (!QJsonDocument::fromJson(bytes, &error).isObject()) {
        setError(tr("Файл не является корректным JSON"));
        return false;
    }

    QString fileName = url.fileName();
    if (fileName.isEmpty())
        fileName = QFileInfo(sourcePath).fileName();
    if (fileName.isEmpty())
        fileName = "character.json";
    if (!fileName.endsWith(".json", Qt::CaseInsensitive))
        fileName += ".json";
    const QString destination = uniqueDestinationPath(fileName);
    QFile target(destination);
    if (!target.open(QIODevice::WriteOnly) || target.write(bytes) != bytes.size()) {
        QFile::remove(destination);
        setError(tr("Не удалось импортировать персонажа"));
        return false;
    }
    target.close();
    m_characters->refresh();
    return true;
}

QVariantMap AppController::characterDetails(const QString &path) const {
    CharacterDocument document;
    if (!document.load(path))
        return {};

    const QJsonObject data = document.getData();
    const QJsonObject stats = data.value("stats").toObject();
    QVariantMap out;
    out["filePath"] = path;
    out["name"] = document.getName();
    out["charClass"] = JsonUtils::safeGetString(data, {"info", "charClass"});
    out["subclass"] = JsonUtils::safeGetString(data, {"info", "charSubclass"});
    out["race"] = JsonUtils::safeGetString(data, {"info", "race"});
    out["background"] = JsonUtils::safeGetString(data, {"info", "background"});
    out["alignment"] = JsonUtils::safeGetString(data, {"info", "alignment"});
    out["playerName"] = JsonUtils::safeGetString(data, {"info", "playerName"});
    out["experience"] = JsonUtils::safeGetString(data, {"info", "experience"});
    out["level"] = JsonUtils::safeGetInt(data, {"info", "level"}, 1);
    out["hp"] = document.getHp();
    out["hpMax"] = document.getHpMax();
    out["hpTemp"] = document.getHpTemp();
    out["armorClass"] = document.getArmorClass();
    out["initiative"] = document.getInitiative();
    out["speed"] = JsonUtils::safeGetString(data, {"vitality", "speed"});
    out["hitDie"] = JsonUtils::safeGetString(data, {"vitality", "hit-die"});
    out["passivePerceptionOverride"] = JsonUtils::safeGetInt(
        data, {"vitality", "passive-perception"}, -1);
    out["proficiency"] = data.value("proficiency").toInt(2);
    out["inspiration"] = data.value("inspiration").toBool(false);

    for (const QString &key : {"str", "dex", "con", "int", "wis", "cha"})
        out[key] = stats.value(key).toObject().value("score").toInt(10);

    QVariantMap saves;
    const QJsonObject savesJson = data.value("saves").toObject();
    for (const QString &key : {"str", "dex", "con", "int", "wis", "cha"})
        saves[key] = savesJson.value(key).toObject().value("isProf").toBool(false);
    out["saves"] = saves;

    QVariantList skills;
    const QJsonObject skillsJson = data.value("skills").toObject();
    QStringList skillKeys = skillsJson.keys();
    skillKeys.sort(Qt::CaseInsensitive);
    for (const QString &key : skillKeys) {
        const QJsonObject skill = skillsJson.value(key).toObject();
        QVariantMap item;
        item["key"] = key;
        item["label"] = skill.value("label").toString(key);
        item["baseStat"] = skill.value("baseStat").toString();
        item["profLevel"] = skill.value("isProf").toInt();
        skills.append(item);
    }
    out["skills"] = skills;

    const QJsonObject deathSaves = data.value("deathSaves").toObject();
    out["deathSuccess"] = jsonBoolArrayToVariant(deathSaves.value("success").toArray(), 3);
    out["deathFail"] = jsonBoolArrayToVariant(deathSaves.value("fail").toArray(), 3);

    const QJsonObject text = data.value("text").toObject();
    out["personality"] = tipTapToPlain(text.value("personality"));
    out["ideals"] = tipTapToPlain(text.value("ideals"));
    out["bonds"] = tipTapToPlain(text.value("bonds"));
    out["flaws"] = tipTapToPlain(text.value("flaws"));
    out["features"] = tipTapToPlain(text.value("traits"));
    out["equipment"] = tipTapToPlain(text.value("equipment"));

    QVariantList weapons;
    for (const QJsonValue &value : data.value("weaponsList").toArray()) {
        const QJsonObject weapon = value.toObject();
        QVariantMap item;
        item["name"] = JsonUtils::safeGetString(weapon, {"name"});
        item["damage"] = JsonUtils::safeGetString(weapon, {"dmg"});
        item["ability"] = weapon.value("ability").toString("str");
        item["isProf"] = weapon.value("isProf").toBool(true);
        item["magicBonus"] = JsonUtils::safeGetInt(weapon, {"modBonus"});
        item["notes"] = wrappedString(weapon, "notes");
        weapons.append(item);
    }
    out["weapons"] = weapons;

    out["casterClass"] = JsonUtils::safeGetString(data, {"casterClass"});
    QString spellAbility = wrappedString(data, "spellAbility");
    if (spellAbility.isEmpty())
        spellAbility = "int";
    out["spellAbility"] = spellAbility;

    const QJsonObject spells = data.value("spells").toObject();
    QVariantList slotCounts;
    QVariantList expendedSlots;
    const QJsonArray expendedJson = spells.value("expendedSlots").toArray();
    QVariantList spellTexts;
    for (int level = 0; level <= 9; ++level) {
        if (level == 0)
            slotCounts.append(0);
        else
            slotCounts.append(JsonUtils::safeGetInt(spells, {QString("slots-%1").arg(level)}));
        expendedSlots.append(jsonBoolArrayToVariant(
            level < expendedJson.size() ? expendedJson.at(level).toArray() : QJsonArray{}));
        spellTexts.append(tipTapToPlain(text.value(QString("spells-level-%1").arg(level))));
    }
    out["spellSlots"] = slotCounts;
    out["expendedSlots"] = expendedSlots;
    out["spellTexts"] = spellTexts;

    return out;
}

bool AppController::saveCharacterBasics(const QString &path, const QVariantMap &v) {
    clearError();
    CharacterDocument document;
    if (!document.load(path)) {
        setError(tr("Не удалось открыть персонажа"));
        return false;
    }

    QJsonObject data = document.getData();
    if (v.contains("name")) data["name"] = wrapped(v.value("name"));
    if (v.contains("proficiency")) data["proficiency"] = v.value("proficiency").toInt();
    if (v.contains("inspiration")) data["inspiration"] = v.value("inspiration").toBool();

    QJsonObject info = data.value("info").toObject();
    const QList<QPair<QString, QString>> infoKeys = {
        {"charClass", "charClass"}, {"subclass", "charSubclass"},
        {"race", "race"}, {"background", "background"},
        {"alignment", "alignment"}, {"playerName", "playerName"},
        {"experience", "experience"}, {"level", "level"}
    };
    for (const auto &pair : infoKeys)
        if (v.contains(pair.first)) info[pair.second] = wrapped(v.value(pair.first));
    data["info"] = info;

    QJsonObject vitality = data.value("vitality").toObject();
    const QList<QPair<QString, QString>> vitalKeys = {
        {"hp", "hp-current"}, {"hpMax", "hp-max"}, {"hpTemp", "hp-temp"},
        {"armorClass", "ac"}, {"initiative", "initiative"},
        {"speed", "speed"}, {"hitDie", "hit-die"},
        {"passivePerceptionOverride", "passive-perception"}
    };
    for (const auto &pair : vitalKeys) {
        if (!v.contains(pair.first)) continue;
        QVariant value = v.value(pair.first);
        if (pair.first == "initiative" || pair.first == "passivePerceptionOverride")
            value = value.toInt();
        vitality[pair.second] = wrapped(value);
    }
    data["vitality"] = vitality;

    QJsonObject stats = data.value("stats").toObject();
    for (const QString &key : {"str", "dex", "con", "int", "wis", "cha"}) {
        if (!v.contains(key)) continue;
        QJsonObject stat = stats.value(key).toObject();
        stat["score"] = v.value(key).toInt();
        stats[key] = stat;
    }
    data["stats"] = stats;

    if (v.contains("saves")) {
        QJsonObject saves = data.value("saves").toObject();
        const QVariantMap values = v.value("saves").toMap();
        for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
            QJsonObject save = saves.value(it.key()).toObject();
            save["isProf"] = it.value().toBool();
            saves[it.key()] = save;
        }
        data["saves"] = saves;
    }

    if (v.contains("skills")) {
        QJsonObject skills = data.value("skills").toObject();
        for (const QVariant &entry : v.value("skills").toList()) {
            const QVariantMap item = entry.toMap();
            const QString key = item.value("key").toString();
            if (key.isEmpty()) continue;
            QJsonObject skill = skills.value(key).toObject();
            skill["isProf"] = item.value("profLevel").toInt();
            skills[key] = skill;
        }
        data["skills"] = skills;
    }

    if (v.contains("deathSuccess") || v.contains("deathFail")) {
        QJsonObject death = data.value("deathSaves").toObject();
        if (v.contains("deathSuccess"))
            death["success"] = variantBoolArray(v.value("deathSuccess"), 3);
        if (v.contains("deathFail"))
            death["fail"] = variantBoolArray(v.value("deathFail"), 3);
        data["deathSaves"] = death;
    }

    QJsonObject text = data.value("text").toObject();
    const QList<QPair<QString, QString>> textKeys = {
        {"personality", "personality"}, {"ideals", "ideals"},
        {"bonds", "bonds"}, {"flaws", "flaws"},
        {"features", "traits"}, {"equipment", "equipment"}
    };
    for (const auto &pair : textKeys)
        if (v.contains(pair.first))
            text[pair.second] = plainToTipTapValue(v.value(pair.first).toString());

    if (v.contains("spellTexts")) {
        const QVariantList spellTexts = v.value("spellTexts").toList();
        for (int level = 0; level < spellTexts.size() && level <= 9; ++level)
            text[QString("spells-level-%1").arg(level)] =
                plainToTipTapValue(spellTexts.at(level).toString());
    }
    data["text"] = text;

    if (v.contains("weapons")) {
        QJsonArray weapons;
        for (const QVariant &entry : v.value("weapons").toList()) {
            const QVariantMap item = entry.toMap();
            QJsonObject weapon;
            weapon["name"] = wrapped(item.value("name"));
            weapon["dmg"] = wrapped(item.value("damage"));
            weapon["ability"] = item.value("ability", "str").toString();
            weapon["isProf"] = item.value("isProf", true).toBool();
            weapon["modBonus"] = wrapped(item.value("magicBonus", 0));
            if (!item.value("notes").toString().isEmpty())
                weapon["notes"] = wrapped(item.value("notes"));
            weapons.append(weapon);
        }
        data["weaponsList"] = weapons;
    }

    if (v.contains("casterClass"))
        data["casterClass"] = wrapped(v.value("casterClass"));
    if (v.contains("spellAbility"))
        data["spellAbility"] = wrapped(v.value("spellAbility"));

    if (v.contains("spellSlots") || v.contains("expendedSlots")) {
        QJsonObject spells = data.value("spells").toObject();
        if (v.contains("spellSlots")) {
            const QVariantList slotValues = v.value("spellSlots").toList();
            for (int level = 1; level < slotValues.size() && level <= 9; ++level)
                spells[QString("slots-%1").arg(level)] = wrapped(slotValues.at(level).toInt());
        }
        if (v.contains("expendedSlots")) {
            QJsonArray allLevels;
            for (const QVariant &level : v.value("expendedSlots").toList())
                allLevels.append(variantBoolArray(level));
            while (allLevels.size() < 10)
                allLevels.append(QJsonArray{});
            spells["expendedSlots"] = allLevels;
        }
        data["spells"] = spells;
    }

    document.updateFullData(data);
    if (!document.save()) {
        setError(tr("Не удалось сохранить персонажа"));
        return false;
    }

    m_characters->refresh();
    emit characterSaved(path);
    return true;
}

QString AppController::characterPathByName(const QString &name) const {
    QDir dir(Storage::charactersDir());
    for (const QString &file : dir.entryList({"*.json"}, QDir::Files, QDir::Name)) {
        const QString path = dir.absoluteFilePath(file);
        CharacterDocument doc;
        if (doc.load(path) && doc.getName().compare(name, Qt::CaseInsensitive) == 0)
            return path;
    }
    return {};
}

void AppController::clearError() {
    if (m_lastError.isEmpty()) return;
    m_lastError.clear();
    emit lastErrorChanged();
}

void AppController::setError(const QString &message) {
    if (m_lastError == message) return;
    m_lastError = message;
    emit lastErrorChanged();
}

QString AppController::uniqueDestinationPath(const QString &fileName) const {
    QDir dir(Storage::charactersDir());
    QFileInfo info(fileName);
    QString base = info.completeBaseName();
    base.replace(' ', '_');
    if (base.isEmpty()) base = "character";
    const QString ext = info.suffix().isEmpty() ? "json" : info.suffix();
    QString path = dir.filePath(base + "." + ext);
    for (int i = 1; QFileInfo::exists(path); ++i)
        path = dir.filePath(QString("%1_%2.%3").arg(base).arg(i).arg(ext));
    return path;
}
