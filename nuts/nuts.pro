
TEMPLATE = app

CONFIG += qt warn_on qdbus exceptions
QT -= gui
QT += network

CODECFORSRC = UTF-8

SOURCES += main.cpp device.cpp config.cpp hardware.cpp hardware_ext.c
HEADERS += common.h device.h   config.h   hardware.h

SOURCES += sighandler.cpp log.cpp dhcppacket.cpp \
 dbus.cpp
HEADERS += sighandler.h   log.h   dhcppacket.h   exception.h \
 dbus.h

YACCSOURCES += configparser.y
LEXSOURCES += configparser.l

OBJECTS_DIR = build/
UI_DIR = build/
MOC_DIR = build/
DESTDIR = .

INCLUDEPATH += ../

LIBS +=  -lnl ../common/libnutcommon.a
TARGETDEPS += ../common/libnutcommon.a

TARGET = nuts
target.path = /usr/sbin/
INSTALLS += target
