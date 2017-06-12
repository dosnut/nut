//
// C++ Implementation: CDeviceSettings
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include "cdevicesettings.h"

namespace qnut {
	CDeviceSettings::CDeviceSettings(QWidget* parent) : QDialog(parent), m_CommandListModel(this) {
		ui.setupUi(this);
		ui.commandList->setModel(&m_CommandListModel);
		connect(ui.stateCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CDeviceSettings::updateCommandList);
		connect(ui.addButton, &QPushButton::clicked, this, &CDeviceSettings::addCommand);
		connect(ui.removeButton, &QPushButton::clicked, this, &CDeviceSettings::removeSelectedCommands);
		connect(ui.enableAllButton, &QPushButton::clicked, this, &CDeviceSettings::enableAllCommands);
		connect(ui.disableAllButton, &QPushButton::clicked, this, &CDeviceSettings::disableAllCommands);
	}

	bool CDeviceSettings::execute(std::array<QList<ToggleableCommand>, 5>& commandLists, bool& commandsEnabled, bool& trayIconVisibility, bool& notificationEnabled, bool globalNotifications) {
		m_CommandLists = commandLists;

		m_LastList = ui.stateCombo->currentIndex();
		if (m_LastList != -1) {
			m_CommandListModel.setList(m_CommandLists[m_LastList]);
		}

		ui.trayiconCheckBox->setChecked(trayIconVisibility);
		ui.disableNotificationsCheck->setEnabled(globalNotifications);
		ui.disableNotificationsCheck->setChecked(!notificationEnabled);
		ui.scriptBox->setChecked(commandsEnabled);

		auto result = exec();

		if (QDialog::Accepted == result) {
			if (m_LastList != -1) {
				m_CommandLists[m_LastList] = m_CommandListModel.cachedList();
			}
			commandLists = m_CommandLists;
			trayIconVisibility = ui.trayiconCheckBox->isChecked();
			notificationEnabled = !ui.disableNotificationsCheck->isChecked();
			commandsEnabled = ui.scriptBox->isChecked();

			return true;
		}
		else {
			return false;
		}
	}

	void CDeviceSettings::updateCommandList(int list) {
		if (m_LastList != -1) {
			m_CommandLists[m_LastList] = m_CommandListModel.cachedList();
		}

		if (list != -1) {
			m_CommandListModel.setList(m_CommandLists[list]);
		} else {
			m_CommandListModel.setList(QList<ToggleableCommand>());
		}

		m_LastList = list;
	}

	void CDeviceSettings::enableAllCommands() {
		m_CommandListModel.setAllEnabled(true);
	}

	void CDeviceSettings::disableAllCommands() {
		m_CommandListModel.setAllEnabled(false);
	}

	void CDeviceSettings::addCommand() {
		ToggleableCommand newCommand;
		newCommand.path = tr("echo 'New Command'");
//		ui.commandList->edit(
		m_CommandListModel.appendRow(newCommand);//);
	}

	void CDeviceSettings::removeSelectedCommands() {
		QModelIndexList selected = ui.commandList->selectionModel()->selectedIndexes();
		foreach (QModelIndex i, selected)
			m_CommandListModel.removeRow(i.row());
	}
}
