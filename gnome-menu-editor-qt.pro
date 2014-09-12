#-------------------------------------------------
#
# Project created by QtCreator 2014-07-21T10:14:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gnome-menu-editor-qt
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    editdfile.cpp

HEADERS  += mainwindow.h \
    editdfile.h \
    env.h

FORMS    += mainwindow.ui \
    editdfile.ui

QMAKE_CXXFLAGS -= -O2
QMAKE_CFLAGS -= -O2
QMAKE_CXXFLAGS += -std=c++11 -O3
QMAKE_CFLAGS += -O3
