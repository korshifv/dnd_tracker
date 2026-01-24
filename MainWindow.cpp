#include "MainWindow.h"
#include "TrackerColumn.h" 
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollArea>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
    // ПРОБЛЕМА: Хардкодированный размер окна - не сохраняется при закрытии
    // РЕКОМЕНДАЦИЯ: Сохранять размер в QSettings
    setWindowTitle("D&D Tracker");
    resize(1200, 800);
    auto *root = new QVBoxLayout(this);
    
    auto *btn = new QPushButton("+ Создать новую группу", this);
    btn->setFixedHeight(45);
    // ПРОБЛЕМА: Инлайн-стили вместо централизованного управления темой
    // РЕКОМЕНДАЦИЯ: Использовать палитру QApplication или QSS файл
    btn->setStyleSheet("background: #333; color: white; border-radius: 8px; font-weight: bold;");
    connect(btn, &QPushButton::clicked, this, &MainWindow::addColumn);

    auto *s = new QScrollArea(this);
    s->setWidgetResizable(true);
    s->setFrameShape(QFrame::NoFrame);

    QWidget *c = new QWidget();
    columnsLayout = new QHBoxLayout(c);
    columnsLayout->setAlignment(Qt::AlignLeft);
    s->setWidget(c);

    root->addWidget(btn);
    root->addWidget(s);
    addColumn(); 
}

void MainWindow::addColumn() {
    columnsLayout->addWidget(new TrackerColumn("Группа " + QString::number(columnsLayout->count() + 1), this));
}

MainWindow::~MainWindow() {}