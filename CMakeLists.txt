cmake_minimum_required(VERSION 3.16)

project(dnd_tracker VERSION 1.0 LANGUAGES CXX)

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
    main.cpp
    MainWindow.cpp
    InitiativeTracker.cpp
    CharacterRepository.cpp
    TrackerColumn.cpp
    CharacterCard.cpp
    CharacterSheet.cpp
    NoteEditor.cpp
    NoteRepository.cpp
    JsonUtils.cpp
    CharacterDocument.cpp
    MarkdownHighlighter.cpp
    # Include headers for IDEs
    MainWindow.h
    InitiativeTracker.h
    CharacterRepository.h
    TrackerColumn.h
    CharacterCard.h
    CharacterSheet.h
    NoteEditor.h
    NoteRepository.h
    Storage.h
    FileUtils.h
    JsonUtils.h
    CharacterDocument.h
    MarkdownHighlighter.h
)

# Executable or Android Shared Library
if(ANDROID)
    add_library(dnd_tracker SHARED ${PROJECT_SOURCES})
    set_target_properties(dnd_tracker PROPERTIES
        QT_ANDROID_PACKAGE_NAME "org.dnd_tracker.app"
    )
else()
    add_executable(dnd_tracker ${PROJECT_SOURCES})
endif()

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

# Installation (optional, but good practice)
install(TARGETS dnd_tracker
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
