#include "CharacterRepository.h"
#include "FileUtils.h"
#include "Storage.h"
#include <QDrag>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFont>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QVBoxLayout>
#include "JsonUtils.h"

// --- RepositoryListWidget: D&D support ---

RepositoryListWidget::RepositoryListWidget(QWidget *parent) : QListWidget(parent) {
  setDragEnabled(true);
}

void RepositoryListWidget::startDrag(Qt::DropActions supportedActions) {
  auto *item = currentItem();
  if (!item)
    return;

  const QString filePath = item->data(Qt::UserRole).toString();
  if (filePath.isEmpty())
    return;

  auto *drag = new QDrag(this);
  auto *mimeData = new QMimeData;
  // Передаём путь к файлу как текст — TrackerColumn его принимает.
  mimeData->setData("application/x-character-filepath", filePath.toUtf8());
  drag->setMimeData(mimeData);
  drag->exec(supportedActions);
}

// --- CharacterRepository ---

CharacterRepository::CharacterRepository(QWidget *parent) : QWidget(parent) {
  auto *root = new QVBoxLayout(this);
  root->setContentsMargins(10, 10, 10, 10);
  root->setSpacing(8);

  // Верхняя панель: импорт + удаление.
  auto *topBar = new QHBoxLayout();
  auto *importBtn = new QPushButton("+ Импорт LSS");
  importBtn->setFixedHeight(38);
  QFont btnFont = importBtn->font();
  btnFont.setBold(true);
  importBtn->setFont(btnFont);
  connect(importBtn, &QPushButton::clicked, this,
          &CharacterRepository::importLss);

  auto *removeBtn = new QPushButton("Удалить выбранный");
  removeBtn->setFixedHeight(38);
  removeBtn->setFont(btnFont);
  connect(removeBtn, &QPushButton::clicked, this,
          &CharacterRepository::removeSelected);

  topBar->addWidget(importBtn);
  topBar->addStretch();
  topBar->addWidget(removeBtn);
  root->addLayout(topBar);

  // Список персонажей (с поддержкой D&D).
  list = new RepositoryListWidget(this);
  list->setAlternatingRowColors(true);
  list->setStyleSheet("QListWidget::item { padding: 8px; }");
  connect(list, &QListWidget::itemActivated, this,
          &CharacterRepository::onItemActivated);
  connect(list, &QListWidget::itemDoubleClicked, this,
          &CharacterRepository::onItemActivated);
  root->addWidget(list, 1);

  countLabel = new QLabel();
  countLabel->setAlignment(Qt::AlignCenter);
  countLabel->setProperty("class", "sublabel");
  root->addWidget(countLabel);

  refresh();
}

void CharacterRepository::refresh() {
  list->clear();

  QDir dir(Storage::charactersDir());
  if (!dir.exists()) {
    countLabel->setText("Папка хранилища не найдена.");
    return;
  }

  const QStringList files = dir.entryList({"*.json"}, QDir::Files, QDir::Name);
  for (const auto &fileName : files) {
    const QString filePath = dir.absoluteFilePath(fileName);
    const QString desc = describeFile(filePath);
    auto *item = new QListWidgetItem(desc, list);
    item->setData(Qt::UserRole, filePath);
    item->setToolTip(filePath);
  }

  countLabel->setText(QString("Персонажей: %1").arg(list->count()));
}

void CharacterRepository::importLss() {
  const QString path =
      QFileDialog::getOpenFileName(this, "Импорт чарника LSS", "", "JSON (*.json)");
  if (path.isEmpty())
    return;

  const QString localPath = FileUtils::copyToData(path);
  if (localPath.isEmpty()) {
    QMessageBox::warning(this, "Ошибка импорта",
                         "Не удалось скопировать файл в хранилище:\n" + path);
    return;
  }
  refresh();
  emit openRequested(localPath);
}

void CharacterRepository::onItemActivated(QListWidgetItem *item) {
  if (!item)
    return;
  const QString filePath = item->data(Qt::UserRole).toString();
  if (!filePath.isEmpty())
    emit openRequested(filePath);
}

void CharacterRepository::removeSelected() {
  auto *item = list->currentItem();
  if (!item)
    return;
  const QString filePath = item->data(Qt::UserRole).toString();
  if (filePath.isEmpty())
    return;

  if (QMessageBox::question(
          this, "Удаление персонажа",
          "Удалить файл персонажа?\n" + QFileInfo(filePath).fileName() +
              "\n\nЭто действие необратимо.") != QMessageBox::Yes) {
    return;
  }
  QFile::remove(filePath);
  refresh();
}

QString CharacterRepository::describeFile(const QString &filePath) {
  QFile f(filePath);
  if (!f.open(QIODevice::ReadOnly))
    return QFileInfo(filePath).fileName() + "  (ошибка чтения)";

  QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
  if (doc.isNull())
    return QFileInfo(filePath).fileName() + "  (не JSON)";

  // Формат LSS: реальные данные вложены в строковое поле "data".
  const QJsonObject root = doc.object();
  const QString innerJson = root.value("data").toString();
  const QJsonObject data = QJsonDocument::fromJson(innerJson.toUtf8()).object();

  // Если это не LSS-формат (нет data), пробуем читать как плоский объект.
  const QJsonObject charData = data.isEmpty() ? root : data;

  const QString name = JsonUtils::safeGetString(charData, {"name"});
  const QString charClass = JsonUtils::safeGetString(charData, {"info", "charClass"});
  const int level = JsonUtils::safeGetInt(charData, {"info", "level"});

  QString desc = name.isEmpty() ? QFileInfo(filePath).completeBaseName() : name;
  if (!charClass.isEmpty())
    desc += "  —  " + charClass;
  if (level > 0)
    desc += QString("  (ур. %1)").arg(level);
  return desc;
}

QString CharacterRepository::filePathForName(const QString &name) {
  QDir dir(Storage::charactersDir());
  if (!dir.exists())
    return {};

  const QStringList files = dir.entryList({"*.json"}, QDir::Files);
  for (const auto &fileName : files) {
    const QString filePath = dir.absoluteFilePath(fileName);
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly))
      continue;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (doc.isNull())
      continue;

    const QJsonObject root = doc.object();
    const QString innerJson = root.value("data").toString();
    const QJsonObject data = QJsonDocument::fromJson(innerJson.toUtf8()).object();
    const QJsonObject charData = data.isEmpty() ? root : data;
    const QString charName = JsonUtils::safeGetString(charData, {"name"});

    if (charName == name)
      return filePath;
  }
  return {};
}

CharacterRepository::~CharacterRepository() {}
