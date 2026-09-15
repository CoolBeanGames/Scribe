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
endif()

# Post-build: deploy Qt DLLs (optional, run windeployqt manually if needed)


