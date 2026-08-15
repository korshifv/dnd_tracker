#include "JsonUtils.h"

namespace JsonUtils {

QJsonValue safeGetValue(const QJsonObject &obj, const QStringList &path) {
    if (path.isEmpty())
        return {};

    QJsonObject current = obj;
    for (int i = 0; i < path.size() - 1; ++i) {
        const QString &key = path.at(i);
        const QJsonValue value = current.value(key);
        if (!value.isObject())
            return QJsonValue(QJsonValue::Undefined);
        current = value.toObject();
    }

    return current.value(path.last());
}

QString safeGetString(const QJsonObject &obj, const QStringList &path,
                      const QString &def) {
    const QJsonValue value = safeGetValue(obj, path);
    if (value.isObject()) {
        const QJsonValue inner = value.toObject().value("value");
        if (inner.isString())
            return inner.toString();
        if (inner.isDouble())
            return QString::number(inner.toDouble());
    } else if (value.isString()) {
        return value.toString();
    } else if (value.isDouble()) {
        return QString::number(value.toDouble());
    }
    return def;
}

int safeGetInt(const QJsonObject &obj, const QStringList &path, int def) {
    const QJsonValue value = safeGetValue(obj, path);

    auto parse = [def](const QJsonValue &candidate) {
        if (candidate.isDouble())
            return candidate.toInt();

        if (candidate.isString()) {
            bool ok = false;
            const int parsed = candidate.toString().trimmed().toInt(&ok);
            return ok ? parsed : def;
        }
        return def;
    };

    if (value.isObject())
        return parse(value.toObject().value("value"));
    return parse(value);
}

bool safeGetBool(const QJsonObject &obj, const QStringList &path, bool def) {
    const QJsonValue value = safeGetValue(obj, path);
    if (value.isObject()) {
        const QJsonValue inner = value.toObject().value("value");
        return inner.isBool() ? inner.toBool() : def;
    }
    return value.isBool() ? value.toBool() : def;
}

QJsonObject safeGetObject(const QJsonObject &obj, const QStringList &path) {
    const QJsonValue value = safeGetValue(obj, path);
    return value.isObject() ? value.toObject() : QJsonObject();
}

} // namespace JsonUtils
