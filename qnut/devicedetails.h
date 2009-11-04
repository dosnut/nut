//
// C++ Interface: deviceoptions
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_DEVICEDETAILS_H
#define QNUT_DEVICEDETAILS_H

#include <QWidget>
#include <QSystemTrayIcon>
#include <libnutcommon/device.h>
#include "ui/ui_devdet.h"
#include "common.h"

namespace libnutclient {
	class CDevice;
	class CInterface;
}

class QSettings;

namespace qnut {
#ifndef QNUT_NO_WIRELESS
	class CWirelessSettings;
#endif
	
	/**
	 * @brief CDeviceDetails interacts directly with CDevice.
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 * 
	 * On creation, the CDeviceDetails sets up the user interface for a detailed view
	 * of the environments/interfaces tree.
	 * 
	 * The main tasks of CDeviceDetails are:
	 *  - interact directly with CDevice
	 *  - provide detailed information for environments and interfaces
	 *  - delegate scripting/wireless settings and user specifiable interface configuration
	 *  - execute scripts for each status change of the given device (if enabled in scripting settings)
	 *  - provide a tray icon for the given device witch reflects the current status (tool tip and pop-up messages)
	 *  - save the configurations made for the given device (in "~/.qnut/<device name>/dev.conf")
	 * 
	 * The devices details provides functions to open the windows for scritping and
	 * wireless settings (if the device has wireless extensions).
	 */
	class CDeviceDetails : public QWidget {
		Q_OBJECT
	private:
		Ui::devdet ui;
		quint8 m_ScriptFlags;
		bool m_CommandsEnabled;
		bool m_NotificationsEnabled;
		
		libnutclient::CDevice * m_Device;
		
#ifndef QNUT_NO_WIRELESS
		CWirelessSettings * m_WirelessSettings;
#endif
		QList<ToggleableCommand> m_CommandList[5];
		
		QMenu * m_DeviceMenu;
		QList<QAction *> m_DeviceActions;
		
		QAction * m_EnterEnvironmentAction;
		QAction * m_IPConfigurationAction;
		
		// TODO rename to m_TrayIcon
		QSystemTrayIcon * m_trayIcon;
		
		QSet<libnutclient::CInterface *> m_IPConfigsToRemember;
		
		inline void readCommands(QSettings * settings);
		inline void readIPConfigs(QSettings * settings);

		inline void writeCommands(QSettings * settings);
		inline void writeIPConfigs(QSettings * settings);
		
		inline void readSettings();
		inline void writeSettings();
		inline void createActions();
		inline void createView();
		inline void setHeadInfo();
		
		inline void executeCommand(QStringList & env, QString path);
		
		inline void showNotification(libnutcommon::DeviceState state);
	public:
		/// @brief Returns the flags for script execution.
		inline quint8 scriptFlags() const { return m_ScriptFlags; }
//		/// @brief Sets the flags for script execution.
//		inline void setScriptFlags(quint8 value) { m_ScriptFlags = value; }
		
		/// @brief Returns a pointer to the managed device.
		inline libnutclient::CDevice * device() const { return m_Device; }
		
		// TODO rename to deviceMenu
		/// @brief Returns a pointer to the sub menu of the managed device.
		inline QMenu * trayMenu() const { return m_DeviceMenu; }
		
		/// @brief Returns the list of Actions of the managed device.
		inline QList<QAction *> deviceActions() const { return m_DeviceActions; }
		/// @brief Returns the list of Actions in the context menu of the environments/interfaces tree.
		inline QList<QAction *> environmentTreeActions() const { return ui.environmentTree->actions(); }
		
		inline QList<ToggleableCommand> & commandList(int state) { return m_CommandList[state]; }
		
		inline QSystemTrayIcon * trayIcon() { return m_trayIcon; }
		
//		/// @brief Returns the path of the command for given state and index
//		inline QString & commandAt(DeviceState state, int index) { return m_CommandList[state][index]->path; }
//		/// @brief Returns the enabled state of the command for given state and index
//		inline bool commandAtIsEnabled(DeviceState state, int index) { return m_CommandList[state][index]->enabled; }
//		/// @brief Returns how many commands are present for a given state
//		inline int commandCount(DeviceState state) { return m_CommandList[state].size(); }
		
		/**
		 * @brief Creates the object, initializes the user interface and reads the settings to "~/.qnut/<device name>/dev.conf".
		 * @param parentDevice pointer to the device to be managed
		 * @param parent pointer to the parent widget
		 */
		CDeviceDetails(libnutclient::CDevice * parentDevice, QWidget * parent = 0);
		
		/// @brief Destroyes the object and writes the settings to "~/.qnut/<device name>/dev.conf".
		~CDeviceDetails();
	private slots:
		void handleTrayActivated(QSystemTrayIcon::ActivationReason);
		void handleSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
		void handleDeviceStateChange(libnutcommon::DeviceState state);
		void showTheeseDetails();
		void openIPConfiguration();
		void copySelectedProperty();
	public slots:
		/// @brief Opens the device settings dialog.
		void openDeviceSettings();
#ifndef QNUT_NO_WIRELESS
		/// @brief Opens ths wireless settings window.
		void openWirelessSettings();
#endif
	signals:
		/**
		 * @brief Emitted when showing a pop-up message is requested.
		 * @param title title for the requested message
		 * @param message the message itself
		 * @param trayIcon pointer to the tray icon for showing the message (NULL means the default tray icon)
		 */
		void showMessageRequested(QString title, QString message, QSystemTrayIcon * trayIcon = NULL);
		/**
		 * @brief Emitted when showing this widget is requested.
		 * @param widget pointer to this instance
		 */
		void showDetailsRequested(QWidget * widget);
	};
}

#endif
