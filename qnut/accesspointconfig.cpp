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
		
		QRegExp regexp("[0123456789abcdefABCDEF]*");
		hexValidator = new QRegExpValidator(regexp, this);
		
		connect(ui.ssidHexCheck, SIGNAL(toggled(bool)), this, SLOT(convertSSID(bool)));
		connect(ui.wepKey0HexCheck, SIGNAL(toggled(bool)), this, SLOT(convertWEPKey0(bool)));
		connect(ui.wepKey1HexCheck, SIGNAL(toggled(bool)), this, SLOT(convertWEPKey1(bool)));
		connect(ui.wepKey2HexCheck, SIGNAL(toggled(bool)), this, SLOT(convertWEPKey2(bool)));
		connect(ui.wepKey3HexCheck, SIGNAL(toggled(bool)), this, SLOT(convertWEPKey3(bool)));
		connect(ui.keyManagementCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(uiHandleAuthChanged(int)));
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
			ui.confTabs->setEnabled(true);
			ui.confTabs->setTabEnabled(0, true);
			ui.confTabs->setCurrentIndex(0);
			ui.confTabs->setTabEnabled(1, false);
			break;
		case 1:
		case 3:
			ui.confTabs->setEnabled(true);
			ui.confTabs->setTabEnabled(1, true);
			ui.confTabs->setCurrentIndex(1);
			ui.confTabs->setTabEnabled(0, false);
			break;
		default:
			ui.confTabs->setEnabled(false);
			break;
		}
		
		ui.rsnCheck->setEnabled((type == 2) || (type == 3));
		
		switch (type) {
		case 0:
		case 1:
			ui.encCombo->addItem("NONE");
			ui.encCombo->addItem("WEP");
			break;
		case 2:
		case 3:
			ui.encCombo->addItem("TKIP");
			ui.encCombo->addItem("CCMP");
			break;
		default:
			ui.encCombo->addItem("UNDEFINED");
			break;
		}
	}
	
	void CAccessPointConfig::uiHandleEncChanged(QString text) {
		if (ui.keyManagementCombo->currentIndex() == 0)
			ui.confTabs->setEnabled(text == "WEP");
		else
			ui.confTabs->setTabEnabled(2, text == "WEP");
	}
	
	bool CAccessPointConfig::execute(wps_scan scanResult) {
		if (scanResult.keyManagement & WKM_WPA_EAP)
			ui.keyManagementCombo->setCurrentIndex(3);
		else if (scanResult.keyManagement & WKM_WPA_PSK)
			ui.keyManagementCombo->setCurrentIndex(2);
		else if (scanResult.keyManagement & WKM_IEEE8021X)
			ui.keyManagementCombo->setCurrentIndex(1);
		else
			ui.keyManagementCombo->setCurrentIndex(0);
		
		ui.rsnCheck->setChecked(scanResult.protocols & WP_RSN);
		
		if (
			(scanResult.ciphers & WC_WEP40) ||
			(scanResult.ciphers & WC_WEP104) ||
			(scanResult.ciphers & WC_CCMP)
		)
			ui.encCombo->setCurrentIndex(1);
		else
			ui.encCombo->setCurrentIndex(0);
		
		ui.ssidEdit->setText(scanResult.ssid);
		
		if (exec()) { //TODO:addNetwork
			return true;
		}
		else
			return false;
	}
	
	bool CAccessPointConfig::execute(int id) {
		wps_network_config config = supplicant->getNetworkConfig(id);
		
		if (config.ssid[0] == '\"')
			ui.ssidEdit->setText(config.ssid.mid(1, config.ssid.length()-2));
		else
			ui.ssidEdit->setText(config.ssid);
		
		if (config.keyManagement & WKM_WPA_EAP)
			ui.keyManagementCombo->setCurrentIndex(3);
		else if (config.keyManagement & WKM_WPA_PSK)
			ui.keyManagementCombo->setCurrentIndex(2);
		else if (config.keyManagement & WKM_IEEE8021X)
			ui.keyManagementCombo->setCurrentIndex(1);
		else
			ui.keyManagementCombo->setCurrentIndex(0);
		
		if (
			(config.group & WC_WEP40) ||
			(config.group & WC_WEP104) ||
			(config.group & WC_CCMP)
		)
			ui.encCombo->setCurrentIndex(1);
		else
			ui.encCombo->setCurrentIndex(0);
		
		if (exec()) { //TODO:configNetwork
			return true;
		}
		else
			return false;
	}
	
	inline void CAccessPointConfig::convertLineEditText(QLineEdit * lineEdit, bool hex) {
		if (hex) {
			lineEdit->setText(lineEdit->text().toAscii().toHex());
			lineEdit->setValidator(hexValidator);
		}
		else {
			lineEdit->setText(QByteArray::fromHex(lineEdit->text().toAscii()));
			ui.ssidEdit->setValidator(0);
		}
	}
	
	void CAccessPointConfig::convertSSID(bool hex) {
		convertLineEditText(ui.ssidEdit, hex);
	}
	
	void CAccessPointConfig::convertWEPKey0(bool hex) {
		convertLineEditText(ui.wepKey0Edit, hex);
	}
	
	void CAccessPointConfig::convertWEPKey1(bool hex) {
		convertLineEditText(ui.wepKey1Edit, hex);
	}
	
	void CAccessPointConfig::convertWEPKey2(bool hex) {
		convertLineEditText(ui.wepKey2Edit, hex);
	}
	
	void CAccessPointConfig::convertWEPKey3(bool hex) {
		convertLineEditText(ui.wepKey3Edit, hex);
	}
};
