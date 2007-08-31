
TEMPLATE = lib
CONFIG += staticlib create_prl
TARGET = nutcommon

CONFIG += qt warn_on qdbus exceptions
QT -= gui
QT += network

CODECFORSRC = UTF-8

SOURCES += macaddress.cpp types.cpp
HEADERS += macaddress.h   types.h \
 dbus.h \
 common.h

OBJECTS_DIR = build/
UI_DIR = build/
MOC_DIR = build/
