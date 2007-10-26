//
// C++ Implementation: accesspointconfig
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "accesspointconfig.h"
#include <QVBoxLayout>

namespace qnut {
	CAccessPointConfig::CAccessPointConfig(CWpa_Supplicant * wpa_supplicant, QWidget *parent) : QDialog(parent) {
		supplicant = wpa_supplicant;
		pskWidget = new QWidget();
		eapWidget = new QWidget();
		wepWidget = new QWidget();
		
		ui.setupUi(this);
		pskUi.setupUi(pskWidget);
		eapUi.setupUi(eapWidget);
		wepUi.setupUi(wepWidget);
		
		QVBoxLayout * confLayout = new QVBoxLayout();
		confLayout->addWidget(pskWidget);
		confLayout->addWidget(eapWidget);
		confLayout->addWidget(wepWidget);
		ui.confFrame->setLayout(confLayout);
		
		connect(ui.authCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(uiHandleAuthChanged(int)));
	}
	
	CAccessPointConfig::~CAccessPointConfig() {
	
	}
	
	void CAccessPointConfig::uiHandleAuthChanged(int type) {
		pskWidget->setVisible((type == 2) || (type == 4));
		eapWidget->setVisible((type == 1) || (type == 3) || (type == 5));
	}
	
	bool CAccessPointConfig::execute(wps_scan scanResult) {
		return false;
	}
	
	bool CAccessPointConfig::execute(wps_network network) {
		return false;
	}

};
