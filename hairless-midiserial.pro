#-------------------------------------------------
#
# Project created by QtCreator 2011-08-08T17:57:42
#
#-------------------------------------------------

QT       += core gui

TARGET = hairless-midiserial
TEMPLATE = app

DEFINES +=APPNAME=\"hairless-midiserial\"

# Main Program

SOURCES += \
        main.cpp\
        mainwindow.cpp \
        src/RtMidi.cpp \
    src/QRtMidiIn.cpp \
    src/Bridge.cpp

HEADERS  += \
    src/RtMidi.h \
    src/RtError.h \
    mainwindow.h \
    src/QRtMidiIn.h \
    src/Bridge.h \
    src/fixlatency.h

FORMS    += mainwindow.ui

# libftdi
#LIBS += -lftdi

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
  QMAKE_LFLAGS += -static -static-libgcc
  SOURCES          += qextserialport/win_qextserialport.cpp \
                      qextserialport/qextserialenumerator_win.cpp
  DEFINES          += WINVER=0x0501 # needed for mingw to pull in appropriate dbt business...probably a better way to do this
  LIBS             += -lsetupapi
}

# RtMidi

unix:!macx { # linux doesn't get picked up, not sure what else to use
  DEFINES += __LINUX_ALSASEQ__
  CONFIG += link_pkgconfig x11
  PKGCONFIG += alsa
  LIBS += -lpthread
}

win32 {
  DEFINES += __WINDOWS_MM__
  LIBS += -lwinmm
}


# latency fixes

linux-g++ {
   SOURCES += src/fixlatency_linux.cpp
}
win32 {
   SOURCES += src/fixlatency_win32.cpp
}


# Resources

RESOURCES += \
    resources.qrc
