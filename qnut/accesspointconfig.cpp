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
#include <QToolTip>

namespace qnut {
	using namespace libnutcommon;
	using namespace libnutwireless;
	
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
		delete hexValidator;
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
		NetconfigStatus status;
		NetworkConfig config;
		
		config.ssid = ui.ssidHexCheck->isChecked() ? ui.ssidEdit->text() : '\"' + ui.ssidEdit->text() + '\"';
		
		config.disabled = (BOOL)ui.autoEnableCheck->isChecked();
		config.scan_ssid = (BOOL)ui.scanCheck->isChecked();
		
		if (!ui.anyBSSIDCheck->isChecked())
			config.bssid = MacAddress(ui.ssidEdit->text());
		
		if (ui.encCombo->currentText() == "WEP") {
			config.group = (GroupCiphers)(GCI_WEP104 | GCI_WEP40);
			config.pairwise = PCI_NONE;
			
			config.wep_key0 = ui.wepKey0HexCheck->isChecked() ? ui.wepKey0Edit->text() : '\"' + ui.wepKey0Edit->text() + '\"';
			config.wep_key1 = ui.wepKey1HexCheck->isChecked() ? ui.wepKey1Edit->text() : '\"' + ui.wepKey1Edit->text() + '\"';
			config.wep_key2 = ui.wepKey2HexCheck->isChecked() ? ui.wepKey2Edit->text() : '\"' + ui.wepKey2Edit->text() + '\"';
			config.wep_key3 = ui.wepKey3HexCheck->isChecked() ? ui.wepKey3Edit->text() : '\"' + ui.wepKey3Edit->text() + '\"';
			
			if (ui.wepKey0Radio->isChecked())
				config.wep_tx_keyidx = 0;
			else if (ui.wepKey1Radio->isChecked())
				config.wep_tx_keyidx = 1;
			else if (ui.wepKey2Radio->isChecked())
				config.wep_tx_keyidx = 2;
			else if (ui.wepKey3Radio->isChecked())
				config.wep_tx_keyidx = 3;
		}
		else if (ui.encCombo->currentText() == "CCMP") {
			config.group = GCI_CCMP;
			config.pairwise = PCI_CCMP;
		}
		else if (ui.encCombo->currentText() == "TKIP") {
			config.group = GCI_TKIP;
			config.pairwise = PCI_TKIP;
		}
		else {
			config.wep_tx_keyidx = -1;
			config.pairwise = PCI_NONE;
		}
		
		switch (ui.keyManagementCombo->currentIndex()) {
		case 0:
			config.keyManagement = KM_NONE;
			break;
		case 1:
			config.keyManagement = KM_IEEE8021X;
			writeEAPConfig(config.eap_config);
			break;
		case 2:
			config.keyManagement = KM_WPA_PSK;
			
			if (ui.rsnCheck->isChecked())
				config.protocols = PROTO_WPA;
			else
				config.protocols = PROTO_RSN;
			
			if ((!ui.pskLeaveButton->isChecked()) && (!ui.pskEdit->text().length() == 0))
				config.psk = '\"' + ui.pskEdit->text() + '\"';
			
			break;
		case 3:
			config.keyManagement = KM_WPA_EAP;
			
			if (ui.rsnCheck->isChecked())
				config.protocols = PROTO_WPA;
			else
				config.protocols = PROTO_RSN;
			
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
		
		if (status.failures != NCF_NONE) {
			qDebug("general failures:");
			if (status.failures & ENCF_ALL)             qDebug("NCF_ALL");
			if (status.failures & NCF_SSID)             qDebug("NCF_SSID");
			if (status.failures & NCF_BSSID)            qDebug("NCF_BSSID");
			if (status.failures & NCF_DISABLED)         qDebug("NCF_DISABLED");
			if (status.failures & NCF_ID_STR)           qDebug("NCF_ID_STR");
			if (status.failures & NCF_SCAN_SSID)        qDebug("NCF_SCAN_SSID");
			if (status.failures & NCF_PRIORITY)         qDebug("NCF_PRIORITY");
			if (status.failures & NCF_MODE)             qDebug("NCF_MODE");
			if (status.failures & NCF_FREQ)             qDebug("NCF_FREQ");
			if (status.failures & NCF_PROTO)            qDebug("NCF_PROTO");
			if (status.failures & NCF_KEYMGMT)          qDebug("NCF_KEYMGMT");
			if (status.failures & NCF_AUTH_ALG)         qDebug("NCF_AUTH_ALG");
			if (status.failures & NCF_PAIRWISE)         qDebug("NCF_PAIRWISE");
			if (status.failures & NCF_GROUP)            qDebug("NCF_GROUP");
			if (status.failures & NCF_PSK)              qDebug("NCF_PSK");
			if (status.failures & NCF_EAPOL_FLAGS)      qDebug("NCF_EAPOL_FLAGS");
			if (status.failures & NCF_MIXED_CELL)       qDebug("NCF_MIXED_CELL");
			if (status.failures & NCF_PROA_KEY_CACHING) qDebug("NCF_PROA_KEY_CACHING");
			if (status.failures & NCF_WEP_KEY0)         qDebug("NCF_WEP_KEY0");
			if (status.failures & NCF_WEP_KEY1)         qDebug("NCF_WEP_KEY1");
			if (status.failures & NCF_WEP_KEY2)         qDebug("NCF_WEP_KEY2");
			if (status.failures & NCF_WEP_KEY3)         qDebug("NCF_WEP_KEY3");
			if (status.failures & NCF_WEP_KEY_IDX)      qDebug("NCF_WEP_KEY_IDX");
			if (status.failures & NCF_PEERKEY)          qDebug("NCF_PEERKEY");
			return;
		}
		
		if (status.eap_failures != ENCF_NONE) {
			qDebug("eap failures:");
			if (status.eap_failures & ENCF_ALL)                 qDebug("ENCF_ALL");
			if (status.eap_failures & ENCF_EAP)                 qDebug("ENCF_EAP");
			if (status.eap_failures & ENCF_IDENTITY)            qDebug("ENCF_IDENTITY");
			if (status.eap_failures & ENCF_ANON_IDENTITY)       qDebug("ENCF_ANON_IDENTITY");
			if (status.eap_failures & ENCF_PASSWD)              qDebug("ENCF_PASSWD");
			if (status.eap_failures & ENCF_CA_CERT)             qDebug("ENCF_CA_CERT");
			if (status.eap_failures & ENCF_CA_PATH)             qDebug("ENCF_CA_PATH");
			if (status.eap_failures & ENCF_CLIENT_CERT)         qDebug("ENCF_CLIENT_CERT");
			if (status.eap_failures & ENCF_PRIVATE_KEY)         qDebug("ENCF_PRIVATE_KEY");
			if (status.eap_failures & ENCF_PRIVATE_KEY_PASSWD)  qDebug("ENCF_PRIVATE_KEY_PASSWD");
			if (status.eap_failures & ENCF_DH_FILE)             qDebug("ENCF_DH_FILE");
			if (status.eap_failures & ENCF_SUBJECT_MATCH)       qDebug("ENCF_SUBJECT_MATCH");
			if (status.eap_failures & ENCF_ALTSUBJECT_MATCH)    qDebug("ENCF_ALTSUBJECT_MATCH");
			if (status.eap_failures & ENCF_PHASE1)              qDebug("ENCF_PHASE1");
			if (status.eap_failures & ENCF_PHASE2)              qDebug("ENCF_PHASE2");
			if (status.eap_failures & ENCF_CA_CERT2)            qDebug("ENCF_CA_CERT2");
			if (status.eap_failures & ENCF_CA_PATH2)            qDebug("ENCF_CA_PATH2");
			if (status.eap_failures & ENCF_CLIENT_CERT2)        qDebug("ENCF_CLIENT_CERT2");
			if (status.eap_failures & ENCF_PRIVATE_KEY2)        qDebug("ENCF_PRIVATE_KEY2");
			if (status.eap_failures & ENCF_PRIVATE_KEY2_PASSWD) qDebug("ENCF_PRIVATE_KEY2_PASSWD");
			if (status.eap_failures & ENCF_DH_FILE2)            qDebug("ENCF_DH_FILE2");
			if (status.eap_failures & ENCF_SUBJECT_MATCH2)      qDebug("ENCF_SUBJECT_MATCH2");
			if (status.eap_failures & ENCF_ALTSUBJECT_MATCH2)   qDebug("ENCF_ALTSUBJECT_MATCH2");
			if (status.eap_failures & ENCF_FRAGMENT_SIZE)       qDebug("ENCF_FRAGMENT_SIZE");
			if (status.eap_failures & ENCF_EAPPSK)              qDebug("ENCF_EAPPSK");
			if (status.eap_failures & ENCF_NAI)                 qDebug("ENCF_NAI");
			if (status.eap_failures & ENCF_PAC_FILE)            qDebug("ENCF_PAC_FILE");
			return;
		}
		
		accept();
	}
	
	bool CAccessPointConfig::execute() {
		ui.pskLeaveButton->setVisible(false);
		ui.pskLeaveButton->setChecked(false);
		ui.passwordLeaveButton->setVisible(false);
		ui.passwordLeaveButton->setChecked(false);
		
		setAuthConfig(0);
		currentID = -1;
		return exec();
	}
	
	bool CAccessPointConfig::execute(ScanResult scanResult) {
		ui.pskLeaveButton->setVisible(false);
		ui.pskLeaveButton->setChecked(false);
		ui.passwordLeaveButton->setVisible(false);
		ui.passwordLeaveButton->setChecked(false);
		
		if (scanResult.keyManagement & KM_WPA_EAP)
			ui.keyManagementCombo->setCurrentIndex(3);
		else if (scanResult.keyManagement & KM_WPA_PSK)
			ui.keyManagementCombo->setCurrentIndex(2);
		else if (scanResult.keyManagement & KM_IEEE8021X)
			ui.keyManagementCombo->setCurrentIndex(1);
		else
			ui.keyManagementCombo->setCurrentIndex(0);
		
		ui.pskLeaveButton->setVisible(false);
		ui.pskLeaveButton->setChecked(false);
		ui.rsnCheck->setChecked(scanResult.protocols & PROTO_RSN);
		
		if (
			(scanResult.ciphers & CI_WEP40) ||
			(scanResult.ciphers & CI_WEP104) ||
			(scanResult.ciphers & CI_CCMP)
		)
			ui.encCombo->setCurrentIndex(1);
		else
			ui.encCombo->setCurrentIndex(0);
		
		ui.ssidEdit->setText(scanResult.ssid);
		
		currentID = -1;
		
		return exec();
	}
	
	bool CAccessPointConfig::execute(int id) {
		NetworkConfig config = supplicant->getNetworkConfig(id);
		
		if (config.ssid[0] == '\"') {
			ui.ssidHexCheck->setChecked(false);
			ui.ssidEdit->setText(config.ssid.mid(1, config.ssid.length()-2));
		}
		else {
			ui.ssidHexCheck->setChecked(true);
			ui.ssidEdit->setText(config.ssid);
		}
		
		if (config.keyManagement & KM_WPA_EAP) {
			ui.keyManagementCombo->setCurrentIndex(3);
			readEAPConfig(config.eap_config);
			ui.pskLeaveButton->setVisible(false);
			ui.pskLeaveButton->setChecked(false);
			ui.passwordLeaveButton->setVisible(true);
			ui.passwordLeaveButton->setChecked(true);
		}
		else if (config.keyManagement & KM_WPA_PSK) {
			ui.keyManagementCombo->setCurrentIndex(2);
//			ui.pskEdit->setText("");
			ui.pskLeaveButton->setVisible(true);
			ui.pskLeaveButton->setChecked(true);
			ui.passwordLeaveButton->setVisible(false);
			ui.passwordLeaveButton->setChecked(false);
		}
		else if (config.keyManagement & KM_IEEE8021X) {
			ui.keyManagementCombo->setCurrentIndex(1);
			readEAPConfig(config.eap_config);
			ui.pskLeaveButton->setVisible(false);
			ui.pskLeaveButton->setChecked(false);
			ui.passwordLeaveButton->setVisible(true);
			ui.passwordLeaveButton->setChecked(true);
		}
		else {
			ui.keyManagementCombo->setCurrentIndex(0);
			ui.pskLeaveButton->setVisible(false);
			ui.pskLeaveButton->setChecked(false);
			ui.passwordLeaveButton->setVisible(false);
			ui.passwordLeaveButton->setChecked(false);
		}
		
		if (
			(config.group & CI_CCMP) ||
			((config.wep_tx_keyidx >= 0) && ((config.group & CI_WEP40) || (config.group & CI_WEP104)))
		) {
			ui.encCombo->setCurrentIndex(1);
			switch (config.wep_tx_keyidx) {
			default: ui.wepKey0Radio->setChecked(true); break;
			case 1:  ui.wepKey1Radio->setChecked(true); break;
			case 2:  ui.wepKey2Radio->setChecked(true); break;
			case 3:  ui.wepKey3Radio->setChecked(true); break;
			}
		}
		else
			ui.encCombo->setCurrentIndex(0);
		
		ui.rsnCheck->setChecked(config.protocols & PROTO_RSN);
		ui.autoEnableCheck->setChecked(config.disabled);
		
		currentID = id;
		
		return exec();
	}
	
	inline void CAccessPointConfig::writeEAPConfig(EapNetworkConfig &eap_config) {
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
		
		if (!ui.passwordLeaveButton->isChecked())
			eap_config.private_key_passwd = '\"' + ui.passwordEdit->text() + '\"';
	}
	
	inline QString CAccessPointConfig::convertQuoted(QString text) {
		if (text[0] == '\"')
			return text.mid(1, text.length()-2);
		else
			return text;
	}
	
	inline void CAccessPointConfig::readEAPConfig(EapNetworkConfig &eap_config) {
		if (eap_config.eap & EAPM_LEAP)
			ui.encCombo->setCurrentIndex(7);
		else if (eap_config.eap & EAPM_OTP)
			ui.encCombo->setCurrentIndex(6);
		else if (eap_config.eap & EAPM_GTC)
			ui.encCombo->setCurrentIndex(5);
		else if (eap_config.eap & EAPM_TTLS)
			ui.encCombo->setCurrentIndex(4);
		else if (eap_config.eap & EAPM_PEAP)
			ui.encCombo->setCurrentIndex(3);
		else if (eap_config.eap & EAPM_MSCHAPV2)
			ui.encCombo->setCurrentIndex(2);
		else if (eap_config.eap & EAPM_TLS)
			ui.encCombo->setCurrentIndex(1);
		else
			ui.encCombo->setCurrentIndex(0);
		
		ui.identificationEdit->setText(convertQuoted(eap_config.identity));
		ui.caEdit->setText(convertQuoted(eap_config.ca_cert));
		ui.clientEdit->setText(convertQuoted(eap_config.client_cert));
		ui.keyFileEdit->setText(convertQuoted(eap_config.private_key));
		ui.passwordEdit->setText("");//convertQuoted(eap_config.private_key_passwd));
	}
	
	inline void CAccessPointConfig::convertLineEditText(QLineEdit * lineEdit, bool hex) {
		if (hex) {
			lineEdit->setText(lineEdit->text().toAscii().toHex());
			lineEdit->setValidator(hexValidator);
		}
		else {
			lineEdit->setText(QByteArray::fromHex(lineEdit->text().toAscii()));
			lineEdit->setValidator(0);
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
/*		QFileDialog dialog(this);
		dialog.setFileMode(QFileDialog::ExistingFile);*/
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
