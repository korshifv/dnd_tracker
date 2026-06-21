#include "JsonUtils.h"

namespace JsonUtils {

QJsonValue safeGetValue(const QJsonObject& obj, const QStringList& path) {
    if (path.isEmpty()) return QJsonValue();
    
    QJsonObject currentObj = obj;
    for (int i = 0; i < path.size() - 1; ++i) {
        const QString& key = path[i];
        if (!currentObj.contains(key) || !currentObj.value(key).isObject()) {
            return QJsonValue(QJsonValue::Undefined);
        }
        currentObj = currentObj.value(key).toObject();
    }
    
    return currentObj.value(path.last());
}

QString safeGetString(const QJsonObject& obj, const QStringList& path, const QString& def) {
    QJsonValue val = safeGetValue(obj, path);
    if (val.isObject()) {
        QJsonObject o = val.toObject();
        if (o.contains("value") && o.value("value").isString()) {
            return o.value("value").toString();
        }
    } else if (val.isString()) {
        return val.toString();
    }
    return def;
}

int safeGetInt(const QJsonObject& obj, const QStringList& path, int def) {
    QJsonValue val = safeGetValue(obj, path);
    if (val.isObject()) {
        QJsonObject o = val.toObject();
        if (o.contains("value") && o.value("value").isDouble()) {
            return o.value("value").toInt();
        }
    } else if (val.isDouble()) {
        return val.toInt();
    }
    return def;
}

bool safeGetBool(const QJsonObject& obj, const QStringList& path, bool def) {
    QJsonValue val = safeGetValue(obj, path);
    if (val.isObject()) {
        QJsonObject o = val.toObject();
        if (o.contains("value") && o.value("value").isBool()) {
            return o.value("value").toBool();
        }
    } else if (val.isBool()) {
        return val.toBool();
    }
    return def;
}

QJsonObject safeGetObject(const QJsonObject& obj, const QStringList& path) {
    QJsonValue val = safeGetValue(obj, path);
    if (val.isObject()) {
        return val.toObject();
    }
    return QJsonObject();
}

} // namespace JsonUtils
