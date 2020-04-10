#-------------------------------------------------
#
# Project created by QtCreator 2019-05-04T23:32:02
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = elevator_sim
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    elevator.cpp \
    building.cpp \
    controller.cpp

HEADERS += \
        mainwindow.h \
    elevator.h \
    building.h \
    controller.h

FORMS += \
        mainwindow.ui \
    elevator.ui \
    building.ui \
    controller.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
RC_ICONS = "../img/icon.ico"

QMAKE_CXXFLAGS_RELEASE += -g

QMAKE_CFLAGS_RELEASE += -g

QMAKE_LFLAGS_RELEASE = -mthreads -Wl
