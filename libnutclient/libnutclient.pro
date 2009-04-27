
TEMPLATE = lib
CONFIG += static create_prl
CONFIG -= thread
TARGET = nutclient

CONFIG += qt warn_on qdbus exceptions \
 staticlib
QT -= gui
QT += network

CODECFORSRC = UTF-8

HEADERS += client.h server_proxy.h cinterface.h \
 cenvironment.h \
 cdevice.h \
 cdevicemanager.h \
 clibnut.h \
 clog.h
SOURCES += server_proxy.cpp \
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

TARGETDEPS += ../libnutwireless/libnutwireless.a \
  ../libnutcommon/libnutcommon.a

LIBS += ../libnutcommon/libnutcommon.a \
  ../libnutwireless/libnutwireless.a

