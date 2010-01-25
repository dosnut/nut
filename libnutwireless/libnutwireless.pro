TEMPLATE = lib
CONFIG += static create_prl
CONFIG -= thread
TARGET = nutwireless

CONFIG += qt warn_on qdbus exceptions \
 staticlib
QT -= gui
QT += network

CODECFORSRC = UTF-8

HEADERS += wpa_supplicant.h \
 types.h \
 wstypes.h \
 hwtypes.h \
 cwireless.h \
 cwirelesshw.h \
 cwexthw.h \
 wpa_ctrl/wpa_ctrl.h \
 wpa_ctrl/includes.h \
 wpa_ctrl/build_config.h \
 wpa_ctrl/common.h \
 wpa_ctrl/os.h \
 wpa_ctrl/wpa_debug.h \
 cnetworkconfig.h \
 conversion.h \
 cconfigparser.h
SOURCES += wpa_supplicant.cpp \
 cws_net.cpp \
 parsers.cpp \
 base.cpp \
 types.cpp \
 wstypes.cpp \
 cwireless.cpp \
 cwexthw.cpp \
 cwexthw_parse.cpp \
 wpa_ctrl/wpa_ctrl.c \
 wpa_ctrl/common.c \
 wpa_ctrl/wpa_debug.c \
 wpa_ctrl/os_unix.c \
 cnetworkconfig.cpp \
 conversion.cpp \
 cconfigparser.cpp

OBJECTS_DIR = build/
UI_DIR = build/
MOC_DIR = build/

target.path = /usr/lib/
INSTALLS += target
DESTDIR = .

DEFINES += CONFIG_CTRL_IFACE \
 CONFIG_CTRL_IFACE_UNIX

INCLUDEPATH += ..

LIBS += ../libnutcommon/libnutcommon.a

TARGETDEPS += ../libnutcommon/libnutcommon.a

LEXSOURCES += configparser.l

YACCSOURCES += configparser.y

