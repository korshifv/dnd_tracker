#ifndef CHARACTERSHEET_H
#define CHARACTERSHEET_H

#include <QDialog>
#include <QJsonObject>
#include <QMap>

// Форвард-декларации для ускорения сборки
class QTabWidget;
class QSpinBox;
class QLabel;
class QPushButton;
class QTextEdit;
class QLineEdit;
class QVBoxLayout;

// Диалоговое окно редактирования персонажа
class CharacterSheet : public QDialog {
    Q_OBJECT
public:
    // Конструктор принимает оригинал файла и распарсенные данные для сохранения совместимости структуры JSON
    explicit CharacterSheet(const QJsonObject &root, const QJsonObject &data, QWidget *parent = nullptr);
    virtual ~CharacterSheet();

private slots:
    // Пересчитывает все зависимые значения (модификаторы, навыки, атаки) при изменении характеристик
    void updateAllCalculations();              
    // Переключает уровень владения навыком (Не владею -> Владею -> Компетентность)
    void toggleSkillProficiency(const QString &key); 
    // Переключает владение спасброском (Вкл/Выкл)
    void toggleSaveProficiency(const QString &key);  
    // Сохраняет изменения персонажа обратно в JSON файл (формат LSS)
    void saveToFile();                         

private:
    // Методы настройки вкладок интерфейса
    void setupGeneralTab(const QJsonObject &data); // Вкладка "Основа" (Характеристики)
    void setupSkillsTab(const QJsonObject &data);  // Вкладка "Навыки"
    void setupCombatTab(const QJsonObject &data);  // Вкладка "Бой" (Заклинания и Оружие)
    void setupNotesTab(const QJsonObject &data);   // Вкладка "Инфо" (Инвентарь и Заметки)
    
    // Вспомогательные функции
    QString tipTapToPlain(const QJsonObject &obj);  // Конвертация JSON формата редактора TipTap в обычный текст
    QJsonObject plainToTipTap(const QString &text); // Конвертация обычного текста обратно в формат TipTap
    int calculateMod(int score);                    // Расчет модификатора: (значение - 10) / 2
    
    // Создает визуальный блок для одной характеристики (Сила, Ловкость и т.д.)
    QWidget* createStatBox(const QString &label, int score, bool isProfSave, const QString &statKey);

    QTabWidget *tabs;
    QSpinBox *globalProfSpin; // Поле "Бонус мастерства" в верхней панели

    // Хранение оригинальных данных для сохранения структуры файла при экспорте
    QJsonObject originalRoot;
    QJsonObject originalData;

    // Связь логических ключей (например, "str") с элементами интерфейса
    QMap<QString, QSpinBox*> statSpins;       // Поля ввода значений характеристик
    QMap<QString, QLabel*> modLabels;         // Метки отображения модификаторов (+3)
    QMap<QString, QLabel*> saveBonusLabels;   // Метки бонуса спасброска
    QMap<QString, QSpinBox*> skillSpins;      // Поля итоговых значений навыков
    QMap<QString, QLabel*> skillCalcLabels;   // Детализация расчета навыка (+мод +мастерство)
    QMap<QString, QPushButton*> skillProfBtns; // Кнопки переключения владения навыком
    QMap<QString, QPushButton*> saveProfBtns;  // Кнопки переключения владения спасом

    // Текстовые редакторы
    QTextEdit *traitsEdit;    // Особенности и умения
    QTextEdit *inventoryEdit; // Инвентарь
    QTextEdit *notesEdit;     // Заметки
    QLineEdit *nameEditField; // Редактирование имени

    // Структуры данных состояния
    struct SkillInfo { QString baseStat; int profLevel; };
    QMap<QString, SkillInfo> skillsState; // Состояние навыков (базовая хар-ка + уровень владения)
    QMap<QString, bool> savesState;       // Состояние владения спасбросками (да/нет)

    // Структура для отображения оружия в списке
    struct WeaponUI { 
        QLineEdit* nameInput; 
        QLabel* hitLabel; 
        QLineEdit* dmgInput; 
        QString ability; 
        bool isProf; 
        int magicBonus; 
    };
    QList<WeaponUI> weaponsUI; // Список всего оружия персонажа
};

#endif // CHARACTERSHEET_H