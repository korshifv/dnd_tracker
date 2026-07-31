#!/bin/bash
set -e

# Проверка наличия переменных окружения
if [ -z "$ANDROID_NDK_ROOT" ] && [ -z "$ANDROID_NDK" ]; then
    echo "Ошибка: Не задана переменная ANDROID_NDK или ANDROID_NDK_ROOT."
    echo "Установите её, например: export ANDROID_NDK=/path/to/android-ndk"
    exit 1
fi

NDK_PATH="${ANDROID_NDK:-$ANDROID_NDK_ROOT}"

BUILD_DIR="build-android"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "=== Конфигурация CMake для Android (arm64-v8a) ==="
qt-cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$NDK_PATH/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-23 \
    -DQT_ANDROID_BUILD_ALL_ABIS=OFF

echo "=== Компиляция проекта ==="
cmake --build . --parallel $(nproc)

echo "=== Сборка APK через androiddeployqt ==="
cmake --build . --target apk

echo "Сборка завершена! Ищите APK в $BUILD_DIR/android-build/dnd_tracker.apk"
