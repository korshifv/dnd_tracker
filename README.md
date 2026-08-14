# DnD Tracker 2.0

Кроссплатформенный трекер для D&D 5e на **Qt 6 / Qt Quick / QML**.

Один интерфейс и одна C++-модель данных используются на Windows, Linux и Android. Приложение умеет работать с существующими JSON/LSS-персонажами, вести инициативу и хранить Markdown-заметки.

## Возможности

- импорт и редактирование JSON/LSS-персонажей;
- характеристики, спасброски, навыки, HP, AC, инициатива и death saves;
- оружие, экипировка, черты и текстовые секции листа;
- заклинания, spellcasting ability и ячейки заклинаний;
- трекер инициативы с раундами, текущим ходом, уроном и лечением;
- синхронизация HP участника инициативы с привязанным файлом персонажа;
- древовидные Markdown-заметки с `[[wiki-links]]`;
- адаптивный QML-интерфейс для desktop и mobile;
- атомарная запись файлов персонажей и заметок.

## Готовые сборки

Release workflow формирует три артефакта:

- `dnd_tracker-windows-x64.zip`;
- `dnd_tracker-linux-x64.tar.gz`;
- `dnd_tracker-android-arm64.apk`.

Desktop-архивы содержат deployable tree с требуемыми Qt/QML runtime-зависимостями. Android APK собирается Qt Android toolchain для `arm64-v8a`.

> APK из CI не заменяет production signing. Для публикации в магазине нужен собственный signing key и соответствующая настройка подписи.

## Сборка из исходников

Требования:

- CMake 3.21+;
- C++17 toolchain;
- Qt 6.8+ с `Core`, `Gui`, `Qml`, `Quick`, `QuickControls2`;
- Ninja рекомендуется, но не обязателен.

### Windows / Linux

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Чтобы получить переносимое дерево приложения с runtime-зависимостями Qt/QML:

```bash
cmake --install build --prefix package
```

### Android

Нужны JDK 17, Android SDK, NDK и Qt 6.8 Android kit. CI использует API 36, Build Tools 36.0.0 и NDK `27.2.12479018`.

Пример после настройки Android Qt kit:

```bash
/path/to/Qt/6.8.x/android_arm64_v8a/bin/qt-cmake \
  -S . -B build-android -G Ninja \
  -DQT_HOST_PATH=/path/to/Qt/6.8.x/gcc_64 \
  -DANDROID_SDK_ROOT=/path/to/android-sdk \
  -DANDROID_NDK_ROOT=/path/to/android-sdk/ndk/27.2.12479018 \
  -DANDROID_ABI=arm64-v8a

cmake --build build-android --target apk --parallel
```

## Данные

По умолчанию данные сохраняются через `QStandardPaths::AppDataLocation`, то есть в корректной пользовательской директории для каждой ОС. Для разработки и тестов корень данных можно переопределить переменной окружения:

```text
DND_TRACKER_DATA_DIR=/custom/path
```

Персонажи и заметки хранятся в отдельных подкаталогах. Старые JSON/LSS-поля, которые приложение не редактирует, сохраняются при записи вместо полной пересборки документа.

## CI

Pull request CI собирает и проверяет:

- Linux x64;
- Windows x64;
- Android ARM64.

Desktop jobs также запускают regression-тесты core-логики. Tag `v*` запускает отдельную упаковку Windows/Linux/Android и публикацию GitHub Release только после успешной сборки всех трёх артефактов.
