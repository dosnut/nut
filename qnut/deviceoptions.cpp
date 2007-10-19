//
// C++ Implementation: deviceoptions
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QHeaderView>
#include <QInputDialog>
#include <QProcess>
#include "deviceoptions.h"
#include "deviceoptionsmodel.h"
#include "ipconfiguration.h"
#include "scriptsettings.h"
#include "common.h"

namespace qnut {
	CDeviceOptions::CDeviceOptions(CDevice * parentDevice, QWidget * parent) :
		QWidget(parent),
		settings(UI_PATH_DEV(parentDevice->name) + "dev.conf", QSettings::IniFormat, this)
	{
		device = parentDevice;
		
		createView();
		createActions();
		
		trayIcon->setToolTip(shortSummary(device));
		trayIcon->setContextMenu(deviceMenu);
		connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(uiShowThisTab()));
		connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			this, SLOT(uiHandleTrayActivated(QSystemTrayIcon::ActivationReason)));
		
		readSettings();
		
		
		connect(device, SIGNAL(stateChanged(DeviceState)), this, SLOT(uiHandleStateChange(DeviceState)));
		
		if (device->state == DS_UP)
			environmentTree->expand(environmentTree->model()->index(device->environments.indexOf(device->activeEnvironment), 0));
	}
	
	CDeviceOptions::~CDeviceOptions() {
		disconnect(device, SIGNAL(stateChanged(DeviceState)), this, SLOT(uiHandleStateChange(DeviceState)));
		disconnect(device, SIGNAL(environmentsUpdated()), environmentTree, SLOT(reset()));
		writeSettings();
		delete deviceMenu;
	}
	
	inline void CDeviceOptions::readSettings() {
		settings.beginGroup("Main");
		scriptFlags = settings.value("scriptFlags", true).toInt();
		trayIcon->setVisible(settings.value("showTrayIcon", true).toBool());
		settings.endGroup();
		showTrayCheck->setChecked(trayIcon->isVisible());
	}
	
	inline void CDeviceOptions::writeSettings() {
		settings.beginGroup("Main");
		settings.setValue("scriptFlags", scriptFlags);
		settings.setValue("showTrayIcon", trayIcon->isVisible());
		settings.endGroup();
	}
	
	inline void CDeviceOptions::createActions() {
		deviceMenu = new QMenu(device->name, NULL);
		enableDeviceAction    = deviceMenu->addAction(QIcon(UI_ICON_DEVICE_ENABLE) , tr("Enable device") , device, SLOT(enable()));
		disableDeviceAction   = deviceMenu->addAction(QIcon(UI_ICON_DEVICE_DISABLE), tr("Disable device"), device, SLOT(disable()));
		deviceMenu->addSeparator();
		showAction            = deviceMenu->addAction(QIcon(UI_ICON_ENVIRONMENT), tr("Environments..."), this, SLOT(uiShowThisTab()));
		deviceSettingsAction  = deviceMenu->addAction(QIcon(UI_ICON_SCRIPT_SETTINGS), tr("Scripting settings..."), this, SLOT(uiChangeDeviceSettings()));
		ipConfigurationAction = deviceMenu->addAction(QIcon(UI_ICON_EDIT), tr("Set IP Configuration..."), this, SLOT(uiChangeIPConfiguration()));
		
		enterEnvironmentAction = new QAction(QIcon(UI_ICON_ENVIRONMENT_ENTER), tr("Enter environment"), this);
		environmentTree->addAction(enterEnvironmentAction);
		
		enableDeviceAction->setEnabled(device->state == DS_DEACTIVATED);
		disableDeviceAction->setDisabled(device->state == DS_DEACTIVATED);
		ipConfigurationAction->setEnabled(false);
		enterEnvironmentAction->setEnabled(false);
	}
	
	inline void CDeviceOptions::createView() {
		trayIcon = new QSystemTrayIcon(QIcon(iconFile(device)), this);
		
		showTrayCheck = new QCheckBox(tr("Show tray icon for this device"));
		connect(showTrayCheck, SIGNAL(toggled(bool)), trayIcon, SLOT(setVisible(bool)));
		
		environmentTree = new QTreeView();
		environmentTree->setContextMenuPolicy(Qt::ActionsContextMenu);
		environmentTree->setAllColumnsShowFocus(true);
		environmentTree->setAlternatingRowColors(true);
		environmentTree->setIconSize(QSize(18, 18));
		environmentTree->setAllColumnsShowFocus(true);
		environmentTree->header()->setResizeMode(QHeaderView::ResizeToContents);
		environmentTree->setModel(new CDeviceOptionsModel(device));
		connect(environmentTree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(uiSelectionChanged(const QItemSelection &, const QItemSelection &)));
		connect(device, SIGNAL(environmentsUpdated()), environmentTree, SLOT(reset()));
		//todo: interfacesänderungen hier
		
		statusIcon = new QLabel();
		statusText = new QLabel();
		statusIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		setHeadInfo();
		
		QHBoxLayout * headlayout = new QHBoxLayout();
		headlayout->addWidget(statusIcon);
		headlayout->addWidget(statusText);
		
		QVBoxLayout * mainlayout = new QVBoxLayout();
		mainlayout->addLayout(headlayout);
		mainlayout->addWidget(environmentTree);
		mainlayout->addWidget(showTrayCheck);
		setLayout(mainlayout);
	}
	
	inline void CDeviceOptions::setHeadInfo() {
		statusIcon->setPixmap(QPixmap(iconFile(device)));
		statusText->setText(toString(device->state));
	}
	
	void CDeviceOptions::uiHandleTrayActivated(QSystemTrayIcon::ActivationReason reason) {
		if (reason == QSystemTrayIcon::Trigger) {
			showAction->trigger();
		}
	}
	
	void CDeviceOptions::uiShowThisTab() {
		emit showOptions(this);
	}
	
	void CDeviceOptions::uiSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected) {
		QModelIndexList deselectedIndexes = deselected.indexes();
		QModelIndexList selectedIndexes = selected.indexes();
		
		if (!deselectedIndexes.isEmpty()) {
			QModelIndex targetIndex = deselectedIndexes[0];
			if (!targetIndex.parent().isValid()) {
				CEnvironment * target = (CEnvironment *)(targetIndex.internalPointer());
				disconnect(target, SIGNAL(activeChanged(bool)), enterEnvironmentAction, SLOT(setDisabled(bool)));
				disconnect(enterEnvironmentAction, SIGNAL(triggered()), target, SLOT(enter()));
			}
		}
		
		if (!selectedIndexes.isEmpty()) {
			QModelIndex targetIndex = selectedIndexes[0];
			if (!targetIndex.parent().isValid()) {
				CEnvironment * target = (CEnvironment *)(targetIndex.internalPointer());
				connect(target, SIGNAL(activeChanged(bool)), enterEnvironmentAction, SLOT(setDisabled(bool)));
				connect(enterEnvironmentAction, SIGNAL(triggered()), target, SLOT(enter()));
				
				enterEnvironmentAction->setDisabled(target == device->activeEnvironment);
			}
			else {
				enterEnvironmentAction->setEnabled(false);
			}
		}
		else {
			enterEnvironmentAction->setEnabled(false);
		}
	}
	
	void CDeviceOptions::uiChangeIPConfiguration() {
		CIPConfiguration dialog(this);
		QModelIndex selectedIndex = (environmentTree->selectionModel()->selection().indexes())[0];

		dialog.execute((CInterface *)(selectedIndex.internalPointer()));
	}
	
	void CDeviceOptions::uiChangeDeviceSettings() {
		CScriptSettings dialog(this);
		dialog.execute(this);
	}
	
	void CDeviceOptions::uiHandleStateChange(DeviceState state) {
		setHeadInfo();
		environmentTree->collapseAll();
		if (state == DS_UP)
			environmentTree->expand(environmentTree->model()->index(device->environments.indexOf(device->activeEnvironment), 0));
		
		enableDeviceAction->setEnabled(state == DS_DEACTIVATED);
		disableDeviceAction->setDisabled(state == DS_DEACTIVATED);
		ipConfigurationAction->setEnabled(state == DS_UNCONFIGURED);

		if (!environmentTree->selectionModel()->selectedIndexes().isEmpty()) {
			QModelIndex targetIndex = environmentTree->selectionModel()->selectedIndexes()[0];
			if (!targetIndex.parent().isValid()) {
				CEnvironment * target = (CEnvironment *)(targetIndex.internalPointer());
				enterEnvironmentAction->setDisabled(target == device->activeEnvironment);
			}
			else {
				enterEnvironmentAction->setEnabled(false);
			}
		}
		else {
			enterEnvironmentAction->setEnabled(false);
		}
		if (trayIcon->isVisible()) {
			trayIcon->setToolTip(shortSummary(device));
			trayIcon->setIcon(QIcon(iconFile(device)));
			
			switch (state) {
			case DS_UP:
				emit showMessage(trayIcon, tr("QNUT - %1...").arg(device->name),
					tr("...is now up and running."));
				break;
			case DS_UNCONFIGURED:
				emit showMessage(trayIcon, tr("QNUT - %1...").arg(device->name),
					tr("...got carrier but needs configuration.\n\nKlick here to open the configuration dialog."));
				break;
			case DS_ACTIVATED:
				emit showMessage(trayIcon, tr("QNUT - %1...").arg(device->name),
					tr("...is now activated an waits for carrier."));
				break;
			case DS_DEACTIVATED: 
				emit showMessage(trayIcon, tr("QNUT - %1...").arg(device->name),
					tr("...is now deactivated"));
				break;
			default:
				break;
			}
		}
		else {
			switch (state) {
			case DS_UP:
				emit showMessage(NULL, tr("QNUT"),
					tr("%1 is now up and running.").arg(device->name));
				break;
			case DS_UNCONFIGURED:
				emit showMessage(NULL, tr("QNUT"),
					tr("%1 got carrier but needs configuration.\n\nKlick here to open the configuration dialog.").arg(device->name));
				break;
			case DS_ACTIVATED:
				emit showMessage(NULL, tr("QNUT"),
					tr("%1 is now activated an waits for carrier.").arg(device->name));
				break;
			case DS_DEACTIVATED:
				emit showMessage(NULL, tr("QNUT"),
					tr("%1 is now deactivated").arg(device->name));
				break;
			default:
				break;
			}
		}
		//emit showMessage(tr("QNUT"), tr("%1 changed its state to \"%2\"").arg(device->name, toString(state)), 4000);
		
		if (scriptFlags) {
			QDir workdir(UI_PATH_DEV(device->name));
			if (scriptFlags && UI_FLAG_SCRIPT_UP) {
				workdir.cd(UI_DIR_SCRIPT_UP);
				foreach(QString i, workdir.entryList()) {
					QProcess::startDetached(workdir.path() + i);
				}
				workdir.cdUp();
			}
			if (scriptFlags && UI_FLAG_SCRIPT_UNCONFIGURED) {
				workdir.cd(UI_DIR_SCRIPT_UNCONFIGURED);
				foreach(QString i, workdir.entryList()) {
					QProcess::startDetached(workdir.path() + i);
				}
				workdir.cdUp();
			}
			if (scriptFlags && UI_FLAG_SCRIPT_CARRIER) {
				workdir.cd(UI_DIR_SCRIPT_UNCONFIGURED);
				foreach(QString i, workdir.entryList()) {
					QProcess::startDetached(workdir.path() + i);
				}
				workdir.cdUp();
			}
			if (scriptFlags && UI_FLAG_SCRIPT_ACTIVATED) {
				workdir.cd(UI_DIR_SCRIPT_UNCONFIGURED);
				foreach(QString i, workdir.entryList()) {
					QProcess::startDetached(workdir.path() + i);
				}
				workdir.cdUp();
			}
			if (scriptFlags && UI_FLAG_SCRIPT_DEACTIVATED) {
				workdir.cd(UI_DIR_SCRIPT_UNCONFIGURED);
				foreach(QString i, workdir.entryList()) {
					QProcess::startDetached(workdir.path() + i);
				}
				workdir.cdUp();
			}
		}
	}
};
