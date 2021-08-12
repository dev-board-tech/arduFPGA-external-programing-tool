QT += widgets serialport
QT -= gui

TARGET = ../../../ptool-win64/ardufpga-ptool

CONFIG += c++11 console
CONFIG -= app_bundle
CONFIG -= import_plugins
QMAKE_LFLAGS += -static-libgcc
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -Os -momit-leaf-frame-pointer
DEFINES += QT_STATIC_BUILD
CONFIG -= embed_manifest_exe

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050000    # disables all the APIs deprecated before Qt 5.0.0

SOURCES += \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


