#-------------------------------------------------
#
# Project created by QtCreator 2011-08-08T17:57:42
#
#-------------------------------------------------

QT       += core gui

TARGET = hairless-midiserial
TEMPLATE = app

DEFINES +=APPNAME=\"hairless-midiserial\"

DEFINES += VERSION=\\\"0.4\\\"

# Main Program

SOURCES += \
    src/main.cpp \
    src/Bridge.cpp \
    src/BlinkenLight.cpp \
    src/ui/mainwindow.cpp \
    src/ui/aboutdialog.cpp \
    src/ui/settingsdialog.cpp

HEADERS  += \
    src/Bridge.h \
    src/Settings.h \
    src/BlinkenLight.h \
    src/ui/mainwindow.h \
    src/ui/settingsdialog.h \
    src/ui/aboutdialog.h

FORMS += src/ui/mainwindow.ui \
    src/ui/settingsdialog.ui \
    src/ui/aboutdialog.ui

INCLUDEPATH += src/

# Universal binary for OS X

macx {
    CONFIG += ppc x86
}


# Static flags for windows

win32 {
  QMAKE_LFLAGS += -static -static-libgcc
}

# Icon stuff

win32 {
  RC_FILE = "images/icon.rc"
}
macx {
  ICON = "images/icon.icns"
}
# linux has no baked-in icon support, but we load a resource to use as a window icon

# qextserialport

HEADERS                 += libraries/qextserialport/src/qextserialport.h \
                          libraries/qextserialport/src/qextserialenumerator.h \
                          libraries/qextserialport/src/qextserialport_global.h
SOURCES                 += libraries/qextserialport/src/qextserialport.cpp \
                           libraries/qextserialport/src/qextserialenumerator.cpp

unix:SOURCES           += libraries/qextserialport/src/qextserialport_unix.cpp
unix:!macx:SOURCES     += libraries/qextserialport/src/qextserialenumerator_linux.cpp
unix:!macx:LIBS        += -ludev

macx {
  SOURCES          += libraries/qextserialport/src/qextserialenumerator_osx.cpp
  LIBS             += -framework IOKit -framework CoreFoundation
}

win32 {
  SOURCES          += libraries/qextserialport/src/win_qextserialport.cpp \
                      libraries/qextserialport/src/qextserialenumerator_win.cpp
  DEFINES          += WINVER=0x0501 # needed for mingw to pull in appropriate dbt business...probably a better way to do this
  LIBS             += -lsetupapi
}

INCLUDEPATH += libraries/qextserialport/src/

# RtMidi

HEADERS +=    libraries/rtmidi/RtMidi.h \
              src/QRtMidiIn.h \

SOURCES +=    libraries/rtmidi/RtMidi.cpp \
              src/QRtMidiIn.cpp

INCLUDEPATH += libraries/rtmidi/

linux-* { # linux doesn't get picked up, not sure what else to use
  DEFINES += __LINUX_ALSASEQ__
  CONFIG += link_pkgconfig x11
  PKGCONFIG += alsa
  LIBS += -lpthread
}
win32 {
  DEFINES += __WINDOWS_MM__
  LIBS += -lwinmm
}
macx {
    DEFINES += __MACOSX_CORE__
    LIBS += -framework \
        CoreMidi \
        -framework \
        CoreAudio \
        -framework \
        CoreFoundation
}

# PortLatency
SOURCES += src/PortLatency.cpp
HEADERS += src/PortLatency.h
linux-* {
   SOURCES += src/PortLatency_linux.cpp
}
win32 {
   SOURCES += src/PortLatency_win32.cpp
}
macx {
   SOURCES += src/PortLatency_osx.cpp
}

# Resources

RESOURCES += \
    resources.qrc
