#-------------------------------------------------
#
# Project created by QtCreator 2018-11-06T22:15:54
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WebServerBySocket
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    server.cpp \
    config.cpp \
    responseresult.cpp \
    respondthread.cpp

HEADERS  += mainwindow.h \
    server.h \
    config.h \
    responseresult.h \
    respondthread.h

LIBS += -lpthread libwsock32 libws2_32

FORMS    += mainwindow.ui
