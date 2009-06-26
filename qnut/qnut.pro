
TEMPLATE = app
CONFIG += qt qdbus
CONFIG -= thread
QT += network

CODECFORSRC = UTF-8
CODECFORTR = ISO-8859-1

OBJECTS_DIR = build/
MOC_DIR = build/
UI_DIR = ui/

TARGET = qnut

FORMS = ipconf.ui \
 scrset.ui \
 airset.ui \
 apconf.ui \
 devdet.ui \
 adhoc.ui

TRANSLATIONS = qnut_de.ts qnut_pt_BR.ts

HEADERS += connectionmanager.h trayicon.h \
 constants.h \
 overviewmodel.h \
 devicedetails.h \
 ipconfiguration.h \
 common.h \
 scriptsettings.h \
 interfacedetailsmodel.h \
 wirelesssettings.h \
 managedapmodel.h \
 environmenttreemodel.h \
 availableapmodel.h \
 accesspointconfig.h \
 environmentdetailsmodel.h \
 dnslistmodel.h \
 ipeditdelegate.h \
 adhocconfig.h
SOURCES += main.cpp connectionmanager.cpp trayicon.cpp \
 overviewmodel.cpp \
 devicedetails.cpp \
 ipconfiguration.cpp \
 common.cpp \
 scriptsettings.cpp \
 interfacedetailsmodel.cpp \
 wirelesssettings.cpp \
 managedapmodel.cpp \
 environmenttreemodel.cpp \
 availableapmodel.cpp \
 accesspointconfig.cpp \
 environmentdetailsmodel.cpp \
 dnslistmodel.cpp \
 ipeditdelegate.cpp \
 adhocconfig.cpp
DESTDIR = .

target.path = /usr/bin
iconstarget.path = /usr/share/qnut/icons
iconstarget.files = res/*.png res/qnut.svg res/qnut_small.svg
langtarget.path = /usr/share/qnut/lang
langtarget.files = qnut_*.qm
shortcuttarget.path = /usr/share/applications
shortcuttarget.files = qnut.desktop

INSTALLS += target iconstarget langtarget shortcuttarget

INCLUDEPATH += ..


QMAKE_CXXFLAGS_DEBUG += -pedantic -Wno-long-long
LIBS += ../libnutclient/libnutclient.a ../libnutwireless/libnutwireless.a ../libnutcommon/libnutcommon.a
LIBS += -liw
TARGETDEPS += ../libnutcommon/libnutcommon.a ../libnutwireless/libnutwireless.a ../libnutclient/libnutclient.a

