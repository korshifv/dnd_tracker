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
    TrackerColumn.cpp
    CharacterCard.cpp
    CharacterSheet.cpp
    # Include headers for IDEs
    MainWindow.h
    TrackerColumn.h
    CharacterCard.h
    CharacterSheet.h
)

# Executable
add_executable(dnd_tracker ${PROJECT_SOURCES})

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
)

# Copy styles.qss to build directory on every build
add_custom_command(TARGET dnd_tracker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/styles.qss"
    "$<TARGET_FILE_DIR:dnd_tracker>/styles.qss"
)
