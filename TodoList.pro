QT += core gui widgets

CONFIG += c++17

TARGET = TodoList
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/taskmanager.cpp

HEADERS += \
    src/mainwindow.h \
    src/taskmanager.h \
    src/task.h
