TEMPLATE = app
INCLUDEPATH += .
CONFIG += qt
QT += network

CODECFORSRC = UTF-8
CODECFORTR = ISO-8859-1

OBJECTS_DIR = build
TARGET = build/qnut

FORMS = connman.ui
TRANSLATIONS = qnut_de.ts

HEADERS += connectionmanager.h trayicon.h \
 constants.h \
 overviewlistmodel.h \
 deviceoptionsmodel.h
SOURCES += main.cpp connectionmanager.cpp trayicon.cpp \
 overviewlistmodel.cpp \
 deviceoptionsmodel.cpp

#dbus zeugs
INCLUDEPATH += ../libnut
include(../libnut/libnut_cli.pri)