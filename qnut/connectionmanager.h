//
// C++ Interface: connectionmanager
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_CONNECTIONMANAGER_H
#define QNUT_CONNECTIONMANAGER_H

#include <QMainWindow>
#include <QItemSelection>
#include <QHash>
#include <libnutclient/cdevicemanager.h>
#include <libnutclient/clog.h>

class QTabWidget;
class QTextEdit;
class QTreeView;
class QSystemTrayIcon;

namespace qnut {
	class CUIDeviceModel;
	class CNotificationManager;
	
	/**
	 * @brief CConnectionManager acts as the main class (and window) of the application
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 * 
	 * The connection manager itself provides only constructor and destructor.
	 * 
	 * It's the main user interface of QNUT with the following tasks:
	 *  - interact with the libnutclient::CDeviceManager
	 *  - provide an overview of all managed devices and an application log (written to file: "~/.qnut/qnut.log")
	 *  - delegate detailed views and settings of each device, environment and interface
	 */
	class CConnectionManager : public QMainWindow {
		Q_OBJECT
	public:
		/**
		 * @brief Creates the object, initializes the basic user interface and reads settings from "~/.qnut/qnut.conf".
		 * @param parent parent widget
		 */
		CConnectionManager(QWidget * parent = 0);
		
		/// @brief Destroyes the object and writes the settings to "~/.qnut/qnut.conf".
		~CConnectionManager();
		
		static CNotificationManager * notificationManager();
	private:
		libnutclient::CDeviceManager * m_DeviceManager;
		libnutclient::CLog m_LogFile;
		
		CNotificationManager * m_NotificationManager;
		CUIDeviceModel * m_UIDeviceModel;
		
		QTabWidget * m_TabWidget;
		QTreeView * m_OverView;
		QTextEdit * m_LogEdit;
		
		QMenu * m_EditMenu;
		QToolBar * m_ToolBar;
		
		QAction * m_ShowLogAction;
		QAction * m_ShowBalloonTipsAction;
		
		QAction * m_RefreshDevicesAction;
		QAction * m_ClearLogAction;
		
		QAction * m_EnableDeviceAction;
		QAction * m_DisableDeviceAction;
		
		QAction * m_DeviceSettingsAction;
#ifndef QNUT_NO_WIRELESS
		QAction * m_WirelessSettingsAction;
#endif
		
		inline void createActions();
		inline void distributeActions(int mode = 0);
		
		inline void readSettings();
		inline void writeSettings();
	private slots:
		void handleTabChanged(int index);
		void handleSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
		void showLog(bool doShow);
		void updateTrayIconInfo();
		void addUiDevice(libnutclient::CDevice * device);
		void removeUiDevice(libnutclient::CDevice * device);
		void handleDeviceStateChange(libnutcommon::DeviceState state);
		void showDeviceDetails(QWidget * widget);
		void showDeviceDetails(const QModelIndex & index);
		void showAbout();
	};
}

#endif
