TEMPLATE = lib
CONFIG += staticlib create_prl
CONFIG -= thread \
 release
TARGET = nutcommon

CONFIG += qt warn_on qdbus exceptions \
 debug
QT -= gui
QT += network

CODECFORSRC = UTF-8

SOURCES += macaddress.cpp config.cpp \
 common.cpp \
 device.cpp \
 dbusmonitor.cpp
HEADERS += macaddress.h   config.h   common.h config.h \
 device.h \
 dbusmonitor.h

OBJECTS_DIR = build/
UI_DIR = build/
MOC_DIR = build/
QMAKE_CXXFLAGS_DEBUG += -pedantic \
-Wno-long-long
