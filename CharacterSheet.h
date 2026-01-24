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

class CharacterSheet : public QDialog {
    Q_OBJECT
public:
    // Конструктор принимает оригинал файла и распарсенные данные для совместимости при сохранении
    explicit CharacterSheet(const QJsonObject &root, const QJsonObject &data, QWidget *parent = nullptr);
    virtual ~CharacterSheet();

private slots:
    void updateAllCalculations();              // Главная математика (пересчет всего от статов)
    void toggleSkillProficiency(const QString &key); // Циклическое переключение ○ -> ● -> ●●
    void toggleSaveProficiency(const QString &key);  // Переключение владения спасброском
    void saveToFile();                         // Сохранение в JSON (совместимый с LSS)

private:
    // Настройка вкладок
    void setupGeneralTab(const QJsonObject &data);
    void setupSkillsTab(const QJsonObject &data);
    void setupCombatTab(const QJsonObject &data);
    void setupNotesTab(const QJsonObject &data);
    
    // Вспомогательные функции
    QString tipTapToPlain(const QJsonObject &obj);  // JSON LSS -> Обычный текст
    QJsonObject plainToTipTap(const QString &text); // Обычный текст -> JSON LSS
    int calculateMod(int score);                    // (score - 10) / 2
    QWidget* createStatBox(const QString &label, int score, bool isProfSave, const QString &statKey);

    QTabWidget *tabs;
    QSpinBox *globalProfSpin; // Поле "Бонус мастерства" в шапке

    // Данные для сохранения оригинальной структуры
    QJsonObject originalRoot;
    QJsonObject originalData;

    // Мапы для связи логики с виджетами
    QMap<QString, QSpinBox*> statSpins;       // str, dex, con...
    QMap<QString, QLabel*> modLabels;         // Надписи (+5) под статами
    QMap<QString, QLabel*> saveBonusLabels;   // Надписи "Спас: +X"
    QMap<QString, QSpinBox*> skillSpins;      // Итоговые цифры навыков
    QMap<QString, QLabel*> skillCalcLabels;   // Текст "(+5 стат, +4 маст)"
    QMap<QString, QPushButton*> skillProfBtns; // Кнопки владения (○/●/●●)
    QMap<QString, QPushButton*> saveProfBtns;  // Кнопки спасов

    // Текстовые редакторы
    QTextEdit *traitsEdit;    // Особенности
    QTextEdit *inventoryEdit; // Инвентарь
    QTextEdit *notesEdit;     // Заметки
    QLineEdit *nameEditField; // Имя в шапке

    // Состояния владения
    struct SkillInfo { QString baseStat; int profLevel; };
    QMap<QString, SkillInfo> skillsState;
    QMap<QString, bool> savesState;

    // Структура для обновления точности оружия
    struct WeaponUI { 
        QLineEdit* nameInput; 
        QLabel* hitLabel; 
        QLineEdit* dmgInput; 
        QString ability; 
        bool isProf; 
        int magicBonus; 
    };
    QList<WeaponUI> weaponsUI;
};

#endif // CHARACTERSHEET_H