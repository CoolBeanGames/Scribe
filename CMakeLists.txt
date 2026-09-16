cmake_minimum_required(VERSION 3.20)
project(Scribe VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Try Qt6 first, fall back to Qt5
find_package(Qt6 COMPONENTS Core Gui Widgets PrintSupport QUIET)
if(Qt6_FOUND)
    message(STATUS "Using Qt6 ${Qt6_VERSION}")
    set(QT_VERSION_MAJOR 6)
    set(QT_LIBS Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport)
else()
    find_package(Qt5 COMPONENTS Core Gui Widgets PrintSupport REQUIRED)
    message(STATUS "Using Qt5 ${Qt5_VERSION}")
    set(QT_VERSION_MAJOR 5)
    set(QT_LIBS Qt5::Core Qt5::Gui Qt5::Widgets Qt5::PrintSupport)
endif()

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/PlainTextEditor.cpp
    src/PlainTextEditor.h
    src/RichTextEditor.cpp
    src/RichTextEditor.h
    src/SpreadsheetEditor.cpp
    src/SpreadsheetEditor.h
    src/CodeEditor.cpp
    src/CodeEditor.h
    src/CodeEditorWidget.cpp
    src/FlowLayout.cpp
    src/FlowLayout.h
    src/CodeEditorWidget.h
    src/PythonHighlighter.cpp
    src/PythonHighlighter.h
    src/CSharpHighlighter.cpp
    src/CSharpHighlighter.h
    src/JsonHighlighter.cpp
    src/JsonHighlighter.h
    src/HtmlHighlighter.cpp
    src/HtmlHighlighter.h
    src/CssHighlighter.cpp
    src/CssHighlighter.h
    src/XmlHighlighter.cpp
    src/XmlHighlighter.h
    src/TabWidget.cpp
    src/TabWidget.h
    src/EditorBase.h
    src/Theme.h
    src/InsertTableDialog.cpp
    src/InsertTableDialog.h
    src/InsertLinkDialog.cpp
    src/InsertLinkDialog.h
    src/FileAssociations.cpp
    src/FileAssociations.h
    src/DefaultAppDialog.cpp
    src/DefaultAppDialog.h
    scribe.qrc
    scribe.rc
)

add_executable(Scribe WIN32 ${SOURCES})

target_link_libraries(Scribe PRIVATE ${QT_LIBS})
if(WIN32)
    target_link_libraries(Scribe PRIVATE shell32 advapi32 ole32 shlwapi)
endif()

target_include_directories(Scribe PRIVATE src)

if(WIN32)
    set_target_properties(Scribe PROPERTIES
        WIN32_EXECUTABLE TRUE
    )

    # Keep every Windows build directly launchable from its output directory.
    # Resolve windeployqt beside the qmake imported by the selected Qt package so
    # Debug builds receive debug DLLs and Release builds receive release DLLs.
    if(QT_VERSION_MAJOR EQUAL 6)
        get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    else()
        get_target_property(QT_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    endif()

    get_filename_component(QT_BIN_DIRECTORY "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE
        NAMES windeployqt
        HINTS "${QT_BIN_DIRECTORY}"
        NO_DEFAULT_PATH
    )
    if(NOT WINDEPLOYQT_EXECUTABLE)
        message(FATAL_ERROR "windeployqt was not found beside ${QT_QMAKE_EXECUTABLE}")
    endif()

    add_custom_command(TARGET Scribe POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-translations
                "$<TARGET_FILE:Scribe>"
        COMMENT "Deploying Qt runtime dependencies"
        VERBATIM
    )
endif()


