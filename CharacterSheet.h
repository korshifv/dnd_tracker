#ifndef CHARACTERSHEET_H
#define CHARACTERSHEET_H

#include <QDialog>
#include <QJsonObject>
#include <QList>
#include <QMap>

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

class CharacterSheet : public QDialog {
  Q_OBJECT
public:
  explicit CharacterSheet(const QJsonObject &root, const QJsonObject &data,
                          QWidget *parent = nullptr);
  virtual ~CharacterSheet();

private slots:
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

  QString tipTapToPlain(const QJsonObject &obj);
  QJsonObject plainToTipTap(const QString &text);

  QTabWidget *tabs;
  QSpinBox *globalProfSpin;
  QCheckBox *inspirationCheck;
  QLineEdit *passivePerceptionLabel;

  QJsonObject originalRoot;
  QJsonObject originalData;

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
};

#endif // CHARACTERSHEET_H