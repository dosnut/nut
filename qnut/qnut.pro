
TEMPLATE = app
CONFIG += qt qdbus
QT += network

CODECFORSRC = UTF-8
CODECFORTR = ISO-8859-1

OBJECTS_DIR = build/
MOC_DIR = build/
UI_DIR = build/
RCC_DIR = build/
TARGET = qnut

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
RESOURCES += res/qnut.qrc
DESTDIR = .

INCLUDEPATH += ..

LIBS += ../common/libnutcommon.a \
-L../libnut \
-lnut
TARGETDEPS += ../common/libnutcommon.a \
../libnut/libnut.so

target.path = /usr/bin/
INSTALLS += target
