#ifndef INITIATIVETRACKER_H
#define INITIATIVETRACKER_H

#include <QWidget>
#include <QList>

class QHBoxLayout;
class QLabel;
class QPushButton;
class TrackerColumn;
class QTimer;
class CharacterCard;

// Виджет трекера инициативы с единой логикой боя для ПК и мобилок.
// Имеет верхнюю панель управления боем: "СЕЙЧАС ХОДИТ: ...", "Раунд N", кнопку "Следующий ход".
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

public slots:
  // Переход к следующему ходу в бое
  void nextTurn();
  // Сброс боя (раунд 1, первый персонаж)
  void resetCombat();
  // Отсортировать все карточки во всех колонках по инициативе
  void sortAllColumns();

private slots:
  void scheduleSave();
  void addColumn();
  void clearAllData();

private:
  void saveState();
  void loadState();

  // Обновить плашку текущего хода
  void updateTurnBanner();
  // Получить отсортированный список всех карточек боя
  QList<CharacterCard *> getAllCardsSorted() const;

  QHBoxLayout *columnsLayout; // Лейаут для колонок
  QTimer *m_saveTimer;        // Debounce-таймер автосохранения

  // Панель управления боем
  QLabel *m_turnLabel;
  QLabel *m_roundLabel;
  QPushButton *m_nextTurnBtn;
  
  int m_roundCount = 1;
  int m_currentTurnIndex = -1;
};

#endif // INITIATIVETRACKER_H
