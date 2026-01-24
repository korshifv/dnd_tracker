#ifndef CHARACTERCARD_H
#define CHARACTERCARD_H

#include <QFrame>
#include <QJsonObject>

// Форвард-декларации (ускоряют компиляцию)
class QLineEdit;
class QSpinBox;
class QLabel;
class QGraphicsOpacityEffect;

class CharacterCard : public QFrame {
    Q_OBJECT
public:
    explicit CharacterCard(QWidget *parent = nullptr);
    virtual ~CharacterCard();

    // Для сортировки карточек в колонке
    int getInitiative() const;
    
    // Анимация "всплытия" (Fade)
    void animateAppearance();

    // Парсинг LSS JSON (двойной парсинг внутреннего поля data)
    void loadLssJson(const QByteArray &rawData);

protected:
    // Событие нажатия мыши (для работы Drag & Drop)
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void applyDamage();   // Кнопка урона [-]
    void applyHeal();     // Кнопка лечения [+]
    void openLssFile();    // Диалог выбора файла
    void showFullSheet();  // Открыть полноценный редактор CharacterSheet

private:
    // Виджеты на карточке
    QLineEdit *nameEdit;
    QSpinBox *hpSpin;
    QSpinBox *modSpin;
    QSpinBox *initSpin;
    QLineEdit *statusEdit;
    QLabel *acLabel;

    // Данные LSS: храним и корень, и персонажа для сохранения правок
    QJsonObject rootLssJson;   // Оригинальный файл целиком
    QJsonObject characterData; // Распарсенное содержимое строки "data"

    // Эффекты
    QGraphicsOpacityEffect *opacityEffect; 
    void setRandomColor(QWidget* widget);
};

#endif // CHARACTERCARD_H