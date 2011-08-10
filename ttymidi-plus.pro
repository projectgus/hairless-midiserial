#-------------------------------------------------
#
# Project created by QtCreator 2011-08-08T17:57:42
#
#-------------------------------------------------

QT       += core gui

TARGET = ttymidi-plus
TEMPLATE = app

# Main Program

SOURCES += \
        main.cpp\
        mainwindow.cpp \
        src/RtMidi.cpp

HEADERS  += \
    src/RtMidi.h \
    src/RtError.h \
    mainwindow.h

FORMS    += mainwindow.ui

# qextserialport

HEADERS                 += qextserialport/qextserialport.h \
                          qextserialport/qextserialenumerator.h \
                          qextserialport/qextserialport_global.h
SOURCES                 += qextserialport/qextserialport.cpp

unix:SOURCES           += qextserialport/posix_qextserialport.cpp
unix:!macx:SOURCES     += qextserialport/qextserialenumerator_unix.cpp
macx {
  SOURCES          += qextserialport/qextserialenumerator_osx.cpp
  LIBS             += -framework IOKit -framework CoreFoundation
}

win32 {
  SOURCES          += qextserialport/win_qextserialport.cpp \
                      qextserialport/qextserialenumerator_win.cpp
  DEFINES          += WINVER=0x0501 # needed for mingw to pull in appropriate dbt business...probably a better way to do this
  LIBS             += -lsetupapi
}

# RtMidi TODO: make properly cross-platform

DEFINES += __LINUX_ALSASEQ__
CONFIG += link_pkgconfig x11
PKGCONFIG += alsa
LIBS += -lpthread

SUBDIRS = qextserialport
