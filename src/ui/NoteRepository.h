#ifndef NOTEREPOSITORY_H
#define NOTEREPOSITORY_H

#include <QWidget>

class QTreeView;
class QFileSystemModel;
class QLabel;
class QSplitter;
class NoteEditor;

// Вкладка «Заметки»: единое рабочее пространство (дерево папок слева, редактор справа).
class NoteRepository : public QWidget {
  Q_OBJECT
public:
  explicit NoteRepository(QWidget *parent = nullptr);
  ~NoteRepository();

  // Перечитать список файлов. Вызывается после создания/удаления и при
  // сохранении заметки во вкладке (чтобы подхватить новую).
  void refresh();

  // Вызывается из MainWindow::closeEvent
  void flushSave();

signals:
  // Запрос открыть персонажа (от wiki-ссылки в редакторе)
  void requestOpenCharacter(const QString &charName);

private slots:
  void createNote();                 // «+ Новая заметка»
  void createFolder();               // «+ Новая папка»
  void removeSelected();             // Удалить
  void renameSelected();             // Переименовать
  // Одинарный или двойной клик по элементу дерева -> загрузка в редактор
  void onItemSelected(const class QModelIndex &index); 

private:
  QSplitter *splitter;
  QTreeView *treeView;
  QFileSystemModel *fileModel;
  QLabel *countLabel;
  NoteEditor *editor;
};

#endif // NOTEREPOSITORY_H
