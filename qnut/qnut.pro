TEMPLATE = app
INCLUDEPATH += .
CONFIG += qt

CODECFORSRC = UTF-8
CODECFORTR = ISO-8859-1

OBJECTS_DIR = build

FORMS = connman.ui
TRANSLATIONS = qnut_de.ts
#RESOURCES = qnut.qrc

HEADERS += connman.h trayicon.h
SOURCES += main.cpp connman.cpp trayicon.cpp
