#include "NoteRepository.h"
#include "NoteEditor.h"
#include "Storage.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QTreeView>
#include <QFileSystemModel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMenu>
#include <QContextMenuEvent>
#include <QAction>
#include <QSplitter>

NoteRepository::NoteRepository(QWidget *parent) : QWidget(parent) {
  auto *root = new QVBoxLayout(this);
  root->setContentsMargins(0, 0, 0, 0); // Убираем отступы для сплиттера
  
  splitter = new QSplitter(Qt::Horizontal, this);

  // --- Левая панель (Sidebar) ---
  QWidget *sidebar = new QWidget(splitter);
  auto *sidebarLayout = new QVBoxLayout(sidebar);
  sidebarLayout->setContentsMargins(10, 10, 10, 10);
  sidebarLayout->setSpacing(8);

  // Верхняя панель: создание + удаление.
  auto *topBar = new QHBoxLayout();
  auto *createBtn = new QPushButton("+ Заметка");
  createBtn->setFixedHeight(38);
  QFont btnFont = createBtn->font();
  btnFont.setBold(true);
  createBtn->setFont(btnFont);
  connect(createBtn, &QPushButton::clicked, this, &NoteRepository::createNote);

  auto *createFolderBtn = new QPushButton("+ Папка");
  createFolderBtn->setFixedHeight(38);
  createFolderBtn->setFont(btnFont);
  connect(createFolderBtn, &QPushButton::clicked, this, &NoteRepository::createFolder);

  auto *removeBtn = new QPushButton("Удалить");
  removeBtn->setFixedHeight(38);
  removeBtn->setFont(btnFont);
  connect(removeBtn, &QPushButton::clicked, this, &NoteRepository::removeSelected);

  topBar->addWidget(createBtn);
  topBar->addWidget(createFolderBtn);
  topBar->addStretch();
  topBar->addWidget(removeBtn);
  sidebarLayout->addLayout(topBar);

  // Модель файловой системы
  fileModel = new QFileSystemModel(this);
  fileModel->setRootPath(Storage::notesDir());
  QStringList filters;
  filters << "*.md";
  fileModel->setNameFilters(filters);
  fileModel->setNameFilterDisables(false);
  fileModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
  // Поддержка Drag & Drop
  fileModel->setReadOnly(false); 

  // Дерево заметок
  treeView = new QTreeView(sidebar);
  treeView->setModel(fileModel);
  treeView->setRootIndex(fileModel->index(Storage::notesDir()));
  treeView->setSortingEnabled(true);
  treeView->sortByColumn(0, Qt::AscendingOrder);
  // Оставляем только имя, прячем остальные колонки
  for (int i = 1; i < fileModel->columnCount(); ++i) {
    treeView->hideColumn(i);
  }
  treeView->setHeaderHidden(true);
  // Никаких кастомных QSS, стандартные иконки ОС.
  
  // Drag & Drop
  treeView->setDragEnabled(true);
  treeView->setAcceptDrops(true);
  treeView->setDropIndicatorShown(true);
  treeView->setDragDropMode(QAbstractItemView::InternalMove);

  // Контекстное меню
  treeView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(treeView, &QTreeView::customContextMenuRequested, this, [this](const QPoint &pos) {
    QModelIndex index = treeView->indexAt(pos);
    QMenu menu(this);
    
    QAction *newNoteAct = menu.addAction("Новая заметка здесь");
    QAction *newFolderAct = menu.addAction("Новая папка здесь");
    menu.addSeparator();
    QAction *renameAct = nullptr;
    QAction *deleteAct = nullptr;
    
    if (index.isValid()) {
      renameAct = menu.addAction("Переименовать");
      deleteAct = menu.addAction("Удалить");
    }

    QAction *action = menu.exec(treeView->viewport()->mapToGlobal(pos));
    if (!action) return;

    if (action == newNoteAct) {
      if (index.isValid() && fileModel->isDir(index)) {
        treeView->setCurrentIndex(index);
      }
      createNote();
    } else if (action == newFolderAct) {
      if (index.isValid() && fileModel->isDir(index)) {
        treeView->setCurrentIndex(index);
      }
      createFolder();
    } else if (action == renameAct) {
      renameSelected();
    } else if (action == deleteAct) {
      removeSelected();
    }
  });

  // Загружаем заметку по клику (даже одинарному, как в Obsidian)
  connect(treeView, &QTreeView::clicked, this, &NoteRepository::onItemSelected);
          
  sidebarLayout->addWidget(treeView, 1);

  countLabel = new QLabel(sidebar);
  countLabel->setAlignment(Qt::AlignCenter);
  countLabel->setProperty("class", "sublabel");
  sidebarLayout->addWidget(countLabel);
  
  // --- Правая панель (Main Area) ---
  editor = new NoteEditor(splitter);
  // Обработка Wiki-ссылок из редактора
  connect(editor, &NoteEditor::requestOpenCharacter, this, &NoteRepository::requestOpenCharacter);
  connect(editor, &NoteEditor::requestOpenNote, this, [this](const QString &relativePath) {
    // Находим этот файл в модели и выделяем
    QString absPath = Storage::notesDir() + "/" + relativePath + ".md";
    QModelIndex idx = fileModel->index(absPath);
    if (idx.isValid()) {
      treeView->setCurrentIndex(idx);
      treeView->scrollTo(idx);
      onItemSelected(idx);
    }
  });

  splitter->addWidget(sidebar);
  splitter->addWidget(editor);
  splitter->setStretchFactor(0, 1);
  splitter->setStretchFactor(1, 3);
  
  root->addWidget(splitter, 1);

  refresh();
}

void NoteRepository::refresh() {
  countLabel->setText("");
}

void NoteRepository::flushSave() {
  editor->flushSave();
}

void NoteRepository::createNote() {
  bool ok = false;
  const QString name =
      QInputDialog::getText(this, "Новая заметка", "Имя заметки:",
                            QLineEdit::Normal, "", &ok)
          .trimmed();
  if (!ok || name.isEmpty())
    return;

  // Определяем, где создать заметку
  QString targetDir = Storage::notesDir();
  QModelIndex idx = treeView->currentIndex();
  if (idx.isValid()) {
    if (fileModel->isDir(idx)) {
      targetDir = fileModel->filePath(idx);
    } else {
      targetDir = QFileInfo(fileModel->filePath(idx)).absolutePath();
    }
  }

  const QString path = targetDir + "/" + name + ".md";
  if (!QFile::exists(path)) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
      QMessageBox::warning(this, "Ошибка", "Не удалось создать заметку:\n" + path);
      return;
    }
    f.close();
  }
  
  QModelIndex newIdx = fileModel->index(path);
  if (newIdx.isValid()) {
    treeView->setCurrentIndex(newIdx);
    onItemSelected(newIdx);
  }
}

void NoteRepository::createFolder() {
  bool ok = false;
  const QString name =
      QInputDialog::getText(this, "Новая папка", "Имя папки:",
                            QLineEdit::Normal, "", &ok)
          .trimmed();
  if (!ok || name.isEmpty())
    return;

  QString targetDir = Storage::notesDir();
  QModelIndex idx = treeView->currentIndex();
  if (idx.isValid()) {
    if (fileModel->isDir(idx)) {
      targetDir = fileModel->filePath(idx);
    } else {
      targetDir = QFileInfo(fileModel->filePath(idx)).absolutePath();
    }
  }
  
  QDir d(targetDir);
  d.mkdir(name);
}

void NoteRepository::onItemSelected(const QModelIndex &index) {
  if (!index.isValid())
    return;
  
  if (fileModel->isDir(index)) {
    // Не открываем папку как заметку
    return;
  }
    
  QString absPath = fileModel->filePath(index);
  QDir base(Storage::notesDir());
  QString relPath = base.relativeFilePath(absPath);
  if (relPath.endsWith(".md")) relPath.chop(3);
  
  editor->loadNote(relPath);
}

void NoteRepository::renameSelected() {
  QModelIndex idx = treeView->currentIndex();
  if (!idx.isValid()) return;
  
  QString oldPath = fileModel->filePath(idx);
  QFileInfo fi(oldPath);
  
  bool ok = false;
  QString oldName = fi.isDir() ? fi.fileName() : fi.completeBaseName();
  const QString newName =
      QInputDialog::getText(this, "Переименовать", "Новое имя:",
                            QLineEdit::Normal, oldName, &ok)
          .trimmed();
  if (!ok || newName.isEmpty() || newName == oldName)
    return;
    
  QString newPath = fi.absolutePath() + "/" + newName;
  if (!fi.isDir()) newPath += ".md";
  
  // Если переименовываем открытую заметку, сначала flush + clear
  if (!fi.isDir() && editor->getNoteName() + ".md" == fi.fileName()) {
    editor->clearNote();
  }
  
  QDir().rename(oldPath, newPath);
  
  QModelIndex newIdx = fileModel->index(newPath);
  if (newIdx.isValid()) {
    treeView->setCurrentIndex(newIdx);
    onItemSelected(newIdx);
  }
}

void NoteRepository::removeSelected() {
  QModelIndex idx = treeView->currentIndex();
  if (!idx.isValid())
    return;
    
  QString path = fileModel->filePath(idx);
  bool isDir = fileModel->isDir(idx);
  
  if (QMessageBox::question(
          this, "Удаление",
          QString("Удалить %1?\n%2\n\nЭто действие необратимо.")
              .arg(isDir ? "папку и всё её содержимое" : "заметку", path)) !=
      QMessageBox::Yes) {
    return;
  }
  
  // Если удаляем текущую заметку, очищаем редактор
  if (!isDir && editor->getNoteName() + ".md" == QFileInfo(path).fileName()) {
    editor->clearNote();
  } else if (isDir) {
    // В идеале надо рекурсивно проверить, не открыта ли заметка из этой папки.
    // Пока просто clear для надежности
    editor->clearNote();
  }
  
  if (isDir) {
    QDir(path).removeRecursively();
  } else {
    QFile::remove(path);
  }
}

NoteRepository::~NoteRepository() {}

