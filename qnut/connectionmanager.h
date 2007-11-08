#ifndef QNUT_CONNECTIONMANAGER_H
#define QNUT_CONNECTIONMANAGER_H

#include <QtGui>
#include <QHash>
#include <libnut/libnut_cli.h>
#include "ui/ui_connman.h"
#include "trayicon.h"
#include "deviceoptions.h"

namespace qnut {
	class CConnectionManager : public QMainWindow {
		Q_OBJECT
	private:
		Ui::connMan ui;
		
		libnut::CDeviceManager deviceManager;
		libnut::CLog logFile;
		
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
	private slots:
		void handleTabChanged(int index);
		void handleSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
		void showLog(bool doShow);
	public slots:
		void updateTrayIconInfo();
		void addUiDevice(libnut::CDevice * device);
		void removeUiDevice(libnut::CDevice * device);
		void showMessage(QString title, QString message, QSystemTrayIcon * trayIcon = NULL);
		void showAbout();
		void handleDeviceStateChange(libnut::DeviceState state);
		void showDeviceOptions(QWidget * widget);
	};
};

#endif
