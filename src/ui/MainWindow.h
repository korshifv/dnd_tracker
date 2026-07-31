#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHash>
#include <QWidget>

class QTabWidget;
class QStackedWidget;
class QLabel;
class QPushButton;
class InitiativeTracker;
class CharacterRepository;
class NoteRepository;
class CharacterDocument;
class CharacterSheet;

// Главное окно приложения.
// Использует QStackedWidget для полноценной полноэкранной навигации:
//   Стек [0]: Главный экран приложения (3 фиксированные вкладки: Инициатива, Чарники, Заметки)
//   Стек [1]: Полноэкранный просмотрщик/редактор чарника (с кнопкой "← Назад")
class MainWindow : public QWidget {
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

protected:
  void closeEvent(QCloseEvent *event) override;

private slots:
  // Полноэкранное открытие чарника из хранилища или инициативы
  void openCharacterSheet(const QString &filePath);
  // Возврат из полноэкранного чарника на главный экран
  void closeCharacterSheet();

private:
  CharacterDocument* getDocument(const QString &filePath);

  QStackedWidget *m_stack;        // Стековый переключатель экранов
  QTabWidget *tabs;               // 3 фиксированные главные вкладки

  InitiativeTracker *tracker;
  CharacterRepository *repository;
  NoteRepository *noteRepo;

  // Контейнер полноэкранного просмотра чарника (Страница 1)
  QWidget *m_sheetContainer;
  QLabel *m_sheetTitleLabel;
  CharacterSheet *m_activeSheet = nullptr;

  // Кэш загруженных документов: filePath -> CharacterDocument*
  QHash<QString, CharacterDocument*> m_documents;
};

#endif // MAINWINDOW_H
