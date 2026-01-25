#include "MainWindow.h"
#include "TrackerColumn.h" 
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollArea>

// Конструктор главного окна
MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
    setWindowTitle("D&D Tracker");
    
    // Установка начального размера окна (1200x800)
    resize(1200, 800);
    
    // Основной вертикальный лейаут
    auto *root = new QVBoxLayout(this);
    
    // Кнопка создания новой группы (колонки)
    auto *btn = new QPushButton("+ Создать новую группу", this);
    btn->setFixedHeight(45);
    // Применение стилей к кнопке (темный фон, белый текст, скругление)
    btn->setStyleSheet("background: #333; color: white; border-radius: 8px; font-weight: bold;");
    connect(btn, &QPushButton::clicked, this, &MainWindow::addColumn);

    // Область прокрутки для колонок, чтобы они помещались если их много
    auto *s = new QScrollArea(this);
    s->setWidgetResizable(true); // Контент внутри растягивается
    s->setFrameShape(QFrame::NoFrame); // Без рамок

    // Контейнер для колонок
    QWidget *c = new QWidget();
    columnsLayout = new QHBoxLayout(c);
    columnsLayout->setAlignment(Qt::AlignLeft); // Колонки прижимаются влево
    s->setWidget(c);

    // Добавляем элементы в главный лейаут
    root->addWidget(btn);
    root->addWidget(s);
    
    // Создаем первую колонку по умолчанию
    addColumn(); 
}

// Добавляет новую колонку трекера в интерфейс
void MainWindow::addColumn() {
    // Создаем новую колонку с заголовком "Группа N"
    // columnsLayout->count() + 1 используется для нумерации
    columnsLayout->addWidget(new TrackerColumn("Группа " + QString::number(columnsLayout->count() + 1), this));
}

MainWindow::~MainWindow() {}