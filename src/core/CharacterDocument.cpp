#include "CharacterDocument.h"
#include "JsonUtils.h"

CharacterDocument::CharacterDocument(QObject *parent) : QObject(parent) {}

bool CharacterDocument::load(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;
        
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull())
        return false;
        
    m_rootLssJson = doc.object();
    m_filePath = filePath;
    
    QString innerJsonStr = m_rootLssJson.value("data").toString();
    QJsonDocument innerDoc = QJsonDocument::fromJson(innerJsonStr.toUtf8());
    
    if (!innerDoc.isNull() && !innerDoc.object().isEmpty()) {
        m_characterData = innerDoc.object();
    } else {
        // Fallback если формат не совсем стандартный
        m_characterData = m_rootLssJson;
    }
    
    emit dataChanged();
    return true;
}

bool CharacterDocument::save() {
    if (m_filePath.isEmpty()) return false;
    
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;
        
    // Если данные были вложены в "data", упаковываем обратно
    if (m_rootLssJson.contains("data")) {
        m_rootLssJson["data"] = QString::fromUtf8(QJsonDocument(m_characterData).toJson(QJsonDocument::Compact));
    } else {
        m_rootLssJson = m_characterData;
    }
    
    file.write(QJsonDocument(m_rootLssJson).toJson());
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
    
    // Эмитим сигнал для обновления UI
    emit dataChanged();
}
