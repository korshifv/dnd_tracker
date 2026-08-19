cmake_minimum_required(VERSION 3.21)

project(dnd_tracker VERSION 2.0.1 LANGUAGES CXX)

include(GNUInstallDirs)
include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.8 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    QuickControls2
)

qt_standard_project_setup(REQUIRES 6.8)

set_source_files_properties(qml/Theme.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_add_executable(dnd_tracker
    src/main.cpp

    src/core/CharacterDocument.cpp
    src/core/CharacterDocument.h
    src/core/JsonUtils.cpp
    src/core/JsonUtils.h
    src/core/Storage.h
    src/core/FileUtils.h

    src/app/AppController.cpp
    src/app/AppController.h
    src/app/CharacterRepositoryModel.cpp
    src/app/CharacterRepositoryModel.h
    src/app/InitiativeModel.cpp
    src/app/InitiativeModel.h
    src/app/NotesModel.cpp
    src/app/NotesModel.h
)

qt_add_qml_module(dnd_tracker
    URI DndTracker
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/components/AppButton.qml
        qml/components/LabeledField.qml
        qml/components/Surface.qml
        qml/components/NumberSpinBox.qml
        qml/components/AppTextArea.qml
        qml/components/SquareCheckBox.qml
        qml/pages/InitiativePage.qml
        qml/pages/CharactersPage.qml
        qml/pages/CharacterSheetPage.qml
        qml/pages/CharacterSheetHost.qml
        qml/pages/ClassicCharacterSheetPage.qml
        qml/pages/NotesPage.qml
)

target_include_directories(dnd_tracker PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app
)

target_link_libraries(dnd_tracker PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

if(WIN32)
    set_target_properties(dnd_tracker PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

if(ANDROID)
    set_target_properties(dnd_tracker PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
        QT_ANDROID_MIN_SDK_VERSION 28
        QT_ANDROID_TARGET_SDK_VERSION 36
        QT_ANDROID_SDK_BUILD_TOOLS_REVISION "36.0.0"
        QT_ANDROID_PACKAGE_NAME "org.dnd_tracker.app"
        QT_ANDROID_VERSION_CODE 20001
        QT_ANDROID_VERSION_NAME "${PROJECT_VERSION}"
    )
endif()

if(BUILD_TESTING AND NOT ANDROID)
    find_package(Qt6 6.8 REQUIRED COMPONENTS Test)
    qt_add_executable(dnd_tracker_tests
        tests/CoreTests.cpp
        src/core/CharacterDocument.cpp
        src/core/CharacterDocument.h
        src/core/JsonUtils.cpp
        src/core/JsonUtils.h
    )
    target_include_directories(dnd_tracker_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    )
    target_link_libraries(dnd_tracker_tests PRIVATE
        Qt6::Core
        Qt6::Test
    )
    add_test(NAME core-regressions COMMAND dnd_tracker_tests)
endif()

install(TARGETS dnd_tracker
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(NOT ANDROID)
    qt_generate_deploy_qml_app_script(
        TARGET dnd_tracker
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif()
