# DnD Tracker

Приложение для отслеживания инициативы и управления персонажами в D&D 5e.

## Сборка (Build)

Проект использует **CMake** и **Qt6**.

### Linux
1. Установите зависимости (напр. `sudo apt install qt6-base-dev cmake g++`).
2. В корне проекта:
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```
3. Запуск: `./dnd_tracker`.

### Windows
1. Установите [Qt Online Installer](https://www.qt.io/download-open-source) и выберите **Qt 6.x** и **CMake**.
2. Откройте файл `CMakeLists.txt` в **Qt Creator** ИЛИ:
3. Используйте терминал (PowerShell/CMD) с установленным компилятором (MSVC или MinGW):
   ```powershell
   mkdir build ; cd build
   cmake ..
   cmake --build . --config Release
   ```
4. Убедитесь, что `styles.qss` находится в одной папке с `.exe` файлом.

## Особенности
- Динамическое управление персонажами.
- Продвинутый редактор оружия с авторасчетом бонусов.
- Отслеживание ячеек заклинаний и спасбросков.
- Поддержка JSON формата (LSS совместимость).
