#include "CharacterDocument.h"
#include "JsonUtils.h"

#include <QJsonParseError>
#include <QSaveFile>

CharacterDocument::CharacterDocument(QObject *parent) : QObject(parent) {}

bool CharacterDocument::load(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonParseError rootError;
    const QJsonDocument rootDocument = QJsonDocument::fromJson(file.readAll(), &rootError);
    if (rootError.error != QJsonParseError::NoError || !rootDocument.isObject())
        return false;

    const QJsonObject root = rootDocument.object();
    QJsonObject characterData;

    if (root.contains("data")) {
        const QJsonValue dataValue = root.value("data");
        if (!dataValue.isString())
            return false;

        QJsonParseError innerError;
        const QJsonDocument innerDocument =
            QJsonDocument::fromJson(dataValue.toString().toUtf8(), &innerError);
        if (innerError.error != QJsonParseError::NoError || !innerDocument.isObject())
            return false;
        characterData = innerDocument.object();
    } else {
        characterData = root;
    }

    m_rootLssJson = root;
    m_characterData = characterData;
    m_filePath = filePath;
    emit dataChanged();
    return true;
}

bool CharacterDocument::save() {
    if (m_filePath.isEmpty())
        return false;

    QJsonObject root = m_rootLssJson;
    if (root.contains("data")) {
        root["data"] = QString::fromUtf8(
            QJsonDocument(m_characterData).toJson(QJsonDocument::Compact));
    } else {
        root = m_characterData;
    }

    QSaveFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    if (file.write(QJsonDocument(root).toJson(QJsonDocument::Indented)) < 0)
        return false;
    if (!file.commit())
        return false;

    m_rootLssJson = root;
    return true;
}

QString CharacterDocument::getFilePath() const { return m_filePath; }
void CharacterDocument::setFilePath(const QString &path) { m_filePath = path; }

QJsonObject CharacterDocument::getRoot() const { return m_rootLssJson; }
QJsonObject CharacterDocument::getData() const { return m_characterData; }

void CharacterDocument::updateFullData(const QJsonObject &newData) {
    m_characterData = newData;
    emit dataChanged();
}

QString CharacterDocument::getName() const {
    return JsonUtils::safeGetString(m_characterData, {"name"});
}

int CharacterDocument::getHp() const {
    return JsonUtils::safeGetInt(m_characterData, {"vitality", "hp-current"});
}

void CharacterDocument::setHp(int hp) {
    if (getHp() == hp) return;
    updateVitalityField("hp-current", hp);
    emit hpChanged(hp);
}

int CharacterDocument::getHpMax() const {
    return JsonUtils::safeGetInt(m_characterData, {"vitality", "hp-max"});
}

void CharacterDocument::setHpMax(int hpMax) {
    updateVitalityField("hp-max", hpMax);
}

int CharacterDocument::getHpTemp() const {
    return JsonUtils::safeGetInt(m_characterData, {"vitality", "hp-temp"});
}

void CharacterDocument::setHpTemp(int hpTemp) {
    updateVitalityField("hp-temp", hpTemp);
}

int CharacterDocument::getInitiative() const {
    return JsonUtils::safeGetInt(m_characterData, {"vitality", "initiative"});
}

void CharacterDocument::setInitiative(int init) {
    updateVitalityField("initiative", init);
}

int CharacterDocument::getArmorClass() const {
    return JsonUtils::safeGetInt(m_characterData, {"vitality", "ac"});
}

void CharacterDocument::setArmorClass(int ac) {
    updateVitalityField("ac", ac);
}

void CharacterDocument::updateVitalityField(const QString &field, int value) {
    QJsonObject vitality = m_characterData.value("vitality").toObject();
    QJsonObject fieldObj = vitality.value(field).toObject();
    fieldObj["value"] = value;
    vitality[field] = fieldObj;
    m_characterData["vitality"] = vitality;
    emit dataChanged();
}
