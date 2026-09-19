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

# Linux release archives are relocatable. Keep the executable pointed at the
# sibling lib/ directory instead of whatever Qt/ICU happened to be installed on
# the build runner.
if(UNIX AND NOT APPLE AND NOT ANDROID)
    set_target_properties(dnd_tracker PROPERTIES
        BUILD_RPATH_USE_ORIGIN TRUE
        INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
    )
endif()

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

if(UNIX AND NOT APPLE AND NOT ANDROID)
    # The prebuilt Qt Linux SDK ships the ICU ABI it was linked against, but
    # CMake's runtime dependency scanner cannot necessarily resolve that ICU
    # from the host system (and therefore cannot deploy it). Copy the ICU
    # runtime straight from the same Qt installation. Keep the glob
    # version-agnostic so upgrading Qt/ICU does not recreate this bug.
    get_filename_component(qt_runtime_lib_dir "${Qt6Core_DIR}/../.." ABSOLUTE)
    file(GLOB qt_icu_runtime_libs
        LIST_DIRECTORIES FALSE
        "${qt_runtime_lib_dir}/libicu*.so*"
    )
    if(qt_icu_runtime_libs)
        install(FILES ${qt_icu_runtime_libs}
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    endif()
endif()

if(NOT ANDROID)
    set(deploy_qml_args
        TARGET dnd_tracker
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )

    # Official Qt Linux builds depend on the ICU copy shipped with that Qt
    # build. Qt's generic deployment helper normally omits libraries it treats
    # as system runtime dependencies, which made the archive depend on the
    # builder's libicu SONAME (for Qt 6.8.3: libicu*.so.73). Force ICU into the
    # deploy set so users on rolling distributions do not need the exact same
    # ICU major version installed system-wide.
    if(UNIX AND NOT APPLE)
        list(APPEND deploy_qml_args
            POST_INCLUDE_REGEXES ".*/libicu[^/]*\\.so(\\..*)?$"
        )
    endif()

    qt_generate_deploy_qml_app_script(${deploy_qml_args})
    install(SCRIPT ${deploy_script})
endif()
