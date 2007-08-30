
TEMPLATE = lib
CONFIG += dll create_prl
TARGET = nut
VERSION = 0.1.0

CONFIG += qt warn_on qdbus exceptions
QT -= gui
QT += network

CODECFORSRC = UTF-8

HEADERS += libnut_cli.h libnut_server_proxy.h libnut_exceptions.h
SOURCES += libnut_cli.cpp libnut_server_proxy.cpp

OBJECTS_DIR = build/
UI_DIR = build/
MOC_DIR = build/

INCLUDEPATH += ../

LIBS += ../common/libnutcommon.a
TARGETDEPS += ../common/libnutcommon.a

libnut.path = /usr/lib/
libnut.files = libnut.so.$$VERSION
INSTALLS += libnut
