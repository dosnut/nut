
TEMPLATE = app
CONFIG += qt qdbus
QT += network

CODECFORSRC = UTF-8
CODECFORTR = ISO-8859-1

OBJECTS_DIR = build/
MOC_DIR = build/

TARGET = cnut

HEADERS += main.h nut_cmd_commands.h nut_cmd_parsers.h nut_cmd_types.h \
 server_proxy.h
SOURCES += main.cpp nut_cmd_commands.cpp nut_cmd_parsers.cpp  \
 server_proxy.cpp
DESTDIR = .

target.path = /usr/bin

INSTALLS += target 

INCLUDEPATH += ..

LIBS += ../libnutcommon/libnutcommon.a -L/lib
TARGETDEPS += ../libnutcommon/libnutcommon.a
