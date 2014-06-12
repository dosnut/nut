//
// C++ Interface: CDeviceSettings
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_CDEVICESETTINGS_H
#define QNUT_CDEVICESETTINGS_H

#include <QDialog>

#include "common.h"
#include "ui_devicesettings.h"

namespace qnut {
	class CDeviceDetails;

	/**
	 * @brief CDeviceSettings provides a dialog to configure device specific settings like notfications, scripts to run, etc..
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 *
	 * On creation, the CDeviceSettings sets up its user interface.
	 * Settings so far implemented:
	 * - show Icon in notificaion area
	 * - disable notifications for this device
	 * - add/remove and enable/disable commands to be executed when the device changes its state
	 * It provides a public function to open the dialog for an existing instance of CDeviceDetails.
	 */
	class CDeviceSettings : public QDialog {
		Q_OBJECT
	public:
		/// @brief returns the edited command lists
		inline QList<ToggleableCommand> * commandListsResult() { return m_CommandLists; }
		/// @brief returns the resulting enabled setting for commands in general
		inline bool commandsEnabledResult() const { return ui.scriptBox->isChecked(); }
		/// @brief returns the resulting visibility setting for the tray icon
		inline bool trayIconVisibleResult() const { return ui.trayiconCheckBox->isChecked(); }
		/// @brief returns the resulting enabled state for notifications
		inline bool notificationEnabledResult() const { return !ui.disableNotificationsCheck->isChecked(); }

		/**
		 * @brief Opens the dialog and returns true if changes are made.
		 * @param commandLists initial command lists
		 * @param commandsEnabled initial enabled state of commands in gerenal
		 * @param trayIconVisible initial state of tray icon visibility
		 * @param notificationEnabled initial enabled state of notifications
		 * @param globalNotifications enabled state of notifications in gerenal
		 */
		bool execute(QList<ToggleableCommand> * commandLists, bool commandsEnabled, bool trayIconVisibility, bool notificationEnabled, bool globalNotifications);
		/**
		 * @brief Creates the object and initializes its user interface.
		 * @param parent parent widget
		 */
		CDeviceSettings(QWidget * parent = 0);
	private:
		static int m_LastIndex;

		Ui::devset ui;
		int m_LastList;
		QList<ToggleableCommand> m_CommandLists[5];
	private slots:
		void updateCommandList(int state);
		void enableAllCommands();
		void disableAllCommands();
		void addCommand();
		void removeSelectedCommands();
	};
}

#endif
