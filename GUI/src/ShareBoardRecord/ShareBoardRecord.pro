#-------------------------------------------------
#
# Project created by QtCreator 2014-06-24T11:32:55
#
#-------------------------------------------------

QT       += core sql network

TARGET = ShareBoardRecord
TEMPLATE = app


SOURCES += main.cpp \
    shareboard.cpp

HEADERS  += \
    shareboard.h

LIBS +=-lmysqlclient
