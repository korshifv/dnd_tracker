#ifndef CHARACTERDOCUMENT_H
#define CHARACTERDOCUMENT_H

#include <QObject>
#include <QJsonObject>
#include <QString>
#include <QJsonDocument>
#include <QFile>

class CharacterDocument : public QObject {
    Q_OBJECT
public:
    explicit CharacterDocument(QObject *parent = nullptr);
    
    // Загрузка документа из LSS JSON файла
    bool load(const QString &filePath);
    
    // Сохранение изменений в файл
    bool save();
    
    // Геттеры/сеттеры для базовых свойств
    QString getFilePath() const;
    void setFilePath(const QString &path);
    
    // Доступ к сырым данным JSON (корень и распакованная дата)
    QJsonObject getRoot() const;
    QJsonObject getData() const;
    
    // Обновление всей секции данных (например, из формы CharacterSheet)
    void updateFullData(const QJsonObject &newData);
    
    // Безопасные сеттеры/геттеры для частых боевых параметров
    int getHp() const;
    void setHp(int hp);
    
    int getHpMax() const;
    void setHpMax(int hpMax);
    
    int getHpTemp() const;
    void setHpTemp(int hpTemp);
    
    int getInitiative() const;
    void setInitiative(int init);
    
    int getArmorClass() const;
    void setArmorClass(int ac);
    
    QString getName() const;

signals:
    // Сигнал об изменении HP (используется для синхронизации Карточка <-> Чарник)
    void hpChanged(int newHp);
    
    // Сигнал об изменении любых других данных, если нужно
    void dataChanged();

private:
    QString m_filePath;
    QJsonObject m_rootLssJson;
    QJsonObject m_characterData;
    
    void updateVitalityField(const QString &field, int value);
};

#endif // CHARACTERDOCUMENT_H
