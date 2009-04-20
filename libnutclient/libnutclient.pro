
TEMPLATE = lib
CONFIG += static create_prl
CONFIG -= thread \
 release
TARGET = nutclient

CONFIG += qt warn_on qdbus exceptions \
 staticlib \
 debug
QT -= gui
QT += network

CODECFORSRC = UTF-8

HEADERS += client.h server_proxy.h client_exceptions.h \
 cinterface.h \
 cenvironment.h \
 cdevice.h \
 cdevicemanager.h \
 clibnut.h \
 clog.h
SOURCES += client.cpp server_proxy.cpp \
 cinterface.cpp \
 clibnut.cpp \
 clog.cpp \
 cdevicemanager.cpp \
 cdevice.cpp \
 cenvironment.cpp

OBJECTS_DIR = build/
UI_DIR = build/
MOC_DIR = build/

target.path = /usr/lib/
INSTALLS += target
DESTDIR = .

DEFINES += CONFIG_CTRL_IFACE \
 CONFIG_CTRL_IFACE_UNIX


INCLUDEPATH += ../

QMAKE_CXXFLAGS_DEBUG += -pedantic \
-Wno-long-long
LIBS += ../libnutwireless/libnutwireless.a \
  ../libnutcommon/libnutcommon.a

TARGETDEPS += ../libnutwireless/libnutwireless.a \
  ../libnutcommon/libnutcommon.a

