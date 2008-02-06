
TEMPLATE = app
CONFIG += qt qdbus
CONFIG -= thread
QT -= gui
QT += network

CODECFORSRC = UTF-8
CODECFORTR = ISO-8859-1

OBJECTS_DIR = build/
MOC_DIR = build/

TARGET = cnut

HEADERS += main.h cnut_commands.h cnut_parsers.h cnut_types.h \
 server_proxy.h
SOURCES += main.cpp cnut_commands.cpp cnut_parsers.cpp  \
 server_proxy.cpp
DESTDIR = .

target.path = /usr/bin

INSTALLS += target 

INCLUDEPATH += ..

LIBS += ../libnutcommon/libnutcommon.a -L/lib
TARGETDEPS += ../libnutcommon/libnutcommon.a
