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
#include <QJsonDocument>
#include <QJsonParseError>

namespace {
QJsonObject wrapped(const QVariant &value) {
    return QJsonObject{{"value", QJsonValue::fromVariant(value)}};
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
    const QString sourcePath = url.toLocalFile();
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

    QString fileName = QFileInfo(sourcePath).fileName();
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
    const QJsonObject info = data.value("info").toObject();
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
    out["proficiency"] = data.value("proficiency").toInt(2);
    for (const QString &key : {"str", "dex", "con", "int", "wis", "cha"})
        out[key] = stats.value(key).toObject().value("score").toInt(10);
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
        {"speed", "speed"}, {"hitDie", "hit-die"}
    };
    for (const auto &pair : vitalKeys) {
        if (!v.contains(pair.first)) continue;
        QVariant value = v.value(pair.first);
        if (pair.first == "initiative") value = value.toInt();
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
