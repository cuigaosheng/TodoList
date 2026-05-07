cmake_minimum_required(VERSION 3.16)
project(TodoList)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/taskmanager.cpp
)

set(HEADERS
    src/mainwindow.h
    src/taskmanager.h
    src/task.h
)

add_executable(TodoList ${SOURCES} ${HEADERS})

target_link_libraries(TodoList
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
