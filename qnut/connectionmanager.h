#ifndef QNUT_CONNECTIONMANAGER_H
#define QNUT_CONNECTIONMANAGER_H

#include <QtGui>
#include <libnutclient/client.h>
#include "ui/ui_connman.h"
#include "trayicon.h"
#include "devicedetails.h"

namespace qnut {
	class CConnectionManager : public QMainWindow {
		Q_OBJECT
	private:
		Ui::connMan ui;
		
		libnutclient::CDeviceManager mDeviceManager;
		libnutclient::CLog mLogFile;
		
		CDeviceDetailsHash mDeviceDetails;
		QSettings mStettings;
		
		CTrayIcon mTrayIcon;
		QTabWidget mTabWidget;
		QTreeView mOverView;
		QTextEdit mLogEdit;
		
		QAction * mRefreshDevicesAction;
		QAction * mClearLogAction;
		
		QAction * mEnableDeviceAction;
		QAction * mDisableDeviceAction;
		
		QAction * mDeviceSettingsAction;
		QAction * mWirelessSettingsAction;
		
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
		void addUiDevice(libnutclient::CDevice * device);
		void removeUiDevice(libnutclient::CDevice * device);
		void showMessage(QString title, QString message, QSystemTrayIcon * trayIcon = NULL);
		void showAbout();
		void handleDeviceStateChange(libnutcommon::DeviceState state);
		void showDeviceOptions(QWidget * widget);
	};
};

#endif
