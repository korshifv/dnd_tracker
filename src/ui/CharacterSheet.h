#ifndef CHARACTERSHEET_H
#define CHARACTERSHEET_H

#include <QDialog>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QTimer>

class QTabWidget;
class QSpinBox;
class QLabel;
class QPushButton;
class QTextEdit;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QFrame;
class QGridLayout;
class QVBoxLayout;
class QHBoxLayout;
class QScrollArea;

// Стуктура для хранения данных оружия
struct WeaponData {
  QString name;
  QString ability; // "str", "dex", etc.
  bool isProf;
  int magicBonus;
  QString damage;
  QString notes;
};

// Компактный диалог редактирования оружия
class WeaponEditorDialog : public QDialog {
  Q_OBJECT
public:
  WeaponEditorDialog(const WeaponData &data, int profBonus,
                     const QMap<QString, int> &statMods,
                     QWidget *parent = nullptr);
  WeaponData getWeaponData() const;
  bool isDeleted() const { return deleted; }

private slots:
  void updateHitBonus();

private:
  QLineEdit *nameEdit;
  QComboBox *abilityCombo;
  QCheckBox *profCheck;
  QSpinBox *magicSpin;
  QLineEdit *dmgEdit;
  QTextEdit *notesEdit;
  QLabel *hitBonusLabel;

  int pb;
  QMap<QString, int> mods;
  bool deleted = false;
};

class CharacterDocument;

class CharacterSheet : public QWidget {
  Q_OBJECT
public:
  // filePath — путь к файлу персонажа (для сохранения в то же место и
  // для сигнала saved). Раньше это был QDialog, открывавшийся отдельным окном;
  // теперь QWidget, встраиваемый во вкладку главного окна.
  explicit CharacterSheet(CharacterDocument *doc, QWidget *parent = nullptr);
  virtual ~CharacterSheet();

  QString getFilePath() const;

  // Принудительное немедленное сохранение (без ожидания таймера).
  // Вызывается MainWindow при закрытии вкладки / выходе из приложения.
  void flushSave();

signals:
  // Эмитится после успешного сохранения, чтобы MainWindow перезагрузил
  // карточки с этим filePath (фикс #2 — рассинхрон карточки и листа).
  void saved(const QString &filePath);

private slots:
  // Отметить наличие несохранённых правок: перезапускает таймер автосейва
  // (3с бездействия) и показывает индикатор "Изменения…".
  void markDirty();
  void updateAllCalculations(int = 0);
  void toggleSkillProficiency(const QString &key);
  void toggleSaveProficiency(const QString &key);
  void saveToFile();
  void updateSpellCalculations();
  void updateSpellSlots(int level, int count);
  void addNewWeapon();
  void editWeapon(int index);
  void expandAttacksBlock();
  void shrinkAttacksBlock();
  void onDocumentHpChanged(int newHp);

private:
  void setupMainLayout(QWidget *dashboard, const QJsonObject &data);
  QWidget *createStatBlock(const QString &label, const QString &statKey,
                           int score);
  QWidget *createSavingThrowsBlock(const QJsonObject &data);
  QWidget *createSkillsBlock(const QJsonObject &data);
  QWidget *createCombatStatsBlock(const QJsonObject &data);
  QWidget *createHPBlock(const QJsonObject &data);
  QWidget *createPersonalityBlock(const QJsonObject &data);
  QWidget *createFeaturesBlock(const QJsonObject &data);
  QWidget *createEquipmentBlock(const QJsonObject &data);
  QWidget *createAttacksBlock(const QJsonObject &data);
  QWidget *createHeaderBlock(const QJsonObject &data);
  void setupMagicTab(const QJsonObject &data);

  void refreshWeaponList();

  // Применяет минимальные scoped-стили на основе системной палитры:
  // индикаторы (спасброски/навыки/ячейки), кнопка сохранения, карточные панели.
  // Никаких HEX-цветов — только роли palette(...), чтобы интерфейс
  // автоматически подстраивался под светлую/тёмную тему ОС.
  void applyThemeStyle();

  QString tipTapToPlain(const QJsonObject &obj);
  QJsonObject plainToTipTap(const QString &text);

  QTabWidget *tabs;
  QSpinBox *globalProfSpin;
  QCheckBox *inspirationCheck;
  QLineEdit *passivePerceptionLabel;

  CharacterDocument *m_document;

  QMap<QString, QSpinBox *> statSpins;
  QMap<QString, QLineEdit *> modLabels;
  QMap<QString, QLineEdit *> saveBonusLabels;
  QMap<QString, QSpinBox *> skillSpins;
  QMap<QString, QLabel *> skillCalcLabels;
  QMap<QString, QPushButton *> skillProfBtns;
  QMap<QString, QPushButton *> saveProfBtns;

  QTextEdit *traitsEdit;
  QTextEdit *idealsEdit;
  QTextEdit *bondsEdit;
  QTextEdit *flawsEdit;
  QTextEdit *featuresEdit;
  QTextEdit *inventoryEdit;

  QLineEdit *nameEditField;
  QLineEdit *classEdit;
  QLineEdit *subclassEdit;
  QLineEdit *raceEdit;
  QLineEdit *backgroundEdit;
  QLineEdit *alignmentEdit;
  QSpinBox *levelSpin;
  QLineEdit *expEdit;
  QLineEdit *playerNameEdit;

  QSpinBox *acSpin;
  QLineEdit *initEdit;
  QLineEdit *speedEdit;
  QSpinBox *hpMaxSpin;
  QSpinBox *hpCurrentSpin;
  QSpinBox *hpTempSpin;
  QLineEdit *hdValueEdit;

  QList<QPushButton *> deathSavesSuccess;
  QList<QPushButton *> deathSavesFail;

  QMap<QString, QTextEdit *> magicEdits;
  QLineEdit *spellClassEdit;
  QComboBox *spellAbilityCombo;
  QLineEdit *spellSaveDCEdit;
  QLineEdit *spellAttackBonusEdit;
  QMap<int, QSpinBox *> spellSlotSpins;
  QMap<int, QHBoxLayout *> spellCirclesLayouts;
  // Потраченные ячейки заклинаний по уровням: список указателей на pip-кнопки.
  // Раньше состояние pips не сохранялось (аудит: несохраняемые поля).
  QMap<int, QList<QPushButton *>> spellSlotPips;

  struct SkillInfo {
    QString baseStat;
    int profLevel;
  };
  QMap<QString, SkillInfo> skillsState;
  QMap<QString, bool> savesState;

  QList<WeaponData> weaponsData;
  QVBoxLayout *weaponListLayout;
  QGridLayout *attacksGrid;
  QFrame *attacksBlockFrame;

  // --- Автосейв (debounced) ---
  QTimer *m_autosaveTimer; // single-shot, 3000 мс → saveToFile()
  QTimer *m_statusTimer;   // single-shot, 2000 мс → скрыть индикатор статуса
  QLabel *saveStatusLabel; // "Изменения…" → "Сохранено ✓" → ""
  bool m_loaded = false;   // блокирует markDirty во время начальной настройки
  void connectDirty(QWidget *w);          // helper: подключить виджет к markDirty
  void connectDirtyField(QLineEdit *f);   // QLineEdit::textChanged
  void connectDirtySpin(QSpinBox *s);     // QSpinBox::valueChanged
  void connectDirtyEdit(QTextEdit *e);    // QTextEdit::textChanged
  void connectDirtyCheck(QCheckBox *c);   // QCheckBox::toggled
  void connectDirtyBtn(QPushButton *b);   // QPushButton::toggled/clicked
};

#endif // CHARACTERSHEET_H