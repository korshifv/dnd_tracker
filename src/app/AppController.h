#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QUrl>
#include <QVariantMap>

class CharacterRepositoryModel;
class InitiativeModel;
class NotesModel;

class AppController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit AppController(CharacterRepositoryModel *characters,
                           InitiativeModel *initiative,
                           NotesModel *notes,
                           QObject *parent = nullptr);

    QString lastError() const { return m_lastError; }

    Q_INVOKABLE bool importCharacter(const QUrl &sourceUrl);
    Q_INVOKABLE QVariantMap characterDetails(const QString &filePath) const;
    Q_INVOKABLE bool saveCharacterBasics(const QString &filePath,
                                         const QVariantMap &values);
    Q_INVOKABLE QString characterPathByName(const QString &name) const;
    Q_INVOKABLE void clearError();

signals:
    void lastErrorChanged();
    void characterSaved(const QString &filePath);

private:
    CharacterRepositoryModel *m_characters;
    InitiativeModel *m_initiative;
    NotesModel *m_notes;
    QString m_lastError;

    void setError(const QString &message);
    QString uniqueDestinationPath(const QString &fileName) const;
};

#endif // APPCONTROLLER_H
