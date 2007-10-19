#ifndef QNUT_CONNECTIONMANAGER_H
#define QNUT_CONNECTIONMANAGER_H

#include <QtGui>
#include <QHash>
#include <libnut/libnut_cli.h>
#include "ui/ui_connman.h"
#include "trayicon.h"
#include "overviewmodel.h"
#include "deviceoptions.h"

namespace qnut {
	using namespace libnut;
	
	class CConnectionManager : public QMainWindow {
		Q_OBJECT
	private:
		Ui::connMan ui;
		
		CDeviceManager deviceManager;
		CLog logFile;
		CDeviceOptionsHash deviceOptions;
		
		CTrayIcon trayicon;
		
		QAction * refreshDevicesAction;
		QAction * enableDeviceAction;
		QAction * disableDeviceAction;
		
		QAction * deviceSettingsAction;
		QAction * ipConfigurationAction;
		
		QTabWidget tabWidget;
		QTreeView overView;
		QTextEdit logEdit;
		
		QSettings settings;
		
		bool showBalloonTips;
		
		inline void createActions();
		void distributeActions(int mode = 0);
		
		inline void readSettings();
		inline void writeSettings();
	public:
		CConnectionManager(QWidget * parent = 0);
		~CConnectionManager();
		
	public slots:
		void uiUpdateTrayIconInfo();
		void uiAddedDevice(CDevice * dev);
		void uiRemovedDevice(CDevice * dev);
		void uiCurrentTabChanged(int index);
		void uiSelectedDeviceChanged(const QItemSelection & selected, const QItemSelection & deselected);
		void uiShowMessage(QSystemTrayIcon * trayIcon, QString title, QString message);
		void uiShowAbout();
		void uiHandleDeviceStateChanged(DeviceState state);
		void uiHandleShowLogToggle(bool state);
		void uiShowOptions(QWidget * widget);
	};
};

#endif
