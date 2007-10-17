//
// C++ Implementation: ipconf
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "ipconfiguration.h"

namespace qnut {

	bool CIPConfiguration::execute(CInterface * interface) {
		ui.ipEdit->setText(interface->ip.toString());
		ui.gatewayEdit->setText(interface->gateway.toString());
		ui.netmaskEdit->setText(interface->netmask.toString());
		if (exec() == QDialog::Accepted) {
			QHostAddress tempAddr;
			
			tempAddr.setAddress(ui.ipEdit->text());
			interface->setIP(tempAddr);
			
			tempAddr.setAddress(ui.gatewayEdit->text());
			interface->setGateway(tempAddr);
			
			tempAddr.setAddress(ui.netmaskEdit->text());
			interface->setNetmask(tempAddr);
			
			if (ui.dynamicRadio->isChecked())
				interface->setDynamic();
			return true;
		}
		else {
			return false;
		}
	}
	
	CIPConfiguration::CIPConfiguration(QWidget * parent) : QDialog(parent) {
		ui.setupUi(this);
	}
	
	CIPConfiguration::~CIPConfiguration() {
	}
};
