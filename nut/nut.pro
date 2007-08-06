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

SOURCES += nut.cpp \
 ../libnut/libnut_types.cpp \
 nut_library.cpp \
 nut_interactive.cpp \
 nut_commandline.cpp \
 ../libnut/libnut_server.cpp

HEADERS += servertest.h \
 ../libnut/libnut_types.h \
 nut_library.h \
 nut_interactive.h \
 nut_commandline.h \
 ../libnut/libnut_server.h \
 ../libnut/libnut_server_proxy.h

DISTFILES += ../libnut/libnut_server.xml

