TEMPLATE = app
CONFIG += qt qdbus
CONFIG -= gui
QT += network

CODECFORSRC = UTF-8

OBJECTS_DIR = build
TARGET = build/nut

SOURCES += nut.cpp


#DISTFILES += ../libnut/libnut_server.xml

DESTDIR = .

INCLUDEPATH += ..

LIBS += ../common/libnutcommon.a \
-L../libnut \
-lnut
TARGETDEPS += ../common/libnutcommon.a \
../libnut/libnut.so
