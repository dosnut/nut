#ifndef QNUT_CONNECTIONMANAGER_H
#define QNUT_CONNECTIONMANAGER_H

#include <QtGui>
#include <libnutclient/client.h>
#include "ui/ui_connman.h"
#include "trayicon.h"
#include "devicedetails.h"

namespace qnut {
	/** @brief CConnectionManager acts as the main class (and window) of the application
		The connection manager itself provides only the constructor and destructor.
		It's the main user interface of QNUT, that interacts with the libnutclient::CDeviceManager.
	*/
	class CConnectionManager : public QMainWindow {
		Q_OBJECT
	protected:
		Ui::connMan ui;
		
		libnutclient::CDeviceManager m_DeviceManager;
		libnutclient::CLog m_LogFile;
		
		CDeviceDetailsHash m_DeviceDetails;
		QSettings m_Stettings;
		
		CTrayIcon m_TrayIcon;
		QTabWidget m_TabWidget;
		QTreeView m_OverView;
		QTextEdit m_LogEdit;
		
		QAction * m_RefreshDevicesAction;
		QAction * m_ClearLogAction;
		
		QAction * m_EnableDeviceAction;
		QAction * m_DisableDeviceAction;
		
		QAction * m_DeviceSettingsAction;
		QAction * m_WirelessSettingsAction;
		
		inline void createActions();
		inline void distributeActions(int mode = 0);
		
		inline void readSettings();
		inline void writeSettings();
	public:
		CConnectionManager(QWidget * parent = 0);
		~CConnectionManager();
	protected slots:
		void handleTabChanged(int index);
		void handleSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
		void showLog(bool doShow);
		void updateTrayIconInfo();
		void showMessage(QString title, QString message, QSystemTrayIcon * trayIcon = NULL);
		void addUiDevice(libnutclient::CDevice * device);
		void removeUiDevice(libnutclient::CDevice * device);
		void handleDeviceStateChange(libnutcommon::DeviceState state);
		void showDeviceOptions(QWidget * widget);
		void showAbout();
	};
};

#endif
