TEMPLATE = app
INCLUDEPATH += .
CONFIG += qt

CODECFORSRC = UTF-8
CODECFORTR = ISO-8859-1

OBJECTS_DIR = build
DESTDIR = build

FORMS = connman.ui
TRANSLATIONS = qnut_de.ts

HEADERS += connman.h trayicon.h
SOURCES += main.cpp connman.cpp trayicon.cpp

#dbus zeugs
INCLUDEPATH += ../libnut
include(../libnut/libnut_qnut.pri)