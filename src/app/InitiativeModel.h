#ifndef INITIATIVEMODEL_H
#define INITIATIVEMODEL_H

#include <QAbstractListModel>
#include <QJsonObject>
#include <QList>
#include <QTimer>
#include <QVector>

class InitiativeModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int round READ round NOTIFY roundChanged)
    Q_PROPERTY(QString currentTurnId READ currentTurnId NOTIFY currentTurnChanged)
    Q_PROPERTY(QString currentTurnName READ currentTurnName NOTIFY currentTurnChanged)
    Q_PROPERTY(bool hasCombatants READ hasCombatants NOTIFY countChanged)

public:
    enum Role {
        IdRole = Qt::UserRole + 1,
        NameRole, HpRole, ArmorClassRole, InitiativeRole,
        StatusRole, FilePathRole, GroupRole, AvatarColorRole, ActiveRole
    };
    Q_ENUM(Role)

    explicit InitiativeModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int round() const { return m_round; }
    QString currentTurnId() const { return m_currentTurnId; }
    QString currentTurnName() const;
    bool hasCombatants() const { return !m_items.isEmpty(); }

    Q_INVOKABLE void addBlank(const QString &group = QString());
    Q_INVOKABLE bool addCharacter(const QString &filePath, const QString &group = QString());
    Q_INVOKABLE void removeAt(int row);
    Q_INVOKABLE void setName(int row, const QString &name);
    Q_INVOKABLE void setInitiative(int row, int value);
    Q_INVOKABLE void setStatus(int row, const QString &status);
    Q_INVOKABLE void setGroup(int row, const QString &group);
    Q_INVOKABLE void setAvatarColor(int row, const QString &color);
    Q_INVOKABLE void applyDamage(int row, int amount);
    Q_INVOKABLE void applyHeal(int row, int amount);
    Q_INVOKABLE void sortByInitiative();
    Q_INVOKABLE void nextTurn();
    Q_INVOKABLE void resetCombat();
    Q_INVOKABLE void clearAll();
    Q_INVOKABLE void flush();

signals:
    void roundChanged();
    void currentTurnChanged();
    void countChanged();
    void operationFailed(const QString &message);

private:
    struct Combatant {
        QString id;
        QString name;
        int hp = 10;
        int armorClass = 0;
        int initiative = 0;
        QString status;
        QString filePath;
        QString group;
        QString avatarColor;
    };

    QVector<Combatant> m_items;
    int m_round = 1;
    QString m_currentTurnId;
    QTimer m_saveTimer;

    void scheduleSave();
    void saveState();
    void loadState();
    bool loadV2(const QJsonObject &root);
    bool loadLegacy(const QJsonObject &root);
    QString portableCharacterRef(const QString &filePath) const;
    QString resolveCharacterRef(const QString &ref) const;
    void syncHpToCharacter(const Combatant &item);
    int indexOfId(const QString &id) const;
    void emitRowChanged(int row, const QList<int> &roles);
};

#endif
