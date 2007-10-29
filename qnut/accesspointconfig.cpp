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
		
		ui.setupUi(this);
		uiHandleAuthChanged(0);
		
		QRegExp regexp("\\d*");
		hexValidator = new QRegExpValidator(regexp, this);
		
		connect(ui.ssidHexCheck, SIGNAL(toggled(bool)), this, SLOT(convertSSID(bool)));
		connect(ui.authCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(uiHandleAuthChanged(int)));
		connect(ui.encCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(uiHandleEncChanged(QString)));
	}
	
	CAccessPointConfig::~CAccessPointConfig() {
	}
	
	void CAccessPointConfig::uiHandleAuthChanged(int type) {
		ui.encCombo->clear();
		
		switch (type) {
		case 0:
			ui.confTabs->setEnabled(false);
			ui.confTabs->setTabEnabled(2, true);
			ui.confTabs->setCurrentIndex(2);
			ui.confTabs->setTabEnabled(1, false);
			ui.confTabs->setTabEnabled(0, false);
			break;
		case 2:
		case 4:
			ui.confTabs->setEnabled(true);
			ui.confTabs->setTabEnabled(0, true);
			ui.confTabs->setCurrentIndex(0);
			ui.confTabs->setTabEnabled(1, false);
			break;
		case 1:
		case 3:
		case 5:
			ui.confTabs->setEnabled(true);
			ui.confTabs->setTabEnabled(1, true);
			ui.confTabs->setCurrentIndex(1);
			ui.confTabs->setTabEnabled(0, false);
			break;
		default:
			ui.confTabs->setEnabled(false);
			break;
		}
		
		switch (type) {
		case 0:
		case 1:
			ui.encCombo->addItem("NONE");
			ui.encCombo->addItem("WEP");
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			ui.encCombo->addItem("TKIP");
			ui.encCombo->addItem("CCMP");
			break;
		default:
			ui.encCombo->addItem("UNDEFINED");
			break;
		}
	}
	
	void CAccessPointConfig::uiHandleEncChanged(QString text) {
		if (ui.authCombo->currentIndex() == 0)
			ui.confTabs->setEnabled(text == "WEP");
		else
			ui.confTabs->setTabEnabled(2, text == "WEP");
	}
	
	void CAccessPointConfig::convertSSID(bool hex) {
		QString result;
		if (hex) {
			result = ui.ssidEdit->text().toAscii().toHex();
			ui.ssidEdit->setValidator(hexValidator);
		}
		else {
			result = QByteArray::fromHex(ui.ssidEdit->text().toAscii());
			ui.ssidEdit->setValidator(0);
		}
		ui.ssidEdit->setText(result);
	}
	
	bool CAccessPointConfig::execute(wps_scan scanResult) {
		if (scanResult.key_mgmt & KEYMGMT_WPA2_EAP)
			ui.authCombo->setCurrentIndex(5);
		else if (scanResult.key_mgmt & KEYMGMT_WPA_EAP)
			ui.authCombo->setCurrentIndex(3);
		else if (scanResult.key_mgmt & KEYMGMT_WPA2_PSK)
			ui.authCombo->setCurrentIndex(4);
		else if (scanResult.key_mgmt & KEYMGMT_WPA_PSK)
			ui.authCombo->setCurrentIndex(2);
		else if (scanResult.key_mgmt & KEYMGMT_IEEE8021X)
			ui.authCombo->setCurrentIndex(1);
		else
			ui.authCombo->setCurrentIndex(0);
		
		if (
			(scanResult.ciphers & CI_WEP) ||
			(scanResult.ciphers & CI_WEP40) ||
			(scanResult.ciphers & CI_WEP104) ||
			(scanResult.ciphers & CI_CCMP)
		)
			ui.encCombo->setCurrentIndex(1);
		else
			ui.encCombo->setCurrentIndex(0);
		
		ui.ssidEdit->setText(scanResult.ssid);
		
		if (exec()) {
			return true;
		}
		else
			return false;
	}
	
	bool CAccessPointConfig::execute(wps_network network) {
		
		if (exec()) {
			return true;
		}
		else
			return false;
	}

};
