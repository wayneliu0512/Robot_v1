#-------------------------------------------------
#
# Project created by QtCreator 2017-10-21T15:28:33
#
#-------------------------------------------------

QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Robot_v1
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp\
        mainwindow.cpp \
    communication.cpp \
    tooling.cpp \
    robot.cpp \
    ccd.cpp \
    task.cpp \
    taskmanager.cpp \
    light.cpp \
    prescangroupbox.cpp \
    dynamicsetting.cpp \
    base.cpp \
    database.cpp

HEADERS  += mainwindow.h \
    communication.h \
    tooling.h \
    robot.h \
    ccd.h \
    task.h \
    taskmanager.h \
    light.h \
    prescangroupbox.h \
    dynamicsetting.h \
    base.h \
    database.h

FORMS    += mainwindow.ui \
    tooling.ui \
    robot.ui \
    ccd.ui \
    prescangroupbox.ui

RESOURCES += \
    myresourse.qrc
