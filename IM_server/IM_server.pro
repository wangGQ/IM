#-------------------------------------------------
#
# Project created by QtCreator 2015-01-10T22:44:30
#
#-------------------------------------------------

QT       += core

QT       -= gui
QT       += network
QT       += sql

TARGET = IM_server
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += src/main.cpp \
    src/Server.cpp \
    src/CSocketThread.cpp \
    src/DbHelper.cpp \
    src/User.cpp \
    src/MsgProxy.cpp

HEADERS += \
    include/Server.h \
    include/CSocketThread.h \
    include/DbHelper.h \
    include/MacroDefine.h \
    include/User.h \
    include/MsgProxy.h

RESOURCES += \
    IM_server.qrc



