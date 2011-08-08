#-------------------------------------------------
#
# Project created by QtCreator 2011-08-08T17:57:42
#
#-------------------------------------------------

QT       += core gui

TARGET = ttymidi-plus
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    src/RtMidi.cpp

HEADERS  += mainwindow.h \
    src/RtMidi.h \
    src/RtError.h

FORMS    += mainwindow.ui
