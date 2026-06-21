#ifndef CHARACTERREPOSITORY_H
#define CHARACTERREPOSITORY_H

#include <QListWidget>
#include <QWidget>

class QLabel;

// Подкласс QListWidget с поддержкой Drag & Drop для перетаскивания файлов
// персонажей из хранилища в трекер инициативы.
// MIME: application/x-character-filepath — текст с путём к файлу JSON.
class RepositoryListWidget : public QListWidget {
  Q_OBJECT
public:
  explicit RepositoryListWidget(QWidget *parent = nullptr);

protected:
  void startDrag(Qt::DropActions supportedActions) override;
};

// Вкладка хранилища персонажей.
// Сканирует Storage::charactersDir() на LSS-файлы, показывает список,
// позволяет импортировать новые, открывать и удалять персонажей.
// Сама не открывает чарник — эмитит openRequested, MainWindow создаёт вкладку.
class CharacterRepository : public QWidget {
  Q_OBJECT
public:
  explicit CharacterRepository(QWidget *parent = nullptr);
  ~CharacterRepository();

  // Перечитать список файлов из хранилища.
  void refresh();

  // Извлечь краткую инфу (имя/класс/уровень) из LSS-файла для отображения.
  // Публичный статический — нужен CharacterCard::pickCharacterPath() и другим.
  static QString describeFile(const QString &filePath);

  // Найти путь к файлу персонажа по имени. Сканиит charactersDir(),
  // парсит JSON, сравнивает поле name. Возвращает пустую строку при неудаче.
  static QString filePathForName(const QString &name);

signals:
  void openRequested(const QString &filePath);

private slots:
  void importLss();
  void onItemActivated(QListWidgetItem *item);
  void removeSelected();

private:
  RepositoryListWidget *list;
  QLabel *countLabel;
};

#endif // CHARACTERREPOSITORY_H
