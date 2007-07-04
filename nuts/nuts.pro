
TEMPLATE = app

CONFIG += debug qt warn_on dbus exceptions
QT -= gui
QT += network

SOURCES += main.cpp device.cpp config.cpp hardware.cpp hardware_ext.c
HEADERS += common.h device.h   config.h   hardware.h

SOURCES += sighandler.cpp log.cpp dhcppacket.cpp
HEADERS += sighandler.h   log.h   dhcppacket.h   exception.h

YACCSOURCES += configparser.y
LEXSOURCES += configparser.l

OBJECTS_DIR = build/
UI_DIR = build/
MOC_DIR = build/
LIBS += -lnl

