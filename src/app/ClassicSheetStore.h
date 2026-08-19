#ifndef CLASSICSHEETSTORE_H
#define CLASSICSHEETSTORE_H

#include <QObject>
#include <QVariantMap>

class ClassicSheetStore : public QObject {
    Q_OBJECT
public:
    explicit ClassicSheetStore(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE QVariantMap load(const QString &filePath) const;
    Q_INVOKABLE bool save(const QString &filePath, const QVariantMap &values) const;
};

#endif // CLASSICSHEETSTORE_H
