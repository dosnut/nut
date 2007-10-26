
TEMPLATE = lib
CONFIG += static create_prl
TARGET = nut

CONFIG += qt warn_on qdbus exceptions \
 staticlib
QT -= gui
QT += network

CODECFORSRC = UTF-8

HEADERS += libnut_cli.h libnut_server_proxy.h libnut_exceptions.h \
 libnut_wpa_supplicant.h \
 wpa_ctrl.h \
 includes.h \
 build_config.h \
 common.h \
 os.h \
 libnut_wpa_supplicant_types.h
SOURCES += libnut_cli.cpp libnut_server_proxy.cpp \
 libnut_wpa_supplicant.cpp \
 wpa_ctrl.c \
 common.c \
 libnut_wpa_supplicant_types.cpp

OBJECTS_DIR = build/
UI_DIR = build/
MOC_DIR = build/



target.path = /usr/lib/
INSTALLS += target
DESTDIR = .

DEFINES += CONFIG_CTRL_IFACE \
 CONFIG_CTRL_IFACE_UNIX




INCLUDEPATH += ../

LIBS += ../common/libnutcommon.a

TARGETDEPS += ../common/libnutcommon.a

