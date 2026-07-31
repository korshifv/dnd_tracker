cmake_minimum_required(VERSION 3.16)

project(dnd_tracker VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

# Enable automatic Qt features
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Source files
set(PROJECT_SOURCES
    src/main.cpp
    src/core/CharacterDocument.cpp
    src/core/JsonUtils.cpp
    src/ui/MainWindow.cpp
    src/ui/InitiativeTracker.cpp
    src/ui/CharacterRepository.cpp
    src/ui/TrackerColumn.cpp
    src/ui/CharacterCard.cpp
    src/ui/CharacterSheet.cpp
    src/ui/NoteEditor.cpp
    src/ui/NoteRepository.cpp
    src/ui/MarkdownHighlighter.cpp
)

# Headers for IDEs
set(PROJECT_HEADERS
    src/core/CharacterDocument.h
    src/core/Storage.h
    src/core/FileUtils.h
    src/core/JsonUtils.h
    src/ui/MainWindow.h
    src/ui/InitiativeTracker.h
    src/ui/CharacterRepository.h
    src/ui/TrackerColumn.h
    src/ui/CharacterCard.h
    src/ui/CharacterSheet.h
    src/ui/NoteEditor.h
    src/ui/NoteRepository.h
    src/ui/MarkdownHighlighter.h
    src/ui/TouchUtils.h
    src/ui/MobileTheme.h
)

# Cross-platform Qt executable (handles Android APK packaging automatically)
qt_add_executable(dnd_tracker ${PROJECT_SOURCES} ${PROJECT_HEADERS})

# Include directories so #include "Storage.h" or #include "MainWindow.h" resolves everywhere
target_include_directories(dnd_tracker PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
)

# Target linking
target_link_libraries(dnd_tracker PRIVATE 
    Qt6::Widgets 
    Qt6::Core 
    Qt6::Gui
)

# Windows specific: remove console window for release builds
if(WIN32)
    set_target_properties(dnd_tracker PROPERTIES WIN32_EXECUTABLE ON)
endif()

# Installation
install(TARGETS dnd_tracker
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
