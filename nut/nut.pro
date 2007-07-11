TEMPLATE = app
INCLUDEPATH += .
CONFIG += qt
QT += network

CODECFORSRC = UTF-8

OBJECTS_DIR = build
TARGET = build/nut

#HEADERS +=
SOURCES += nut.cpp

INCLUDEPATH += ../libnut
include(../libnut/libnut_cli.pri)

