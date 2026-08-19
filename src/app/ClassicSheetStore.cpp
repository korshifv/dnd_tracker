#include "ClassicSheetStore.h"
#include "CharacterDocument.h"

#include <QJsonObject>

QVariantMap ClassicSheetStore::load(const QString &filePath) const {
    CharacterDocument document;
    if (!document.load(filePath))
        return {};

    return document.getData().value("dndTrackerClassic").toObject().toVariantMap();
}

bool ClassicSheetStore::save(const QString &filePath, const QVariantMap &values) const {
    CharacterDocument document;
    if (!document.load(filePath))
        return false;

    QJsonObject data = document.getData();
    QJsonObject extras = data.value("dndTrackerClassic").toObject();
    const QJsonObject incoming = QJsonObject::fromVariantMap(values);
    for (auto it = incoming.constBegin(); it != incoming.constEnd(); ++it)
        extras.insert(it.key(), it.value());

    data.insert("dndTrackerClassic", extras);
    document.updateFullData(data);
    return document.save();
}
