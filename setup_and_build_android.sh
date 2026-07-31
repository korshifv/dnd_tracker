#!/bin/bash
set -e

DEPS_DIR="/home/korshi/android-deps"
mkdir -p "$DEPS_DIR"

echo "=== 1. Download Android NDK r26b ==="
NDK_ZIP="$DEPS_DIR/ndk.zip"
NDK_DIR="$DEPS_DIR/ndk/android-ndk-r26b"

if [ ! -d "$NDK_DIR" ]; then
    echo "Downloading Android NDK..."
    curl -L --progress-bar -o "$NDK_ZIP" "https://dl.google.com/android/repository/android-ndk-r26b-linux.zip"
    echo "Unpacking NDK..."
    mkdir -p "$DEPS_DIR/ndk"
    unzip -q "$NDK_ZIP" -d "$DEPS_DIR/ndk"
    rm -f "$NDK_ZIP"
    echo "NDK ready at $NDK_DIR"
else
    echo "NDK already present at $NDK_DIR"
fi

echo "=== 2. Download Qt 6.6.3 for Android arm64_v8a ==="
QT_DIR="$DEPS_DIR/qt6"
if [ ! -d "$QT_DIR/6.6.3/android_arm64_v8a" ]; then
    echo "Downloading Qt 6.6.3 Android binaries..."
    /home/korshi/.local/bin/aqt install-qt linux android 6.6.3 android_arm64_v8a --outputdir "$QT_DIR"
    echo "Qt Android ready."
else
    echo "Qt Android already present."
fi

echo "=== 3. Building Android APK ==="
export ANDROID_NDK_ROOT="$NDK_DIR"
export ANDROID_NDK="$NDK_DIR"
export JAVA_HOME="/usr/lib/jvm/default"

PROJECT_DIR="/home/korshi/nextcloud/code/dnd_tracker"
BUILD_DIR="$PROJECT_DIR/build-android"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Running qt-cmake..."
"$QT_DIR/6.6.3/android_arm64_v8a/bin/qt-cmake" "$PROJECT_DIR" \
    -DANDROID_NDK_ROOT="$NDK_DIR" \
    -DQT_HOST_PATH="$QT_DIR/6.6.3/gcc_64" \
    -DANDROID_ABI=arm64-v8a

echo "Compiling C++ sources for arm64-v8a..."
cmake --build . --parallel $(nproc)

echo "Building APK package..."
cmake --build . --target apk

echo "=== SUCCESS! ==="
find "$BUILD_DIR" -name "*.apk"
