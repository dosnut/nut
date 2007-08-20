TEMPLATE = app
INCLUDEPATH += .
CONFIG += qt
QT += network

CODECFORSRC = UTF-8
CODECFORTR = ISO-8859-1

OBJECTS_DIR = build
TARGET = build/qnut

FORMS = connman.ui \
 ipconf.ui
TRANSLATIONS = qnut_de.ts

HEADERS += connectionmanager.h trayicon.h \
 constants.h \
 overviewlistmodel.h \
 deviceoptionsmodel.h \
 deviceoptions.h \
 ipconfiguration.h \
 common.h
SOURCES += main.cpp connectionmanager.cpp trayicon.cpp \
 overviewlistmodel.cpp \
 deviceoptionsmodel.cpp \
 deviceoptions.cpp \
 ipconfiguration.cpp \
 common.cpp

#dbus zeugs
#INCLUDEPATH += ../libnut
include(../libnut/libnut_cli.pri)
