#-------------------------------------------------
#
# Project created by QtCreator 2017-06-14T09:08:42
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): CONFIG += c++11

TARGET = TabWidgetPlusDemo
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    tabwidgetplus.cpp \
    formtabtester.cpp

HEADERS  += mainwindow.h \
    tabwidgetplus.h \
    formtabtester.h

FORMS    += mainwindow.ui \
    formtabtester.ui
