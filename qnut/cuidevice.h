//
// C++ Interface: UI Device
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_CUIDEVICE_H
#define QNUT_CUIDEVICE_H

#include <QWidget>
#include <libnutcommon/device.h>

#include "common.h"
#include "ui_devicedetails.h"

namespace libnutclient {
	class CDevice;
	class CInterface;
}

class QSignalMapper;
class QSettings;

namespace qnut {
#ifndef QNUT_NO_WIRELESS
	class CWirelessSettings;
#endif
	class CNotificationManager;

	/**
	 * @brief CUIDevice interacts directly with CDevice.
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 *
	 * On creation, the CUIDevice sets up the user interface for a detailed view
	 * of the environments/interfaces tree.
	 *
	 * The main tasks of CUIDevice are:
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
	class CUIDevice : public QWidget {
		Q_OBJECT
		friend class CNotificationManager;
	public:
		/**
		 * @brief Creates the object, initializes the user interface and reads the settings to "~/.qnut/<device name>/dev.conf".
		 * @param parentDevice pointer to the device to be managed
		 * @param parent pointer to the parent widget
		 */
		CUIDevice(libnutclient::CDevice * parentDevice, QWidget * parent = 0);

		/// @brief Destroyes the object and saves all settings for this device.
		~CUIDevice();

		/// @brief Returns a pointer to the managed device.
		inline libnutclient::CDevice * device() const { return m_Device; }

		/// @brief Returns a pointer to the sub menu of the managed device.
		inline QMenu * deviceMenu() const { return m_DeviceMenu; }

		/// @brief Returns the list of Actions of the managed device.
		inline QList<QAction *> deviceActions() const { return m_DeviceActions; }
		/// @brief Returns the list of Actions in the context menu of the environments/interfaces tree.
		inline QList<QAction *> environmentTreeActions() const { return ui.environmentTree->actions(); }
		/**
		 * @brief Returns the list of commands for a given state
		 * @param state device state
		 */
		inline QList<ToggleableCommand> & commandList(int state) { return m_CommandList[state]; }

		static void init();
		static void cleanup();
		static const QSignalMapper * showRequestMapper() { return m_ShowRequestMapper; }
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
		 */
		void showMessageRequested(QString title, QString message);

		void showNotificationRequested(libnutcommon::DeviceState state);
		void updateTrayIconRequested(libnutcommon::DeviceState state);
#ifndef QNUT_NO_WIRELESS
		void wirelessInformationUpdated();
#endif
		void showTrayIconRequested(bool value);
	private:
		Ui::devdet ui;
#ifndef QNUT_NO_WIRELESS
		CWirelessSettings * m_WirelessSettings;
#endif

		static QSignalMapper * m_ShowRequestMapper;

		QMenu * m_DeviceMenu;
		QList<QAction *> m_DeviceActions;
		QAction * m_ShowEnvironmentsAction;
		QAction * m_EnterEnvironmentAction;
		QAction * m_IPConfigurationAction;

		bool m_ShowTrayIcon;
		bool m_CommandsEnabled;
		bool m_NotificationsEnabled;
		//TODO create device settings struct/class
		QList<ToggleableCommand> m_CommandList[5];

		QSet<libnutclient::CInterface *> m_IPConfigsToRemember;

		libnutclient::CDevice * m_Device;

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
	private slots:
		void handleSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
		void handleDeviceStateChange(libnutcommon::DeviceState state);
		void openIPConfiguration();
		void copySelectedProperty();
	};
}

#endif
