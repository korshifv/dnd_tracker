#ifndef CHARACTERREPOSITORYMODEL_H
#define CHARACTERREPOSITORYMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QVector>

class CharacterRepositoryModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Role {
        FilePathRole = Qt::UserRole + 1,
        NameRole,
        ClassRole,
        LevelRole,
        HpRole,
        HpMaxRole,
        ArmorClassRole,
        InitiativeRole
    };
    Q_ENUM(Role)

    explicit CharacterRepositoryModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE QString pathAt(int row) const;
    Q_INVOKABLE bool removeAt(int row);

signals:
    void operationFailed(const QString &message);

private:
    struct Entry {
        QString filePath;
        QString name;
        QString charClass;
        int level = 0;
        int hp = 0;
        int hpMax = 0;
        int armorClass = 0;
        int initiative = 0;
    };

    QVector<Entry> m_entries;
    Entry readEntry(const QString &filePath) const;
};

#endif // CHARACTERREPOSITORYMODEL_H
