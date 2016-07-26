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

#include "modelview/ccommandlistmodel.h"

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
		/**
		 * @brief Creates the object and initializes its user interface.
		 * @param parent parent widget
		 */
		explicit CDeviceSettings(QWidget* parent = nullptr);

		/**
		 * @brief Opens the dialog and returns true if changes are made.
		 * @param commandLists initial command lists
		 * @param commandsEnabled initial enabled state of commands in gerenal
		 * @param trayIconVisible initial state of tray icon visibility
		 * @param notificationEnabled initial enabled state of notifications
		 * @param globalNotifications enabled state of notifications in gerenal
		 */
		bool execute(std::array<QList<ToggleableCommand>, 5> &commandLists, bool& commandsEnabled, bool& trayIconVisibility, bool& notificationEnabled, bool globalNotifications);

	private:
		CCommandListModel m_CommandListModel;

		Ui::devset ui;
		int m_LastList{-1};
		std::array<QList<ToggleableCommand>, 5> m_CommandLists;

	private slots:
		void updateCommandList(int list);
		void enableAllCommands();
		void disableAllCommands();
		void addCommand();
		void removeSelectedCommands();
	};
}

#endif
