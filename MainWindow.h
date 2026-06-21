#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHash>
#include <QWidget>

class QTabWidget;
class InitiativeTracker;
class CharacterRepository;
class NoteRepository;
class CharacterDocument;

// Главное окно приложения — контейнер вкладок (browser-style).
// Вкладки:
//   [0] «Инициатива»   — InitiativeTracker (незакрываемая)
//   [1] «Чарники»      — CharacterRepository (незакрываемая)
//   [2] «Заметки»      — NoteRepository (незакрываемая)
//   [3..N] «<Имя> ×»   — открытые чарники/заметки (закрываемые)
class MainWindow : public QWidget {
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  // Количество фиксированных (незакрываемых) вкладок.
  static constexpr int FIXED_TAB_COUNT = 3;

protected:
  // Перехват закрытия окна: flushSave всех динамических вкладок + save трекера.
  void closeEvent(QCloseEvent *event) override;

private slots:
  // Запрос на открытие чарника во вкладке (от трекера или хранилища).
  void openCharacterSheet(const QString &filePath);
  // Закрытие вкладки по крестику (индексы < FIXED_TAB_COUNT игнорируем).
  void onTabCloseRequested(int index);

private:
  // Получить или загрузить документ персонажа (кэшируется)
  CharacterDocument* getDocument(const QString &filePath);

  // Открыть/переключиться на вкладку чарника. filePath — путь к LSS-файлу.
  // Дедупликация: динамический поиск вкладки по filePath.
  void openSheetTab(const QString &filePath, const QString &title);

  QTabWidget *tabs;
  InitiativeTracker *tracker;
  CharacterRepository *repository;
  NoteRepository *noteRepo;

  // Кэш загруженных документов: filePath -> CharacterDocument*
  QHash<QString, CharacterDocument*> m_documents;
};

#endif // MAINWINDOW_H
