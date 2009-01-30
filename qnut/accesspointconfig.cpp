//
// C++ Implementation: accesspointconfig
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_NO_WIRELESS
#include <QFileDialog>
#include <QToolTip>
#include <QMessageBox>
#include "accesspointconfig.h"

#define FLAG_PREPARE_OUTPUT(a, b, c) if(a & c) b << #c;

namespace qnut {
	using namespace libnutcommon;
	using namespace libnutwireless;
	
	CAccessPointConfig::CAccessPointConfig(CWpaSupplicant * wpa_supplicant, QWidget *parent) : QDialog(parent) {
		m_Supplicant = wpa_supplicant;
		
		ui.setupUi(this);
		
		m_OldConfig.group = GCI_UNDEFINED;
		m_OldConfig.pairwise = PCI_UNDEFINED;
		m_OldConfig.protocols = PROTO_UNDEFINED;
		
		ui.pskLeaveButton->setVisible(false);
		ui.passwordLeaveButton->setVisible(false);
		ui.wep0LeaveButton->setVisible(false);
		ui.wep1LeaveButton->setVisible(false);
		ui.wep2LeaveButton->setVisible(false);
		ui.wep3LeaveButton->setVisible(false);
		
		QRegExp regexp("[0123456789abcdefABCDEF]*");
		m_HexValidator = new QRegExpValidator(regexp, this);
		
		connect(ui.ssidHexCheck, SIGNAL(toggled(bool)), this, SLOT(convertSSID(bool)));
		connect(ui.wep0HexCheck, SIGNAL(toggled(bool)), this, SLOT(convertWEPKey0(bool)));
		connect(ui.wep1HexCheck, SIGNAL(toggled(bool)), this, SLOT(convertWEPKey1(bool)));
		connect(ui.wep2HexCheck, SIGNAL(toggled(bool)), this, SLOT(convertWEPKey2(bool)));
		connect(ui.wep3HexCheck, SIGNAL(toggled(bool)), this, SLOT(convertWEPKey3(bool)));
		connect(ui.pskEdit, SIGNAL(textChanged(QString)), this, SLOT(countPskChars(QString)));
		connect(ui.showPlainPSKCheck, SIGNAL(toggled(bool)), this, SLOT(togglePlainPSK(bool)));
		connect(ui.keyManagementCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setAuthConfig(int)));
		connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(verifyConfiguration()));
		connect(ui.caBrowse, SIGNAL(clicked()), this, SLOT(selectCAFile()));
		connect(ui.clientBrowse, SIGNAL(clicked()), this, SLOT(selectClientFile()));
		connect(ui.keyBrowse, SIGNAL(clicked()), this, SLOT(selectKeyFile()));
		
		connect(ui.rsnCheck, SIGNAL(toggled(bool)), this, SLOT(setWEPDisabled(bool)));
		connect(ui.grpCipCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(setEncConfig(QString)));
		setAuthConfig(0);
	}
	
	CAccessPointConfig::~CAccessPointConfig() {
		delete m_HexValidator;
	}
	
	void CAccessPointConfig::setAuthConfig(int type) {
		ui.grpCipCombo->clear();
		ui.grpCipCombo->addItem("none");
		
		ui.confTabs->setTabEnabled(2, type == 2);
		ui.confTabs->setTabEnabled(1, type == 1);
		
		ui.prwCipCombo->setEnabled(type > 0);
		ui.rsnCheck->setEnabled(type > 0);
		
		if (type > 0) {
			m_WEPEnabled = !ui.rsnCheck->isChecked();
			if (m_WEPEnabled)
				ui.grpCipCombo->addItem("WEP");
			ui.grpCipCombo->addItem("TKIP");
			ui.grpCipCombo->addItem("CCMP");
		}
		else {
			m_WEPEnabled = true;
			ui.grpCipCombo->addItem("WEP");
		}
	}
	
	void CAccessPointConfig::setEncConfig(QString value) {
		ui.confTabs->setTabEnabled(3, value == "WEP");
	}
	
	void CAccessPointConfig::setWEPDisabled(bool value) {
		m_WEPEnabled = not value;
		if (value)
			ui.grpCipCombo->removeItem(1);
		else
			ui.grpCipCombo->insertItem(1, "WEP");
	}
	
	bool CAccessPointConfig::execute() {
		setAuthConfig(0);
		m_CurrentID = -1;
		return exec();
	}
	
	bool CAccessPointConfig::execute(ScanResult scanResult) {
		ui.ssidEdit->setText(scanResult.ssid);
		ui.bssidEdit->setText(scanResult.bssid.toString());
		
		if ((scanResult.keyManagement & KM_WPA_EAP) || (scanResult.keyManagement & KM_IEEE8021X))
			ui.keyManagementCombo->setCurrentIndex(2);
		else if (scanResult.keyManagement & KM_WPA_PSK)
			ui.keyManagementCombo->setCurrentIndex(1);
		else
			ui.keyManagementCombo->setCurrentIndex(0);
		
		if (ui.rsnCheck->isEnabled())
			ui.rsnCheck->setChecked(scanResult.protocols & PROTO_RSN);
		
		if (scanResult.pairwise & PCI_CCMP)
			ui.prwCipCombo->setCurrentIndex(2);
		else if (scanResult.pairwise & PCI_TKIP)
			ui.prwCipCombo->setCurrentIndex(1);
		else
			ui.prwCipCombo->setCurrentIndex(0);
		
		if (m_WEPEnabled) {
			if (ui.keyManagementCombo->currentIndex())  {
				if (scanResult.group & GCI_CCMP)
					ui.grpCipCombo->setCurrentIndex(3);
				else if (scanResult.group & GCI_TKIP)
					ui.grpCipCombo->setCurrentIndex(2);
			}
			else if (!(scanResult.keyManagement & KM_OFF) && ((scanResult.group & GCI_WEP104) || (scanResult.group & GCI_WEP40)))
				ui.grpCipCombo->setCurrentIndex(1);
			else
				ui.grpCipCombo->setCurrentIndex(0);
		}
		else {
			if (scanResult.group & GCI_CCMP)
				ui.grpCipCombo->setCurrentIndex(2);
			else if (scanResult.group & GCI_TKIP)
				ui.grpCipCombo->setCurrentIndex(1);
			else
				ui.grpCipCombo->setCurrentIndex(0);
		}
		
		m_CurrentID = -1;
		
		return exec();
	}
	
	bool CAccessPointConfig::execute(int id) {
		NetworkConfig config = m_Supplicant->getNetworkConfig(id);
		
		m_OldConfig.pairwise = config.pairwise;
		m_OldConfig.group = config.group;
		m_OldConfig.protocols = config.protocols;
		
		if (config.ssid[0] == '\"') {
			ui.ssidHexCheck->setChecked(false);
			ui.ssidEdit->setText(config.ssid.mid(1, config.ssid.length()-2));
		}
		else {
			ui.ssidHexCheck->setChecked(true);
			ui.ssidEdit->setText(config.ssid);
		}
		
		ui.anyBSSIDCheck->setChecked(config.bssid.zero());
		ui.bssidEdit->setText(config.bssid.toString());
		
		if ((config.keyManagement & KM_WPA_EAP) || (config.keyManagement & KM_IEEE8021X)) {
			ui.keyManagementCombo->setCurrentIndex(2);
			readEAPConfig(config.eap_config);
			ui.passwordLeaveButton->setVisible(true);
			ui.passwordLeaveButton->setChecked(true);
			ui.rsnCheck->setChecked(config.protocols & PROTO_RSN);
		}
		else if (config.keyManagement & KM_WPA_PSK) {
			ui.keyManagementCombo->setCurrentIndex(1);
			ui.pskLeaveButton->setVisible(true);
			ui.pskLeaveButton->setChecked(true);
			ui.rsnCheck->setChecked(config.protocols & PROTO_RSN);
		}
		else
			ui.keyManagementCombo->setCurrentIndex(0);
		
//		ui.rsnCheck->setChecked(config.protocols & PROTO_RSN);
		
		if (config.pairwise & PCI_CCMP)
			ui.prwCipCombo->setCurrentIndex(2);
		else if (config.pairwise & PCI_TKIP)
			ui.prwCipCombo->setCurrentIndex(1);
		else
			ui.prwCipCombo->setCurrentIndex(0);
		
		if (m_WEPEnabled) {
			if (ui.keyManagementCombo->currentIndex()) {
				if (config.group & GCI_CCMP)
					ui.grpCipCombo->setCurrentIndex(3);
				else if (config.group & GCI_TKIP)
					ui.grpCipCombo->setCurrentIndex(2);
			}
			else if (!(config.keyManagement & KM_OFF) && ((config.group & GCI_WEP104) || (config.group & GCI_WEP40)))
				ui.grpCipCombo->setCurrentIndex(1);
			else
				ui.grpCipCombo->setCurrentIndex(0);
		}
		else {
			if (config.group & GCI_CCMP)
				ui.grpCipCombo->setCurrentIndex(2);
			else if (config.group & GCI_TKIP)
				ui.grpCipCombo->setCurrentIndex(1);
			else
				ui.grpCipCombo->setCurrentIndex(0);
		}
		
		if (ui.confTabs->isTabEnabled(3)) {
			ui.wep0LeaveButton->setVisible(true);
			ui.wep1LeaveButton->setVisible(true);
			ui.wep2LeaveButton->setVisible(true);
			ui.wep3LeaveButton->setVisible(true);
			ui.wep0LeaveButton->setChecked(true);
			ui.wep1LeaveButton->setChecked(true);
			ui.wep2LeaveButton->setChecked(true);
			ui.wep3LeaveButton->setChecked(true);
			
			switch (config.wep_tx_keyidx) {
			default: ui.wep0Radio->setChecked(true); break;
			case 1:  ui.wep1Radio->setChecked(true); break;
			case 2:  ui.wep2Radio->setChecked(true); break;
			case 3:  ui.wep3Radio->setChecked(true); break;
			}
		}
		
		ui.scanCheck->setChecked(config.scan_ssid);
		ui.autoEnableCheck->setChecked(config.disabled);
		
		m_CurrentID = id;
		
		return exec();
	}
	
	void CAccessPointConfig::verifyConfiguration() {
		NetconfigStatus status;
		NetworkConfig config;
		
		if (!ui.ssidEdit->text().isEmpty())
			config.ssid = ui.ssidHexCheck->isChecked() ? ui.ssidEdit->text() : '\"' + ui.ssidEdit->text() + '\"';
		
		config.scan_ssid = toQOOL(ui.scanCheck->isChecked());
		config.disabled = toQOOL(!ui.autoEnableCheck->isChecked());
		
		if (ui.anyBSSIDCheck->isChecked())
			config.bssid.clear();
		else
			config.bssid = MacAddress(ui.bssidEdit->text());
		
		if (ui.confTabs->isTabEnabled(3)) {
			if (!(ui.wep0LeaveButton->isChecked() || ui.wep0Edit->text().isEmpty()))
				config.wep_key0 = ui.wep0HexCheck->isChecked() ?
					ui.wep0Edit->text() :
					'\"' + ui.wep0Edit->text() + '\"';
			if (!(ui.wep1LeaveButton->isChecked() || ui.wep1Edit->text().isEmpty()))
				config.wep_key1 = ui.wep1HexCheck->isChecked() ?
					ui.wep1Edit->text() :
					'\"' + ui.wep1Edit->text() + '\"';
			if (!(ui.wep2LeaveButton->isChecked() || ui.wep2Edit->text().isEmpty()))
				config.wep_key2 = ui.wep2HexCheck->isChecked() ?
					ui.wep2Edit->text() :
					'\"' + ui.wep2Edit->text() + '\"';
			if (!(ui.wep3LeaveButton->isChecked() || ui.wep3Edit->text().isEmpty()))
				config.wep_key3 = ui.wep3HexCheck->isChecked() ?
					ui.wep3Edit->text() :
					'\"' + ui.wep3Edit->text() + '\"';
			
			if (ui.wep0Radio->isChecked())
				config.wep_tx_keyidx = 0;
			else if (ui.wep1Radio->isChecked())
				config.wep_tx_keyidx = 1;
			else if (ui.wep2Radio->isChecked())
				config.wep_tx_keyidx = 2;
			else if (ui.wep3Radio->isChecked())
				config.wep_tx_keyidx = 3;
		}
		
		if (ui.grpCipCombo->currentText() == "TKIP")
			config.group = GCI_TKIP;
		else if (ui.grpCipCombo->currentText() == "CCMP")
			config.group = GCI_CCMP;
		else if (ui.grpCipCombo->currentText() == "WEP")
			config.group = (GroupCiphers)(GCI_WEP104 | GCI_WEP40);
		
		if (config.group & m_OldConfig.group) {
			config.group = m_OldConfig.group;
		}
		
		if (ui.prwCipCombo->isEnabled()) {
			switch (ui.prwCipCombo->currentIndex()) {
			case 0:
				config.pairwise = PCI_NONE; break;
			case 1:
				config.pairwise = PCI_TKIP; break;
			case 2:
				config.pairwise = PCI_CCMP; break;
			default:
				break;
			}
			
			if (config.pairwise & m_OldConfig.pairwise) {
				config.pairwise = m_OldConfig.pairwise;
			}
		}
		
		switch (ui.keyManagementCombo->currentIndex()) {
		case 0:
			config.keyManagement = KM_NONE;
			break;
		case 1:
			config.keyManagement = KM_WPA_PSK;
			
			if (ui.rsnCheck->isChecked())
				config.protocols = PROTO_RSN;
			else
				config.protocols = PROTO_WPA;
			
			if (config.protocols & m_OldConfig.protocols) {
				config.protocols = m_OldConfig.protocols;
			}
			
			if (!(ui.pskLeaveButton->isChecked() || ui.pskEdit->text().isEmpty()))
				config.psk = '\"' + ui.pskEdit->text() + '\"';
			
			break;
		case 2:
			if (ui.grpCipCombo->currentText() == "WEP")
				config.keyManagement = KM_IEEE8021X;
			else
				config.keyManagement = KM_WPA_EAP;
			
			if (ui.rsnCheck->isChecked())
				config.protocols = PROTO_RSN;
			else
				config.protocols = PROTO_WPA;
			
			if (config.protocols & m_OldConfig.protocols) {
				config.protocols = m_OldConfig.protocols;
			}
			
			writeEAPConfig(config.eap_config);
			break;
		default:
			break;
		}
		
		if (m_CurrentID > -1)
			status = m_Supplicant->editNetwork(m_CurrentID, config);
		else
			status = m_Supplicant->addNetwork(config);
		
		QStringList errormsg;
		
		if (status.failures != NCF_NONE) {
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_ALL)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_SSID)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_BSSID)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_DISABLED)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_ID_STR)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_SCAN_SSID)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_PRIORITY)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_MODE)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_FREQ)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_PROTO)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_KEYMGMT)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_AUTH_ALG)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_PAIRWISE)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_GROUP)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_PSK)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_EAPOL_FLAGS)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_MIXED_CELL)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_PROA_KEY_CACHING)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_WEP_KEY0)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_WEP_KEY1)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_WEP_KEY2)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_WEP_KEY3)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_WEP_KEY_IDX)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, NCF_PEERKEY)
		}
		
		if (status.eap_failures != ENCF_NONE) {
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_ALL)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_EAP)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_IDENTITY)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_ANON_IDENTITY)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_PASSWD)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_CA_CERT)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_CA_PATH)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_CLIENT_CERT)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_PRIVATE_KEY)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_PRIVATE_KEY_PASSWD)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_DH_FILE)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_SUBJECT_MATCH)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_ALTSUBJECT_MATCH)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_PHASE1)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_PHASE2)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_CA_CERT2)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_CA_PATH2)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_CLIENT_CERT2)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_PRIVATE_KEY2)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_PRIVATE_KEY2_PASSWD)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_DH_FILE2)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_SUBJECT_MATCH2)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_ALTSUBJECT_MATCH2)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_FRAGMENT_SIZE)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_EAPPSK)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_NAI)
			FLAG_PREPARE_OUTPUT(status.failures, errormsg, ENCF_PAC_FILE)
		}
		
		if (!errormsg.isEmpty()) {
			QString errors = errormsg.join(", ");
			QMessageBox::critical(this, tr("Error on applying settings"),
				tr("WPA supplicant reported the following errors:") + '\n' + errors);
			qDebug(errors.toAscii().data());
			return;
		}
		
		accept();
	}
	
	inline void CAccessPointConfig::writeEAPConfig(EapNetworkConfig &eap_config) {
		switch (ui.eapCombo->currentIndex()) {
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
			ui.eapCombo->setCurrentIndex(7);
		else if (eap_config.eap & EAPM_OTP)
			ui.eapCombo->setCurrentIndex(6);
		else if (eap_config.eap & EAPM_GTC)
			ui.eapCombo->setCurrentIndex(5);
		else if (eap_config.eap & EAPM_TTLS)
			ui.eapCombo->setCurrentIndex(4);
		else if (eap_config.eap & EAPM_PEAP)
			ui.eapCombo->setCurrentIndex(3);
		else if (eap_config.eap & EAPM_MSCHAPV2)
			ui.eapCombo->setCurrentIndex(2);
		else if (eap_config.eap & EAPM_TLS)
			ui.eapCombo->setCurrentIndex(1);
		else
			ui.eapCombo->setCurrentIndex(0);
		
		ui.identificationEdit->setText(convertQuoted(eap_config.identity));
		ui.caEdit->setText(convertQuoted(eap_config.ca_cert));
		ui.clientEdit->setText(convertQuoted(eap_config.client_cert));
		ui.keyFileEdit->setText(convertQuoted(eap_config.private_key));
		ui.passwordEdit->setText("");
	}
	
	inline void CAccessPointConfig::convertLineEditText(QLineEdit * lineEdit, bool hex) {
		if (hex) {
			lineEdit->setText(lineEdit->text().toAscii().toHex());
			lineEdit->setValidator(m_HexValidator);
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
		convertLineEditText(ui.wep0Edit, hex);
	}
	
	void CAccessPointConfig::convertWEPKey1(bool hex) {
		convertLineEditText(ui.wep1Edit, hex);
	}
	
	void CAccessPointConfig::convertWEPKey2(bool hex) {
		convertLineEditText(ui.wep2Edit, hex);
	}
	
	void CAccessPointConfig::convertWEPKey3(bool hex) {
		convertLineEditText(ui.wep3Edit, hex);
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
}
#endif
