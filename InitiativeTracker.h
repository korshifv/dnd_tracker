#ifndef INITIATIVETRACKER_H
#define INITIATIVETRACKER_H

#include <QWidget>

class QHBoxLayout;
class TrackerColumn;
class QTimer;
class CharacterCard;

// Виджет вкладки трекера инициативы.
// Извлечён из MainWindow при реструктуризации в табовое приложение:
// верхняя панель с группами + горизонтальный скролл колонок персонажей.
// Является источником истины для боевого состояния инициативы.
class InitiativeTracker : public QWidget {
  Q_OBJECT
public:
  explicit InitiativeTracker(QWidget *parent = nullptr);
  ~InitiativeTracker();

  // Сохранить состояние в файл (немедленно, останавливая debounce-таймер).
  void save();
  // Перезагрузить карточки, привязанные к файлу filePath, из файла.
  void reloadCardsForFile(const QString &filePath);

signals:
  // Проброс запроса открытия чарника в MainWindow.
  void sheetRequested(const QString &filePath);
  
  // Проброс запроса на привязку документа в MainWindow.
  void requestDocumentBinding(CharacterCard* card, const QString &filePath);

private slots:
  // Debounce-сохранение: запускает одноразовый таймер на 1500мс.
  // Если таймер уже тикает — перезапускает.
  void scheduleSave();
  // Добавление новой колонки (Группы).
  void addColumn();
  // Очистка всех данных и состояния с подтверждением.
  void clearAllData();

private:
  // Сохранение текущего состояния (колонки, персонажи) в JSON.
  void saveState();
  // Восстановление состояния из JSON при запуске.
  void loadState();

  QHBoxLayout *columnsLayout; // Лейаут для горизонтального размещения колонок
  QTimer *m_saveTimer;        // Debounce-таймер автосохранения (1500мс)
};

#endif // INITIATIVETRACKER_H
