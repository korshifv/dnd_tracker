#ifndef CLASSICSHEETSTORE_H
#define CLASSICSHEETSTORE_H

#include <QObject>
#include <QHash>
#include <QJsonObject>
#include <QSet>
#include <QString>
#include <QVariantMap>

class ClassicSheetStore : public QObject {
    Q_OBJECT
public:
    explicit ClassicSheetStore(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE QVariantMap load(const QString &filePath) const;
    Q_INVOKABLE bool save(const QString &filePath, const QVariantMap &values) const;

    // The main character model intentionally exposes spells as plain text.
    // Keep a copy of the original LSS rich-text nodes so classic view can
    // display spell names that live in custom nodes without destroying those
    // nodes merely by saving unrelated character fields.
    Q_INVOKABLE QString spellText(const QString &filePath, int level) const;
    Q_INVOKABLE QString spellLine(const QString &filePath, int level, int index) const;
    Q_INVOKABLE void markSpellEdited(const QString &filePath, int level);
    Q_INVOKABLE bool isSpellLevelEdited(const QString &filePath, int level) const;

private:
    mutable QHash<QString, QJsonObject> m_spellBackups;
    mutable QHash<QString, QSet<int>> m_editedSpellLevels;

    QJsonObject spellBackup(const QString &filePath) const;
};

#endif // CLASSICSHEETSTORE_H