TEMPLATE = app
CONFIG += qt qdbus
CONFIG -= gui
QT += network

CODECFORSRC = UTF-8

OBJECTS_DIR = build
TARGET = build/nut

SOURCES += nut.cpp \
 nut_library.cpp \
 nut_interactive.cpp \
 nut_commandline.cpp \

HEADERS += nut_library.h \
 nut_interactive.h \
 nut_commandline.h

#DISTFILES += ../libnut/libnut_server.xml

DESTDIR = .

INCLUDEPATH += ..

LIBS += ../common/libnutcommon.a \
-L../libnut \
-lnut
TARGETDEPS += ../common/libnutcommon.a \
../libnut/libnut.so
