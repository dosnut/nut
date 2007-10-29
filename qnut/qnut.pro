
TEMPLATE = app
CONFIG += qt qdbus
QT += network

CODECFORSRC = UTF-8
CODECFORTR = ISO-8859-1

OBJECTS_DIR = build/
MOC_DIR = build/
UI_DIR = ui/


#CONFIG(debug, debug|release){
#    TARGET = qnut_debug
#}else {
#    TARGET = qnut
#}

FORMS = connman.ui \
 ipconf.ui \
 scrset.ui \
 devopt.ui \
 wrlset.ui \
 airset.ui \
 eapconf.ui \
 pskconf.ui \
 wepconf.ui \
 apconf.ui
TRANSLATIONS = qnut_de.ts

HEADERS += connectionmanager.h trayicon.h \
 constants.h \
 overviewmodel.h \
 deviceoptions.h \
 ipconfiguration.h \
 common.h \
 scriptsettings.h \
 interfacedetailsmodel.h \
 wirelesssettings.h \
 managedapmodel.h \
 environmenttreemodel.h \
 availableapmodel.h \
 accesspointconfig.h
SOURCES += main.cpp connectionmanager.cpp trayicon.cpp \
 overviewmodel.cpp \
 deviceoptions.cpp \
 ipconfiguration.cpp \
 common.cpp \
 scriptsettings.cpp \
 interfacedetailsmodel.cpp \
 wirelesssettings.cpp \
 managedapmodel.cpp \
 environmenttreemodel.cpp \
 availableapmodel.cpp \
 accesspointconfig.cpp
DESTDIR = .

iconstarget.path = /usr/share/qnut/icons
iconstarget.files = res/*.png
langtarget.path = /usr/share/qnut/lang
langtarget.files = qnut_*.ts
shortcuttarget.path = /usr/share/applications
shortcuttarget.files = qnut.desktop

#CONFIG(debug, debug|release){
#    INSTALLS += target
#}else {
#    INSTALLS += target iconstarget langtarget shortcuttarget
#}

#QMAKE_CXXFLAGS_RELEASE += -DQNUT_RELEASE


INCLUDEPATH += ..

LIBS += ../common/libnutcommon.a \
../libnut/libnut.a
TARGETDEPS += ../common/libnutcommon.a \
../libnut/libnut.a
FORMS -= wpaset.ui \
wrlset.ui \
 pskconf.ui \
 wepconf.ui \
 eapconf.ui

