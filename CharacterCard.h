#ifndef CHARACTERCARD_H
#define CHARACTERCARD_H

#include <QFrame>
#include <QJsonObject>

// Форвард-декларации (ускоряют компиляцию)
class QLineEdit;
class QSpinBox;
class QLabel;
class QGraphicsOpacityEffect;

// Виджет карточки персонажа в трекере инициативы
// Отображает краткую информацию (HP, КД, Инициатива) и предоставляет доступ к полному листу
class CharacterCard : public QFrame {
    Q_OBJECT
public:
    explicit CharacterCard(QWidget *parent = nullptr);
    virtual ~CharacterCard();

    // Возвращает значение инициативы для сортировки в колонке
    int getInitiative() const;
    
    // Анимация плавного появления (fade-in) при добавлении карточки
    void animateAppearance();

    // Загрузка данных персонажа из полного JSON файла формата LSS
    // Извлекает данные из вложенного поля "data"
    void loadLssJson(const QByteArray &rawData);

protected:
    // Обработка клика мыши (используется для перетаскивания карточки)
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void applyDamage();   // Применить урон (вычесть из HP)
    void applyHeal();     // Применить лечение (добавить к HP)
    void openLssFile();   // Открыть диалог выбора JSON файла персонажа
    void showFullSheet(); // Открыть полное окно редактирования персонажа (CharacterSheet)

private:
    // Виджеты интерфейса карточки
    QLineEdit *nameEdit;  // Имя персонажа
    QSpinBox *hpSpin;     // Текущие хиты
    QSpinBox *modSpin;    // Поле ввода значения урона/лечения
    QSpinBox *initSpin;   // Значение инициативы
    QLineEdit *statusEdit;// Строка статусов (например, "Ослеплен", "Сбит с ног")
    QLabel *acLabel;      // Класс доспеха (Armor Class)

    // Хранение данных LSS
    QJsonObject rootLssJson;   // Исходный файл целиком (нужен для сохранения структуры при экспорте)
    QJsonObject characterData; // Распаршенные игровые данные персонажа

    // Визуальные эффекты
    QGraphicsOpacityEffect *opacityEffect; 
    
    // Устанавливает случайный цвет фона для аватара персонажа
    void setRandomColor(QWidget* widget);
};

#endif // CHARACTERCARD_H