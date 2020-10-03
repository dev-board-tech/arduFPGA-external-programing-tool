QT += widgets serialport
QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle
CONFIG -= import_plugins
QMAKE_LFLAGS += -static -static-libgcc
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -Os -momit-leaf-frame-pointer
DEFINES += QT_STATIC_BUILD
CONFIG -= embed_manifest_exe

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


