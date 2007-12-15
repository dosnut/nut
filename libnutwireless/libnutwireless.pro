TEMPLATE = lib
CONFIG += static create_prl
TARGET = nutwireless

CONFIG += qt warn_on qdbus exceptions \
 staticlib
QT -= gui
QT += network

CODECFORSRC = UTF-8

HEADERS += wpa_supplicant.h \
 parsers.h \
 base.h \
 types.h \
 wpa_ctrl/wpa_ctrl.h \
 wpa_ctrl/includes.h \
 wpa_ctrl/build_config.h \
 wpa_ctrl/common.h \
 wpa_ctrl/os.h \
 wireless/iwlib.h \
 wpa_ctrl/wpa_debug.h \
 wireless/wireless.h
SOURCES += wpa_supplicant.cpp \
 parsers.cpp \
 base.cpp \
 types.cpp \
 wpa_ctrl/wpa_ctrl.c \
 wpa_ctrl/common.c \
 wireless/iwlib.c \
 wpa_ctrl/wpa_debug.c \
 wpa_ctrl/os_unix.c

OBJECTS_DIR = build/
UI_DIR = build/
MOC_DIR = build/

target.path = /usr/lib/
INSTALLS += target
DESTDIR = .

DEFINES += CONFIG_CTRL_IFACE \
 CONFIG_CTRL_IFACE_UNIX



INCLUDEPATH += ../

LIBS += ../libnutcommon/libnutcommon.a

TARGETDEPS += ../libnutcommon/libnutcommon.a

