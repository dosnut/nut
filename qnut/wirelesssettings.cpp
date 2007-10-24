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
//#include "managedapmodel.h"
//#include "availableapmodel.h"

namespace qnut {
	CWirelessSettings::CWirelessSettings(CDevice * wireless, QWidget *parent) : QDialog(parent) {
		device = wireless;
		ui.setupUi(this);
		connect(device, SIGNAL(stateChanged(DeviceState)), this, SLOT(uiHandleStateChange(DeviceState)));
		//connect(device->wpa_supplicant); //hier weiter
	}
	
	CWirelessSettings::~CWirelessSettings() {
		disconnect(device, SIGNAL(stateChanged(DeviceState)), this, SLOT(uiHandleStateChange(DeviceState)));
	}
	
	void CWirelessSettings::uiHandleStateChange(DeviceState state) {
		bool wpaAvailable = (state != DS_DEACTIVATED);
		ui.switchButton->setDisabled(wpaAvailable);
		ui.rescanButton->setDisabled(wpaAvailable);
		ui.managedView->setDisabled(wpaAvailable);
		ui.availableView->setDisabled(wpaAvailable);
	}
};
