//
// C++ Implementation: devicesettings
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include "cdevicesettings.h"
#include "devicedetails.h"
#include "commandlistmodel.h"

namespace qnut {
	CDeviceSettings::CDeviceSettings(QWidget * parent) : QDialog(parent) {
		ui.setupUi(this);
		ui.commandList->setModel(new CCommandListModel(this));
		connect(ui.stateCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCommandList(int)));
	}
	
	bool CDeviceSettings::execute(QList<ToggleableCommand> * commandLists, bool trayIconVisibility, bool notificationEnabled, bool globalNotifications) {
		for (int i = 0; i < 5; i++)
			m_CommandLists[i] = commandLists[i];
		
//		m_TrayIconVisibile = trayIconVisibility;
//		m_NotificationEnabled = notificationEnabled;
		
		updateCommandList(ui.stateCombo->currentIndex());
		ui.trayiconCheckBox->setChecked(trayIconVisibility);
		ui.disableNotificationsCheck->setEnabled(globalNotifications);
		ui.disableNotificationsCheck->setChecked(!notificationEnabled);
		
		return exec() == QDialog::Accepted;
	}
	
	void CDeviceSettings::updateCommandList(int state) {
		qobject_cast<CCommandListModel *>(ui.commandList->model())->setList(m_CommandLists[state]);
	}
	
	void CDeviceSettings
}
