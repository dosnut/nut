
TEMPLATE = app
CONFIG += qt qdbus \
 debug_and_release \
 build_all
QT += network

CODECFORSRC = UTF-8
CODECFORTR = ISO-8859-1

OBJECTS_DIR = build/
MOC_DIR = build/
UI_DIR = ui/
RCC_DIR = build/

CONFIG += debug_and_release

CONFIG(debug, debug|release){
    TARGET = qnut_debug
}else {
    TARGET = qnut
}

FORMS = connman.ui \
 ipconf.ui \
 devconf.ui
TRANSLATIONS = qnut_de.ts

HEADERS += connectionmanager.h trayicon.h \
 constants.h \
 overviewmodel.h \
 deviceoptionsmodel.h \
 deviceoptions.h \
 ipconfiguration.h \
 common.h \
 deviceconfiguration.h
SOURCES += main.cpp connectionmanager.cpp trayicon.cpp \
 overviewmodel.cpp \
 deviceoptionsmodel.cpp \
 deviceoptions.cpp \
 ipconfiguration.cpp \
 common.cpp \
 deviceconfiguration.cpp
DESTDIR = .

target.path = /usr/bin/
iconstarget.path = /usr/share/qnut/icons
iconstarget.files = res/*.png
langtarget.path = /usr/share/qnut/lang
langtarget.files = qnut_*.ts
shortcuttarget.path = /usr/share/applications
shortcuttarget.files = qnut.desktop

CONFIG(debug, debug|release){
    INSTALLS += target
}else {
    INSTALLS += target iconstarget langtarget shortcuttarget
}

QMAKE_CXXFLAGS_RELEASE += -DQNUT_RELEASE

CONFIG -= debug

INCLUDEPATH += ..

LIBS += ../common/libnutcommon.a \
../libnut/libnut.a
TARGETDEPS += ../common/libnutcommon.a \
../libnut/libnut.a
