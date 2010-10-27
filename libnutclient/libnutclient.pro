VERSION = 0.5.5
TEMPLATE = lib
CONFIG += create_prl
CONFIG -= thread
TARGET = nutclient

CONFIG += qt warn_on qdbus exceptions dll
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
INSTALLS += target maindev commondev wirelessdev
DESTDIR = .

DEFINES += CONFIG_CTRL_IFACE \
 CONFIG_CTRL_IFACE_UNIX

maindev.files = *.h
maindev.path = /usr/include/libnutclient

commondev.files = ../libnutcommon/*.h
commondev.path = /usr/include/libnutcommon

wirelessdev.files = ../libnutwireless/*.h
wirelessdev.path = /usr/include/libnutwireless

INCLUDEPATH += ..

QMAKE_CXXFLAGS_DEBUG += -pedantic \
-Wno-long-long

TARGETDEPS += ../libnutwireless/libnutwireless.a \
  ../libnutcommon/libnutcommon.a

LIBS += ../libnutcommon/libnutcommon.a \
  ../libnutwireless/libnutwireless.a \
  -liw

