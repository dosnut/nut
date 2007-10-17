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
#include "deviceoptions.h"
#include "deviceoptionsmodel.h"
#include "ipconfiguration.h"
#include "deviceconfiguration.h"
#include "common.h"
#include <QHeaderView>
#include <QInputDialog>

namespace qnut {
	CDeviceOptions::CDeviceOptions(CDevice * parentDevice, QTabWidget * parentTabWidget, QWidget * parent) :
		QTreeView(parent),
		settings(UI_PATH_DEV(parentDevice->name) + "dev.conf", QSettings::IniFormat, this)
	{
		device = parentDevice;
		tabWidget = parentTabWidget;
		setModel(new CDeviceOptionsModel(device));
		
		deviceMenu = new QMenu(device->name, NULL);
		enableDeviceAction    = deviceMenu->addAction(QIcon(UI_ICON_DEVICE_ENABLE) , tr("Enable device") , device, SLOT(enable()));
		disableDeviceAction   = deviceMenu->addAction(QIcon(UI_ICON_DEVICE_DISABLE), tr("Disable device"), device, SLOT(disable()));
		deviceMenu->addSeparator();
		showAction            = deviceMenu->addAction(QIcon(UI_ICON_ENVIRONMENT), tr("Environments..."), this, SLOT(uiShowThisTab()));
		deviceSettingsAction  = deviceMenu->addAction(QIcon(UI_ICON_DEVICE_SETTINGS), tr("General settings..."), this, SLOT(uiChangeDeviceSettings()));
		deviceMenu->addSeparator();
		ipConfigurationAction = deviceMenu->addAction(QIcon(UI_ICON_EDIT), tr("Set IP Configuration..."), this, SLOT(uiChangeIPConfiguration()));
		
		enterEnvironmentAction = new QAction(QIcon(UI_ICON_ENVIRONMENT_ENTER), tr("Enter environment"), this);
		
		ipConfigurationAction->setEnabled(false);
		enterEnvironmentAction->setEnabled(false);
		
		trayIcon = new QSystemTrayIcon(QIcon(iconFile(device)), this);
		trayIcon->setToolTip(shortSummary(device));
		trayIcon->setContextMenu(deviceMenu);
		
		readSettings();
		
		connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
		        this,     SLOT(uiHandleTrayActivated(QSystemTrayIcon::ActivationReason)));
		connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(uiShowThisTab()));
		
		setAllColumnsShowFocus(true);
		
		//setDisabled(device->state == DS_DEACTIVATED);
		enableDeviceAction->setDisabled(device->state == DS_UP);
		disableDeviceAction->setDisabled(device->state == DS_DEACTIVATED);
		
		setContextMenuPolicy(Qt::ActionsContextMenu);
		setAllColumnsShowFocus(true);
		setAlternatingRowColors(true);
		setIconSize(QSize(18, 18));
		
		header()->setResizeMode(QHeaderView::ResizeToContents);
		
		connect(device, SIGNAL(stateChanged(DeviceState)), this, SLOT(uiHandleStateChange(DeviceState)));
		connect(device, SIGNAL(environmentsUpdated()), this, SLOT(reset()));
		
		connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
				this            , SLOT(uiSelectionChanged(const QItemSelection &, const QItemSelection &)));
		if (device->state == DS_UP)
			expand(model()->index(device->environments.indexOf(device->activeEnvironment), 0));
	}
	
	CDeviceOptions::~CDeviceOptions() {
//nicht nötig?
		disconnect(device, SIGNAL(stateChanged(DeviceState)), this, SLOT(uiHandleStateChange(DeviceState)));
		disconnect(device, SIGNAL(environmentsUpdated()), this, SLOT(reset()));
//        CDeviceOptionsModel * targetTreeModel = (CDeviceOptionsModel *)targetDeviceOptions.environmentsTree->model();
//        targetDeviceOptions.environmentsTree->setModel(NULL);
//        delete targetTreeModel;
		writeSettings();
		delete deviceMenu;
	}
	
	void CDeviceOptions::readSettings() {
		settings.beginGroup("Main");
		scriptFlags = settings.value("scriptFlags", true).toInt();
		trayIcon->setVisible(settings.value("showTrayIcon", true).toBool());
		settings.endGroup();
	}
	
	void CDeviceOptions::writeSettings() {
		settings.beginGroup("Main");
		settings.setValue("scriptFlags", scriptFlags);
		settings.setValue("showTrayIcon", trayIcon->isVisible());
		settings.endGroup();
	}
	
	void CDeviceOptions::updateDeviceIcons() {
		tabWidget->setTabIcon(tabWidget->indexOf(this), QIcon(iconFile(device)));
		deviceMenu->setIcon(QIcon(iconFile(device)));
	}
	
	void CDeviceOptions::uiHandleTrayActivated(QSystemTrayIcon::ActivationReason reason) {
		if (reason == QSystemTrayIcon::Trigger) {
			showAction->trigger();
		}
	}
	
	void CDeviceOptions::uiShowThisTab() {
		tabWidget->setCurrentWidget(this);
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
		QModelIndex selectedIndex = (selectionModel()->selection().indexes())[0];

		dialog.execute((CInterface *)(selectedIndex.internalPointer()));
	}
	
	void CDeviceOptions::uiChangeDeviceSettings() {
		CDeviceConfiguration dialog(this);
		dialog.execute(this);
	}
	
	void CDeviceOptions::uiHandleStateChange(DeviceState state) {
		updateDeviceIcons();
		collapseAll();
		if (state == DS_UP)
			expand(model()->index(device->environments.indexOf(device->activeEnvironment), 0));
		//setDisabled(state == DS_DEACTIVATED);
		enableDeviceAction->setDisabled(state == DS_UP);
		disableDeviceAction->setDisabled(state == DS_DEACTIVATED);
		ipConfigurationAction->setEnabled(state == DS_UNCONFIGURED);

		if (!selectedIndexes().isEmpty()) {
			QModelIndex targetIndex = selectedIndexes()[0];
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
		
		//skripting hier
/*		if (scriptFlags) {
			QDir workdir(UI_PATH_DEV(parentDevice->name));
			if (scriptFlags && UI_FLAG_SCRIPT_UP) {
				workdir.cd(UI_DIR_SCRIPT_UP);
			}
			if (scriptFlags && UI_FLAG_SCRIPT_UNCONFIGURED) {
				
			}
			if (scriptFlags && UI_FLAG_SCRIPT_CARRIER) {
			}
			if (scriptFlags && UI_FLAG_SCRIPT_ACTIVATED) {
			}
			if (scriptFlags && UI_FLAG_SCRIPT_DEACTIVATED) {
			}
		}*/
	}
};
