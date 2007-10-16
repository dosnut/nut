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
#include "common.h"
#include <QHeaderView>
#include <QInputDialog>

namespace qnut {
	CDeviceOptions::CDeviceOptions(CDevice * parentDevice, QTabWidget * parentTabWidget, QWidget * parent) : QTreeView(parent) {
		device = parentDevice;
		tabWidget = parentTabWidget;
		setModel(new CDeviceOptionsModel(device));
		
		deviceMenu = new QMenu(device->name, NULL);
		enableDeviceAction  = deviceMenu->addAction(QIcon(UI_ICON_DEVICE_ENABLE) , tr("Enable device") , device, SLOT(enable()));
		disableDeviceAction = deviceMenu->addAction(QIcon(UI_ICON_DEVICE_DISABLE), tr("Disable device"), device, SLOT(disable()));
		deviceMenu->addSeparator();
		showAction          = deviceMenu->addAction(QIcon(UI_ICON_ENVIRONMENT), tr("Environments..."), this, SLOT(uiShowThisTab()));
		deviceMenu->addSeparator();
		editInterfaceAction = deviceMenu->addAction(QIcon(UI_ICON_EDIT), tr("Set IP Configuration..."), this, SLOT(uiChangeIPConfiguration()));
		
		enterEnvironmentAction = new QAction(QIcon(UI_ICON_ENVIRONMENT_ENTER), tr("Enter environment"), this);
		
		editInterfaceAction->setEnabled(false);
		enterEnvironmentAction->setEnabled(false);
		
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
		delete deviceMenu;
	}
	
	void CDeviceOptions::updateDeviceIcons() {
		tabWidget->setTabIcon(tabWidget->indexOf(this), QIcon(iconFile(device)));
		deviceMenu->setIcon(QIcon(iconFile(device)));
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
	
	void CDeviceOptions::uiHandleStateChange(DeviceState state) {
		updateDeviceIcons();
		collapseAll();
		if (state == DS_UP)
			expand(model()->index(device->environments.indexOf(device->activeEnvironment), 0));
		//setDisabled(state == DS_DEACTIVATED);
		enableDeviceAction->setDisabled(state == DS_UP);
		disableDeviceAction->setDisabled(state == DS_DEACTIVATED);
		editInterfaceAction->setEnabled(state == DS_UNCONFIGURED);

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

		switch (device->state) {
			case DS_UP:             emit showMessage(tr("QNUT"), tr("%1 is now up and running.").arg(device->name), 4000);
			case DS_UNCONFIGURED:   emit showMessage(tr("QNUT"), tr("%1 got carrier but needs configuration.\n\nKlick here to open the configuration dialog.").arg(device->name), 4000);
			//case DS_CARRIER:        emit showMessage(tr("QNUT"), tr("%1 got carrier").arg(device->name), 4000);
			case DS_ACTIVATED:      emit showMessage(tr("QNUT"), tr("%1 is now activated an waits for carrier.").arg(device->name), 4000);
			case DS_DEACTIVATED:    emit showMessage(tr("QNUT"), tr("%1 is now deactivated").arg(device->name), 4000);
			default:                break;
		}

		//emit showMessage(tr("QNUT"), tr("%1 changed its state to \"%2\"").arg(device->name, toString(state)), 4000);
	}
};
