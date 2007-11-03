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
#include <QFileDialog>

namespace qnut {
	CAccessPointConfig::CAccessPointConfig(CWpa_Supplicant * wpa_supplicant, QWidget *parent) : QDialog(parent) {
		supplicant = wpa_supplicant;
		
		ui.setupUi(this);
		setAuthConfig(0);
		
		QRegExp regexp("[0123456789abcdefABCDEF]*");
		hexValidator = new QRegExpValidator(regexp, this);
		
		connect(ui.ssidHexCheck, SIGNAL(toggled(bool)), this, SLOT(convertSSID(bool)));
		connect(ui.wepKey0HexCheck, SIGNAL(toggled(bool)), this, SLOT(convertWEPKey0(bool)));
		connect(ui.wepKey1HexCheck, SIGNAL(toggled(bool)), this, SLOT(convertWEPKey1(bool)));
		connect(ui.wepKey2HexCheck, SIGNAL(toggled(bool)), this, SLOT(convertWEPKey2(bool)));
		connect(ui.wepKey3HexCheck, SIGNAL(toggled(bool)), this, SLOT(convertWEPKey3(bool)));
		connect(ui.pskEdit, SIGNAL(textChanged(QString)), this, SLOT(countPskChars(QString)));
		connect(ui.showPlainPSKCheck, SIGNAL(toggled(bool)), this, SLOT(togglePlainPSK(bool)));
		connect(ui.keyManagementCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setAuthConfig(int)));
		connect(ui.encCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(setEncConfig(QString)));
		connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(verifyConfiguration()));
		connect(ui.caBrowse, SIGNAL(clicked()), this, SLOT(selectCAFile()));
		connect(ui.clientBrowse, SIGNAL(clicked()), this, SLOT(selectClientFile()));
		connect(ui.keyBrowse, SIGNAL(clicked()), this, SLOT(selectKeyFile()));
	}
	
	CAccessPointConfig::~CAccessPointConfig() {
	}
	
	void CAccessPointConfig::setAuthConfig(int type) {
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
	
	void CAccessPointConfig::setEncConfig(QString text) {
		if (ui.keyManagementCombo->currentIndex() == 0)
			ui.confTabs->setEnabled(text == "WEP");
		else
			ui.confTabs->setTabEnabled(2, text == "WEP");
	}
		
	void CAccessPointConfig::verifyConfiguration() {
		wps_netconfig_status status;
		wps_network_config config;
		
		config.ssid = ui.ssidHexCheck->isChecked() ? ui.ssidEdit->text() : '\"' + ui.ssidEdit->text() + '\"';
		
		if (ui.encCombo->currentText() == "WEP") {
			config.group = (wps_group_ciphers)(WGC_WEP104 | WGC_WEP40);
			config.wep_key0 = ui.wepKey0HexCheck->isChecked() ? ui.wepKey0Edit->text() : '\"' + ui.wepKey0Edit->text() + '\"';
			config.wep_key1 = ui.wepKey1HexCheck->isChecked() ? ui.wepKey1Edit->text() : '\"' + ui.wepKey1Edit->text() + '\"';
			config.wep_key2 = ui.wepKey2HexCheck->isChecked() ? ui.wepKey2Edit->text() : '\"' + ui.wepKey2Edit->text() + '\"';
			config.wep_key3 = ui.wepKey3HexCheck->isChecked() ? ui.wepKey3Edit->text() : '\"' + ui.wepKey3Edit->text() + '\"';
		}
		else if (ui.encCombo->currentText() == "CCMP")
			config.group = WGC_CCMP;
		else if (ui.encCombo->currentText() == "TKIP")
			config.group = WGC_TKIP;
		
		switch (ui.keyManagementCombo->currentIndex()) {
		case 0:
			config.keyManagement = WKM_NONE;
			break;
		case 1:
			config.keyManagement = WKM_IEEE8021X;
			writeEAPConfig(config.eap_config);
			break;
		case 2:
			config.keyManagement = WKM_WPA_PSK;
			
			if (ui.rsnCheck->isChecked())
				config.protocols = WP_WPA;
			else
				config.protocols = WP_RSN;
			
			if (ui.pskEdit->text().length() == 0)
				config.psk = QString("");
			else
				config.psk = '\"' + ui.pskEdit->text() + '\"';
			
			break;
		case 3:
			config.keyManagement = WKM_WPA_EAP;
			
			if (ui.rsnCheck->isChecked())
				config.protocols = WP_WPA;
			else
				config.protocols = WP_RSN;
			
			writeEAPConfig(config.eap_config);
			break;
		default:
			break;
		}
		
		if (currentID > -1) {
			status = supplicant->editNetwork(currentID, config);
		}
		else {
			status = supplicant->addNetwork(config);
		}
		
		if (status.failures != WCF_NONE) {
/*	WCF_NONE=0, WCF_SSID=1,WCF_BSSID=2,WCF_DISABLED=4,WCF_ID_STR=8,
	WCF_SCAN_SSID=16, WCF_PRIORITY=32, WCF_MODE=64, WCF_FREQ=128,
	WCF_PROTO=256, WCF_KEYMGMT=512, WCF_AUTH_ALG=1024, WCF_PAIRWISE=2048,
	WCF_GROUP=0x0000001000, WCF_PSK=0x0000002000, WCF_EAPOL_FLAGS=0x0000004000, WCF_MIXED_CELL=0x0000008000,
	WCF_PROA_KEY_CACHING=0x00000010000, WCF_WEP_KEY0=0x0000020000, WCF_WEP_KEY1=0x0000040000, WCF_WEP_KEY2=0x0000080000,
	WCF_WEP_KEY3=0x0000100000, WCF_WEP_KEY_IDX=0x0000200000, WCF_PEERKEY=0x0000400000, WCF_ALL=0x00007FFFFF*/
			if (status.failures & WCF_SSID)             qDebug("WCF_SSID");
			if (status.failures & WCF_BSSID)            qDebug("WCF_BSSID");
			if (status.failures & WCF_DISABLED)         qDebug("WCF_DISABLED");
			if (status.failures & WCF_ID_STR)           qDebug("WCF_ID_STR");
			if (status.failures & WCF_SCAN_SSID)        qDebug("WCF_SCAN_SSID");
			if (status.failures & WCF_PRIORITY)         qDebug("WCF_PRIORITY");
			if (status.failures & WCF_MODE)             qDebug("WCF_MODE");
			if (status.failures & WCF_FREQ)             qDebug("WCF_FREQ");
			if (status.failures & WCF_PROTO)            qDebug("WCF_PROTO");
			if (status.failures & WCF_KEYMGMT)          qDebug("WCF_KEYMGMT");
			if (status.failures & WCF_AUTH_ALG)         qDebug("WCF_AUTH_ALG");
			if (status.failures & WCF_PAIRWISE)         qDebug("WCF_PAIRWISE");
			if (status.failures & WCF_GROUP)            qDebug("WCF_GROUP");
			if (status.failures & WCF_PSK)              qDebug("WCF_PSK");
			if (status.failures & WCF_EAPOL_FLAGS)      qDebug("WCF_EAPOL_FLAGS");
			if (status.failures & WCF_MIXED_CELL)       qDebug("WCF_MIXED_CELL");
			if (status.failures & WCF_PROA_KEY_CACHING) qDebug("WCF_PROA_KEY_CACHING");
			if (status.failures & WCF_WEP_KEY0)         qDebug("WCF_WEP_KEY0");
			if (status.failures & WCF_WEP_KEY1)         qDebug("WCF_WEP_KEY1");
			if (status.failures & WCF_WEP_KEY2)         qDebug("WCF_WEP_KEY2");
			if (status.failures & WCF_WEP_KEY3)         qDebug("WCF_WEP_KEY3");
			if (status.failures & WCF_WEP_KEY_IDX)      qDebug("WCF_WEP_KEY_IDX");
			if (status.failures & WCF_PEERKEY)          qDebug("WCF_PEERKEY");
			return;
		}
		
		if (status.eap_failures != WECF_NONE) {
			qDebug("eap failures");
			return;
		}
		
		accept();
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
		
		currentID = -1;
		
		return exec();
	}
	
	bool CAccessPointConfig::execute(int id) {
		wps_network_config config = supplicant->getNetworkConfig(id);
		
		if (config.ssid[0] == '\"')
			ui.ssidEdit->setText(config.ssid.mid(1, config.ssid.length()-2));
		else
			ui.ssidEdit->setText(config.ssid);
		
		if (config.keyManagement & WKM_WPA_EAP) {
			ui.keyManagementCombo->setCurrentIndex(3);
			
		}
		else if (config.keyManagement & WKM_WPA_PSK) {
			ui.keyManagementCombo->setCurrentIndex(2);
			ui.pskEdit->setText("");
		}
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
		
		ui.rsnCheck->setChecked(config.protocols & WP_RSN);
		
		currentID = id;
		
		return exec();
	}
	
	inline void CAccessPointConfig::writeEAPConfig(wps_eap_network_config &eap_config) {
		switch (ui.encCombo->currentIndex()) {
		case 0: eap_config.eap = EAPM_MD5; break;
		case 1: eap_config.eap = EAPM_TLS; break;
		case 2: eap_config.eap = EAPM_MSCHAPV2; break;
		case 3: eap_config.eap = EAPM_PEAP; break;
		case 4: eap_config.eap = EAPM_TTLS; break;
		case 5: eap_config.eap = EAPM_GTC; break;
		case 6: eap_config.eap = EAPM_OTP; break;
		case 7: eap_config.eap = EAPM_LEAP; break;
		default: break;
		}
		eap_config.identity = '\"' + ui.identificationEdit->text() + '\"';
		eap_config.ca_cert = '\"' + ui.caEdit->text() + '\"';
		eap_config.client_cert = '\"' + ui.clientEdit->text() + '\"';
		eap_config.private_key = '\"' + ui.keyFileEdit->text() + '\"';
		eap_config.private_key_passwd = '\"' + ui.passwordEdit->text() + '\"';
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
	
	void CAccessPointConfig::countPskChars(QString psk) {
		ui.charCountLabel->setText(tr("%1 chars").arg(psk.length()));
	}
	
	void CAccessPointConfig::togglePlainPSK(bool show) {
		if (show)
			ui.pskEdit->setEchoMode(QLineEdit::Normal);
		else
			ui.pskEdit->setEchoMode(QLineEdit::Password);
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
	
	void CAccessPointConfig::selectCAFile() {
		ui.caEdit->setText(QFileDialog::getOpenFileName(this,
			tr("Select CA certificate file"), "/", tr("Certificate files (%1)").arg("*.pem")));
	}
	
	void CAccessPointConfig::selectClientFile() {
		ui.clientEdit->setText(QFileDialog::getOpenFileName(this,
			tr("Select client certificate file"), "/", tr("Certificate files (%1)").arg("*.pem")));
	}
	
	void CAccessPointConfig::selectKeyFile() {
		ui.keyFileEdit->setText(QFileDialog::getOpenFileName(this,
			tr("Select key file"), "/", tr("Key files (%1)").arg("*.pem")));
	}
};
