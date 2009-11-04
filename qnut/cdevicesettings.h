//
// C++ Interface: devicesettings
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_DEVICESETTINGS_H
#define QNUT_DEVICESETTINGS_H

#include <QDialog>
#include "ui/ui_devset.h"
#include "common.h"

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
		inline const QList<ToggleableCommand> * commandListsResult() const { return m_CommandLists; }
		inline bool sctiptsEnabledResult() const { return ui.scriptBox->isChecked(); }
		inline bool trayIconVisibleResult() const { return ui.trayiconCheckBox->isChecked(); }
		inline bool notificationEnabledResult() const { return !ui.disableNotificationsCheck->isChecked(); }
		/**
		 * @brief Opens the dialog and returns true if changes are made.
		 * @param commandLists
		 * @param trayIconVisible
		 * @param notificationEnabled
		 * @param globalNotifications
		 */
		bool execute(QList<ToggleableCommand> * commandLists, bool trayIconVisibility, bool notificationEnabled, bool globalNotifications);
		/**
		 * @brief Creates the object and initializes its user interface.
		 * @param parent parent widget
		 */
		CDeviceSettings(QWidget * parent = 0);
	private:
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
