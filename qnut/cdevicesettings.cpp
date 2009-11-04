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
	CDeviceSettings::CDeviceSettings(QWidget * parent) : QDialog(parent), m_LastList(-1) {
		ui.setupUi(this);
		ui.commandList->setModel(new CCommandListModel(this));
		connect(ui.stateCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCommandList(int)));
		connect(ui.addButton, SIGNAL(clicked()), this, SLOT(addCommand()));
		connect(ui.removeButton, SIGNAL(clicked()), this, SLOT(removeSelectedCommands()));
		connect(ui.enableAllButton, SIGNAL(clicked()), this, SLOT(enableAllCommands()));
		connect(ui.disableAllButton, SIGNAL(clicked()), this, SLOT(disableAllCommands()));
	}
	
	bool CDeviceSettings::execute(QList<ToggleableCommand> * commandLists, bool trayIconVisibility, bool notificationEnabled, bool globalNotifications) {
		for (int i = 0; i < 5; i++)
			m_CommandLists[i] = commandLists[i];
		
		updateCommandList(ui.stateCombo->currentIndex());
		ui.trayiconCheckBox->setChecked(trayIconVisibility);
		ui.disableNotificationsCheck->setEnabled(globalNotifications);
		ui.disableNotificationsCheck->setChecked(!notificationEnabled);
		
		return exec() == QDialog::Accepted;
	}
	
	void CDeviceSettings::updateCommandList(int state) {
		if (m_LastList != -1)
			m_CommandLists[m_LastList] = qobject_cast<CCommandListModel *>(ui.commandList->model())->cachedList();
		
		qobject_cast<CCommandListModel *>(ui.commandList->model())->setList(m_CommandLists[state]);
		
		m_LastList = state;
	}
	
	void CDeviceSettings::enableAllCommands() {
		qobject_cast<CCommandListModel *>(ui.commandList->model())->setAllEnabled(true);
	}
	
	void CDeviceSettings::disableAllCommands() {
		qobject_cast<CCommandListModel *>(ui.commandList->model())->setAllEnabled(false);
	}
	
	void CDeviceSettings::addCommand() {
		ui.commandList->edit(qobject_cast<CCommandListModel *>(ui.commandList->model())->appendRow());
	}
	
	void CDeviceSettings::removeSelectedCommands() {
		QModelIndexList selected = ui.commandList->selectionModel()->selectedIndexes();
		foreach (QModelIndex i, selected)
			ui.commandList->model()->removeRow(i.row());
	}
}
