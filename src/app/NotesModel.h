#ifndef NOTESMODEL_H
#define NOTESMODEL_H

#include <QAbstractListModel>
#include <QVector>

class NotesModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Role { TitleRole = Qt::UserRole + 1, RelativePathRole, IsFolderRole, DepthRole };
    Q_ENUM(Role)

    explicit NotesModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE QString loadText(const QString &relativePath) const;
    Q_INVOKABLE bool saveText(const QString &relativePath, const QString &text);
    Q_INVOKABLE bool createNote(const QString &parentPath, const QString &name);
    Q_INVOKABLE bool createFolder(const QString &parentPath, const QString &name);
    Q_INVOKABLE bool renameAt(int row, const QString &newName);
    Q_INVOKABLE bool moveAt(int row, const QString &targetFolder);
    Q_INVOKABLE bool removeAt(int row);
    Q_INVOKABLE QString pathByTitle(const QString &title) const;

signals:
    void operationFailed(const QString &message);

private:
    struct Entry {
        QString title;
        QString relativePath;
        bool isFolder = false;
        int depth = 0;
    };

    QVector<Entry> m_entries;
    QString absolutePath(const QString &relativePath) const;
    QString sanitizeName(const QString &name) const;
    void scanDirectory(const QString &absoluteDir, const QString &relativeDir,
                       int depth, QVector<Entry> &target) const;
};

#endif
