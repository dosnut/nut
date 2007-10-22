
TEMPLATE = lib
CONFIG += static create_prl
TARGET = nut
VERSION = 0.1.0

CONFIG += qt warn_on qdbus exceptions
QT -= gui
QT += network

CODECFORSRC = UTF-8

HEADERS += libnut_cli.h libnut_server_proxy.h libnut_exceptions.h \
 libnut_wpa_supplicant.h \
 wpa_ctrl.h \
 includes.h \
 build_config.h
SOURCES += libnut_cli.cpp libnut_server_proxy.cpp \
 libnut_wpa_supplicant.cpp \
 wpa_ctrl.c

OBJECTS_DIR = build/
UI_DIR = build/
MOC_DIR = build/

INCLUDEPATH += ../

LIBS += ../common/libnutcommon.a
TARGETDEPS += ../common/libnutcommon.a

target.path = /usr/lib/
INSTALLS += target
