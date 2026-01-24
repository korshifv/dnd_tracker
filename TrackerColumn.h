#ifndef TRACKERCOLUMN_H
#define TRACKERCOLUMN_H
#include <QFrame>

class QVBoxLayout;
class QLineEdit;

class TrackerColumn : public QFrame {
    Q_OBJECT
public:
    explicit TrackerColumn(const QString &title, QWidget *parent = nullptr);
    ~TrackerColumn();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

public slots:
    void addCharacter();
    void sortInitiative();

private:
    QVBoxLayout *listLayout;
    QLineEdit *titleEdit;
};
#endif