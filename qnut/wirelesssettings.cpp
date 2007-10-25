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
#include "wirelesssettings.h"
#include "managedapmodel.h"
#include "common.h"
#include <QHeaderView>
//#include "availableapmodel.h"

namespace qnut {
	CWirelessSettings::CWirelessSettings(CDevice * wireless, QWidget *parent) : QDialog(parent) {
		device = wireless;
		ui.setupUi(this);
		ui.nameLabel->setText(device->name);
		uiHandleStateChange(device->state);
		setHeadInfo();
		ui.managedView->header()->setResizeMode(QHeaderView::ResizeToContents);
		ui.managedView->setModel(new CManagedAPModel(device->wpa_supplicant));
		//ui.availableView->setModel(new CAvailableAPModel(device->wpa_supplicant));
		connect(device, SIGNAL(stateChanged(DeviceState)), this, SLOT(uiHandleStateChange(DeviceState)));
		connect(ui.switchButton, SIGNAL(clicked()), this, SLOT(uiHandleSwitchNetwork()));
		connect(ui.rescanButton, SIGNAL(clicked()), device->wpa_supplicant, SLOT(scan()));
	}
	
	CWirelessSettings::~CWirelessSettings() {
		//disconnect(device, SIGNAL(stateChanged(DeviceState)), this, SLOT(uiHandleStateChange(DeviceState)));
	}
	
	inline void CWirelessSettings::setHeadInfo() {
		ui.iconLabel->setPixmap(QPixmap(iconFile(device)));
		ui.statusLabel->setText(toString(device->state));
	}
	
	void CWirelessSettings::uiHandleStateChange(DeviceState state) {
		bool wpaAvailable = (state != DS_DEACTIVATED);
		ui.switchButton->setEnabled(wpaAvailable);
		ui.rescanButton->setEnabled(wpaAvailable);
		ui.managedView->setEnabled(wpaAvailable);
		ui.availableView->setEnabled(wpaAvailable);
		
		setHeadInfo();
	}
	
	void CWirelessSettings::uiHandleSwitchNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		wps_network * network = static_cast<wps_network *>(selectedIndexes[0].internalPointer());
		if (network) {
			device->wpa_supplicant->selectNetwork(network->id);
		}
	}
};
