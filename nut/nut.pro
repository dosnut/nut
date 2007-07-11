TEMPLATE = app
INCLUDEPATH += .
CONFIG += qt qdbus console
CONFIG -= gui
QT += network

CODECFORSRC = UTF-8

OBJECTS_DIR = build
TARGET = build/nut

#HEADERS +=

INCLUDEPATH += ../libnut
include(../libnut/libnut_cli.pri)

SOURCES += nut.cpp

HEADERS += servertest.h

