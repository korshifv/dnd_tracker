#!/bin/bash
set -e

DEPS_DIR="/home/korshi/android-deps"
NDK_DIR="$DEPS_DIR/ndk/android-ndk-r26b"
QT_DIR="$DEPS_DIR/qt6"
SDK_DIR="$DEPS_DIR/sdk"
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build-android"

export ANDROID_HOME="$SDK_DIR"
export ANDROID_SDK_ROOT="$SDK_DIR"
export ANDROID_NDK_ROOT="$NDK_DIR"
export ANDROID_NDK="$NDK_DIR"
export JAVA_HOME="/usr/lib/jvm/java-17-temurin"
export PATH="$JAVA_HOME/bin:$PATH"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "=== 🚀 Сборка Android APK (v1.1.2) ==="
"$QT_DIR/6.6.3/android_arm64_v8a/bin/qt-cmake" "$PROJECT_DIR" \
    -DANDROID_NDK_ROOT="$NDK_DIR" \
    -DANDROID_SDK_ROOT="$SDK_DIR" \
    -DQT_HOST_PATH="$QT_DIR/6.6.3/gcc_64" \
    -DANDROID_ABI=arm64-v8a

cmake --build . --parallel $(nproc)
cmake --build . --target apk

if [ -f "$BUILD_DIR/android-build/dnd_tracker.apk" ]; then
    cp "$BUILD_DIR/android-build/dnd_tracker.apk" "$PROJECT_DIR/dnd_tracker_v1.1.2.apk"
    echo "=== 🟢 APK успешно собран: $PROJECT_DIR/dnd_tracker_v1.1.2.apk ==="
fi
