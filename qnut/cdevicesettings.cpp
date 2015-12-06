//
// C++ Implementation: CDeviceSettings
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include "cdevicesettings.h"
#include "modelview/ccommandlistmodel.h"

namespace qnut {
	int CDeviceSettings::m_LastIndex = 0;

	CDeviceSettings::CDeviceSettings(QWidget * parent) : QDialog(parent), m_LastList(-1) {
		ui.setupUi(this);
		ui.commandList->setModel(new CCommandListModel(this));
		connect(ui.stateCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CDeviceSettings::updateCommandList);
		connect(ui.addButton, &QPushButton::clicked, this, &CDeviceSettings::addCommand);
		connect(ui.removeButton, &QPushButton::clicked, this, &CDeviceSettings::removeSelectedCommands);
		connect(ui.enableAllButton, &QPushButton::clicked, this, &CDeviceSettings::enableAllCommands);
		connect(ui.disableAllButton, &QPushButton::clicked, this, &CDeviceSettings::disableAllCommands);
	}

	bool CDeviceSettings::execute(QList<ToggleableCommand> * commandLists, bool commandsEnabled, bool trayIconVisibility, bool notificationEnabled, bool globalNotifications) {
		for (int i = 0; i < 5; i++)
			m_CommandLists[i] = commandLists[i];

		ui.stateCombo->setCurrentIndex(m_LastIndex);
		ui.trayiconCheckBox->setChecked(trayIconVisibility);
		ui.disableNotificationsCheck->setEnabled(globalNotifications);
		ui.disableNotificationsCheck->setChecked(!notificationEnabled);
		ui.scriptBox->setChecked(commandsEnabled);

		if (exec() == QDialog::Accepted) {
			m_CommandLists[ui.stateCombo->currentIndex()] = qobject_cast<CCommandListModel *>(ui.commandList->model())->cachedList();
			m_LastIndex = ui.stateCombo->currentIndex();
			return true;
		}
		else
			return false;
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
		ToggleableCommand newCommand;
		newCommand.path = tr("echo 'New Command'");
//		ui.commandList->edit(
		qobject_cast<CCommandListModel *>(ui.commandList->model())->appendRow(newCommand);//);
	}

	void CDeviceSettings::removeSelectedCommands() {
		QModelIndexList selected = ui.commandList->selectionModel()->selectedIndexes();
		foreach (QModelIndex i, selected)
			ui.commandList->model()->removeRow(i.row());
	}
}
