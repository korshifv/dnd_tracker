#ifndef JSONUTILS_H
#define JSONUTILS_H

#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QString>

// Утилиты для безопасного парсинга JSON, предотвращающие крэши при неверной структуре.
namespace JsonUtils {

    // Безопасное получение вложенного значения по цепочке ключей.
    // Если на каком-то этапе узел не является объектом (или отсутствует), возвращает QJsonValue(QJsonValue::Undefined).
    QJsonValue safeGetValue(const QJsonObject& obj, const QStringList& path);

    // Безопасное получение строки из LSS формата (обычно структура: { "value": "some_string" }).
    QString safeGetString(const QJsonObject& obj, const QStringList& path, const QString& def = "");

    // Безопасное получение числа из LSS формата (обычно структура: { "value": 10 }).
    int safeGetInt(const QJsonObject& obj, const QStringList& path, int def = 0);

    // Безопасное получение булева значения.
    bool safeGetBool(const QJsonObject& obj, const QStringList& path, bool def = false);

    // Безопасное получение объекта.
    QJsonObject safeGetObject(const QJsonObject& obj, const QStringList& path);

}

#endif // JSONUTILS_H
