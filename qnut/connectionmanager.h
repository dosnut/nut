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
		QSettings settings;
		
		CTrayIcon trayicon;
		QTabWidget tabWidget;
		QTreeView overView;
		QTextEdit logEdit;
		
		QAction * refreshDevicesAction;
		QAction * clearLogAction;
		
		QAction * enableDeviceAction;
		QAction * disableDeviceAction;
		
		QAction * deviceSettingsAction;
		QAction * wirelessSettingsAction;
		
		inline void createActions();
		inline void distributeActions(int mode = 0);
		
		inline void readSettings();
		inline void writeSettings();
	public:
		CConnectionManager(QWidget * parent = 0);
		~CConnectionManager();
	public slots:
		void updateTrayIconInfo();
		void addUiDevice(CDevice * dev);
		void removeUiDevice(CDevice * dev);
		void handleTabChanged(int index);
		void handleSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
		void showMessage(QString title, QString message, QSystemTrayIcon * trayIcon = NULL);
		void showAbout();
		void handleDeviceStateChange(DeviceState state);
		void showLog(bool doShow);
		void showDeviceOptions(QWidget * widget);
	};
};

#endif
