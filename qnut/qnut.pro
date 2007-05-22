TEMPLATE = app
INCLUDEPATH += .
CONFIG += qt

CODECFORSRC = UTF-8
CODECFORTR = ISO-8859-1

OBJECTS_DIR = build
TRANSLATIONS = qnut_de.ts
#RESOURCES = qnut.qrc

HEADERS += trayicon.h
SOURCES += main.cpp trayicon.cpp
