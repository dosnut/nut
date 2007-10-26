//
// C++ Implementation: wirelesssettings
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
#include "wirelesssettings.h"
#include "managedapmodel.h"
#include "common.h"
#include "availableapmodel.h"
#include "accesspointconfig.h"

namespace qnut {
	CWirelessSettings::CWirelessSettings(CDevice * wireless, QWidget * parent) : QWidget(parent) {
		device = wireless;
		
		ui.setupUi(this);
		
		setWindowTitle(tr("Wireless Settings for \"%1\"").arg(device->name));
		setWindowIcon(QIcon(UI_ICON_AIR_SETTINGS));
		
		
		ui.nameLabel->setText(device->name);
		uiHandleStateChange(device->state);
		setHeadInfo();
		ui.managedView->header()->setResizeMode(QHeaderView::ResizeToContents);
		ui.managedView->setModel(new CManagedAPModel(device->wpa_supplicant));
		ui.availableView->header()->setResizeMode(QHeaderView::ResizeToContents);
		ui.availableView->setModel(new CAvailableAPModel(device->wpa_supplicant));
		
		connect(ui.managedView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(uiSelectedDeviceChanged(const QItemSelection &, const QItemSelection &)));
		
		connect(device, SIGNAL(stateChanged(DeviceState)), this, SLOT(uiHandleStateChange(DeviceState)));
		connect(ui.switchButton, SIGNAL(clicked()), this, SLOT(switchToSelectedNetwork()));
		connect(ui.rescanButton, SIGNAL(clicked()), device->wpa_supplicant, SLOT(scan()));
		connect(ui.addButton, SIGNAL(clicked()), this, SLOT(addSelectedScanResult()));
		connect(ui.removeButton, SIGNAL(clicked()), this, SLOT(removeSelectedNetwork()));
		connect(ui.configureButton, SIGNAL(clicked()), this, SLOT(configureSelectedNetwork()));
	}
	
	CWirelessSettings::~CWirelessSettings() {
	}
	
	inline void CWirelessSettings::setHeadInfo() {
		ui.iconLabel->setPixmap(QPixmap(iconFile(device)));
		ui.statusLabel->setText(toString(device->state));
	}
	
	void CWirelessSettings::uiHandleManagedAPSelectionChanged(const QItemSelection & selected) {
		QModelIndexList selectedIndexes = selected.indexes();
		
		ui.switchButton->setEnabled(selectedIndexes.isEmpty());
		ui.configureButton->setEnabled(selectedIndexes.isEmpty());
		ui.removeButton->setEnabled(selectedIndexes.isEmpty());
	}
	
	void CWirelessSettings::uiHandleAvailableAPSelectionChanged(const QItemSelection & selected) {
		QModelIndexList selectedIndexes = selected.indexes();
		
		ui.addButton->setEnabled(selectedIndexes.isEmpty());
	}
	
	void CWirelessSettings::uiHandleStateChange(DeviceState state) {
		ui.managedGroup->setEnabled(state == DS_DEACTIVATED);
		ui.availableGroup->setEnabled(state == DS_DEACTIVATED);
		
		setHeadInfo();
	}
	
	void CWirelessSettings::switchToSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		wps_network * network = static_cast<wps_network *>(selectedIndexes[0].internalPointer());
		if (network) {
			device->wpa_supplicant->selectNetwork(network->id);
		}
	}
	
	void CWirelessSettings::addSelectedScanResult() {
		QModelIndexList selectedIndexes = ui.availableView->selectionModel()->selectedIndexes();
		CAccessPointConfig dialog(device->wpa_supplicant, this);
		wps_scan * scan = static_cast<wps_scan *>(selectedIndexes[0].internalPointer());
		dialog.execute(*scan);
	}
	
	void CWirelessSettings::removeSelectedNetwork() {
		
	}
	
	void CWirelessSettings::configureSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		CAccessPointConfig dialog(device->wpa_supplicant, this);
		wps_network * network = static_cast<wps_network *>(selectedIndexes[0].internalPointer());
		dialog.execute(*network);
	}
};
