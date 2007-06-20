
TEMPLATE = app

CONFIG += debug qt warn_on dbus \
exceptions
QT += network

SOURCES += main.cpp device.cpp config.cpp \
hardware.cpp \
sighandler.cpp \
hardware_ext.c \
log.cpp
HEADERS += common.h device.h   config.h   hardware.h \
exception.h \
sighandler.h \
log.h
YACCSOURCES += configparser.y
LEXSOURCES += configparser.l

OBJECTS_DIR = build/
UI_DIR = build/
MOC_DIR = build/
LIBS += -lnl

