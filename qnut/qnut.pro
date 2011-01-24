TEMPLATE = app
CONFIG += qt \
	qdbus
CONFIG -= thread
QT += network
CODECFORSRC = UTF-8
CODECFORTR = ISO-8859-1
OBJECTS_DIR = build/
MOC_DIR = build/
UI_DIR = build/
TARGET = qnut
FORMS = \
	ui/devicedetails.ui \
	ui/ipconfiguration.ui \
	ui/devicesettings.ui \
	ui/wirelesssettings.ui \
	ui/accesspointconfig.ui \
	ui/adhocconfig.ui
TRANSLATIONS = \
	translations/qnut_de.ts \
	translations/qnut_pt_BR.ts
HEADERS += \
	constants.h \
	common.h \
	cconnectionmanager.h \
	cipconfiguration.h \
	cwirelesssettings.h \
	cabstractwifinetconfigdialog.h \
	caccesspointconfig.h \
	cadhocconfig.h \
	cdevicesettings.h \
	cuidevice.h \
	cnotificationmanager.h \
	modelview/cuidevicemodel.h \
	modelview/cenvironmenttreemodel.h \
	modelview/cinterfacedetailsmodel.h \
	modelview/cenvironmentdetailsmodel.h \
	modelview/cavailableapmodel.h \
	modelview/cmanagedapmodel.h \
	modelview/cdnslistmodel.h \
	modelview/cipeditdelegate.h \
	modelview/ccommandlistmodel.h \
	utils/cerrorcodeevaluator.h
SOURCES += main.cpp \
	common.cpp \
	cconnectionmanager.cpp \
	cipconfiguration.cpp \
	cwirelesssettings.cpp \
	cabstractwifinetconfigdialog.cpp \
	caccesspointconfig.cpp \
	cadhocconfig.cpp \
	cdevicesettings.cpp \
	cuidevice.cpp \
	cnotificationmanager.cpp \
	modelview/cuidevicemodel.cpp \
	modelview/cenvironmenttreemodel.cpp \
	modelview/cinterfacedetailsmodel.cpp \
	modelview/cenvironmentdetailsmodel.cpp \
	modelview/cavailableapmodel.cpp\
	modelview/cmanagedapmodel.cpp \
	modelview/cdnslistmodel.cpp \
	modelview/cipeditdelegate.cpp \
	modelview/ccommandlistmodel.cpp \
	utils/cerrorcodeevaluator.cpp
DESTDIR = .
target.path = /usr/bin
iconstarget.path = /usr/share/qnut/icons
iconstarget.files = res/*.png \
	res/qnut.svg \
	res/qnut_small.svg
langtarget.path = /usr/share/qnut/lang
langtarget.files = translations/qnut_*.qm
shortcuttarget.path = /usr/share/applications
shortcuttarget.files = qnut.desktop
INSTALLS += target \
	iconstarget \
	langtarget \
	shortcuttarget
INCLUDEPATH += ..
QMAKE_CXXFLAGS_DEBUG += -pedantic \
	-Wno-long-long
LIBS += -lnutclient \
	-L../libnutclient
