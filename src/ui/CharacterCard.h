#ifndef CHARACTERCARD_H
#define CHARACTERCARD_H

#include <QFrame>
#include <QJsonObject>

// Форвард-декларации (ускоряют компиляцию)
class QLineEdit;
class QSpinBox;
class QLabel;
class QPushButton;
class QGraphicsOpacityEffect;
class CharacterDocument;

// Виджет карточки персонажа в трекере инициативы
// Отображает краткую информацию (HP, КД, Инициатива) и предоставляет доступ к
// полному листу
class CharacterCard : public QFrame {
  Q_OBJECT
public:
  explicit CharacterCard(QWidget *parent = nullptr);
  virtual ~CharacterCard();

  // Возвращает значение инициативы для сортировки в колонке
  int getInitiative() const;

  // Возвращает имя персонажа
  QString getName() const;

  // Геттеры и сеттеры для пути к файлу (нужно для сохранения состояния)
  QString getFilePath() const;
  void setFilePath(const QString &path);

  // Анимация плавного появления (fade-in) при добавлении карточки
  void animateAppearance();

  // Получить временное состояние для сохранения трекера
  QJsonObject getEphemeralState() const;
  
  // Восстановить временное состояние при загрузке трекера
  void setEphemeralState(const QJsonObject &state);

  // Привязать документ к карточке
  void setDocument(CharacterDocument *doc);
  CharacterDocument* getDocument() const;

  // Перезагрузить карточку из документа (используется после сохранения чарника)
  void reloadFromDocument();

signals:
  // Запрос открыть полный чарник во вкладке. Вместо создания отдельного окна
  // (раньше так и было — это порождало множество листов, аудит #4) карточка
  // просит родителя открыть чарник.
  void sheetRequested(const QString &filePath);

  // Запрос на привязку документа персонажа (перехватывается MainWindow)
  void documentRequested(CharacterCard* card, const QString &filePath);

  // Привязка/смена персонажа на карточке. InitiativeTracker дебаунс-сохраняет
  // состояние трекера, чтобы привязка не терялась.
  void bindingChanged();

protected:
  // Обработка клика мыши (используется для инициации перетаскивания)
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;

private slots:
  void applyDamage();    // Применить урон (вычесть из HP)
  void applyHeal();      // Применить лечение (добавить к HP)
  void onPersButton();   // Кнопка «перс»: выбор персонажа или открыть чарник
  void requestSheet();   // Запросить открытие чарника (если данные загружены)

private:
  // Виджеты интерфейса карточки
  QLineEdit *nameEdit;   // Имя персонажа
  QSpinBox *hpSpin;      // Текущие хиты
  QSpinBox *modSpin;     // Поле ввода значения урона/лечения
  QSpinBox *initSpin;    // Значение инициативы
  QLineEdit *statusEdit; // Строка статусов (например, "Ослеплен", "Сбит с ног")
  QLabel *acLabel;       // Класс доспеха (Armor Class)
  QPushButton *avatarBtn; // Кнопка-аватар (теперь — выбор цвета)

  // Привязанный документ (nullptr если это безымянный монстр)
  CharacterDocument *m_document = nullptr;
  QString currentFilePath; // Оставляем для совместимости Drag&Drop

  // Визуальные эффекты
  QGraphicsOpacityEffect *opacityEffect;

  // Начальная позиция мыши для старта Drag & Drop (фикс корректного старта).
  QPoint dragStartPos;

  // Синхронизирует боевые правки (HP) в документ.
  void syncVitalityToDocument();

  // Диалог выбора персонажа из хранилища (Storage::charactersDir()).
  // Возвращает выбранный filePath или пустую строку при отмене.
  QString pickCharacterPath();
  // Диалог выбора цвета аватара (RGB-ползунки 0–255).
  void pickColor();

  // Применяет цвет к виджету аватара.
  void setColor(QWidget *widget, int r, int g, int b);
  // Случайный пастельный цвет (используется при создании карточки).
  void setRandomColor(QWidget *widget);
};

#endif // CHARACTERCARD_H