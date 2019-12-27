#-------------------------------------------------
#
# Project created by QtCreator 2014-05-02T16:32:22
#
#-------------------------------------------------

QT       += core gui
QT       += xml
QT       += sql
QT       += network
TARGET = cs2
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dialog.cpp \
    ftp.cpp

HEADERS  += mainwindow.h \
    ftp.h \
    dialog.h

FORMS    += mainwindow.ui \
    dialog.ui
