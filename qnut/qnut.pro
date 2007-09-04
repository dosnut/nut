
TEMPLATE = app
CONFIG += qt qdbus
QT += network

CODECFORSRC = UTF-8
CODECFORTR = ISO-8859-1

OBJECTS_DIR = build/
MOC_DIR = build/
UI_DIR = ui/
RCC_DIR = build/
TARGET = qnut

FORMS = connman.ui \
 ipconf.ui
TRANSLATIONS = qnut_de.ts

HEADERS += connectionmanager.h trayicon.h \
 constants.h \
 overviewmodel.h \
 deviceoptionsmodel.h \
 deviceoptions.h \
 ipconfiguration.h \
 common.h
SOURCES += main.cpp connectionmanager.cpp trayicon.cpp \
 overviewmodel.cpp \
 deviceoptionsmodel.cpp \
 deviceoptions.cpp \
 ipconfiguration.cpp \
 common.cpp
DESTDIR = .



target.path = /usr/bin/
restarget.path = /usr/share/qnut
restarget.files = res/*.png
INSTALLS += target
INSTALLS += restarget

INCLUDEPATH += ..

LIBS += ../common/libnutcommon.a \
../libnut/libnut.a
TARGETDEPS += ../common/libnutcommon.a \
../libnut/libnut.a
