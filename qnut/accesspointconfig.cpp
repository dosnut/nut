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
#include <libnutwireless/wpa_supplicant.h>
#include "accesspointconfig.h"
#include <QDebug>
#define FLAG_PREPARE_OUTPUT(a, b, c) if(a & c) b << #c;

namespace qnut {
	using namespace libnutcommon;
	using namespace libnutwireless;
	
	//todo Implement widget for lineedits with hexadecimal digit inputs instead of this ugly implementation
	inline bool setTextAutoHex(QLineEdit * target, QString text) {
		if (text[0] == '\"') {
			target->setText(text.mid(1, text.length()-2));
			return false;
		}
		else {
			target->setText(text);
			return true;
		}
	}
	
	CAccessPointConfig::CAccessPointConfig(CWpaSupplicant * wpa_supplicant, QWidget * parent) : QDialog(parent) {
		m_Supplicant = wpa_supplicant;
		
		ui.setupUi(this);
		
		m_OldConfig.group = GCI_UNDEFINED;
		m_OldConfig.pairwise = PCI_UNDEFINED;
		m_OldConfig.protocols = PROTO_UNDEFINED;
		
		ui.pskLeaveButton->setVisible(false);
		ui.eapPasswordLeaveButton->setVisible(false);
		ui.keyPasswordLeaveButton->setVisible(false);
		ui.eapPSKLeaveButton->setVisible(false);
		ui.wep0LeaveButton->setVisible(false);
		ui.wep1LeaveButton->setVisible(false);
		ui.wep2LeaveButton->setVisible(false);
		ui.wep3LeaveButton->setVisible(false);
		
		QRegExp regexp("[0123456789abcdefABCDEF]*");
		m_HexValidator = new QRegExpValidator(regexp, this);
		
		m_EAPPhaseButtons = new QButtonGroup(this);
		m_EAPPhaseButtons->addButton(ui.phase1Radio, 1);
		m_EAPPhaseButtons->addButton(ui.phase2Radio, 2);
		connect(m_EAPPhaseButtons, SIGNAL(buttonPressed(int)), this, SLOT(setUiEAPPhase(int)));
		
		//prepare maps
		m_FileEditMapper = new QSignalMapper(this);
		FileEditStrings newFileEditStrings;
		connect(m_FileEditMapper, SIGNAL(mapped(QWidget*)), this, SLOT(selectFile(QWidget*)));
		
		newFileEditStrings.title = tr("Select Proxy Access Control (PAC) file");
		newFileEditStrings.filter = tr("Proxy Access Control (PAC) files (%1)").arg("*.pac");
		m_FileSelectStringMap.insert(ui.pacBrowse, newFileEditStrings);
		m_FileEditMapper->setMapping(ui.pacBrowse, ui.pacEdit);
		connect(ui.pacBrowse, SIGNAL(pressed()), m_FileEditMapper, SLOT(map()));
		
		newFileEditStrings.title = tr("Select CA certificate file");
		newFileEditStrings.filter = tr("Certificate files (%1)").arg("*.pem");
		m_FileSelectStringMap.insert(ui.caFileBrowse, newFileEditStrings);
		m_FileEditMapper->setMapping(ui.caFileBrowse, ui.caFileEdit);
		connect(ui.caFileBrowse, SIGNAL(pressed()), m_FileEditMapper, SLOT(map()));
		
		newFileEditStrings.title = tr("Select client certificate file");
		newFileEditStrings.filter = tr("Certificate files (%1)").arg("*.pem");
		m_FileSelectStringMap.insert(ui.clientFileBrowse, newFileEditStrings);
		m_FileEditMapper->setMapping(ui.clientFileBrowse, ui.clientFileEdit);
		connect(ui.clientFileBrowse, SIGNAL(pressed()), m_FileEditMapper, SLOT(map()));
		
		newFileEditStrings.title = tr("Select key file");
		newFileEditStrings.filter = tr("Key files (%1)").arg("*.pem");
		m_FileSelectStringMap.insert(ui.keyFileBrowse, newFileEditStrings);
		m_FileEditMapper->setMapping(ui.keyFileBrowse, ui.keyFileEdit);
		connect(ui.keyFileBrowse, SIGNAL(pressed()), m_FileEditMapper, SLOT(map()));
		
		newFileEditStrings.title = tr("Select DH/DSA file");
		newFileEditStrings.filter = tr("DH/DSA files (%1)").arg("*.pem");
		m_FileSelectStringMap.insert(ui.dhFileBrowse, newFileEditStrings);
		m_FileEditMapper->setMapping(ui.dhFileBrowse, ui.dhFileEdit);
		connect(ui.dhFileBrowse, SIGNAL(pressed()), m_FileEditMapper, SLOT(map()));
		
		m_HexEditMap.insert(ui.ssidHexCheck, ui.ssidEdit);
		m_HexEditMap.insert(ui.wep0HexCheck, ui.wep0Edit);
		m_HexEditMap.insert(ui.wep1HexCheck, ui.wep1Edit);
		m_HexEditMap.insert(ui.wep2HexCheck, ui.wep2Edit);
		m_HexEditMap.insert(ui.wep3HexCheck, ui.wep3Edit);
		connect(ui.ssidHexCheck, SIGNAL(toggled(bool)), this, SLOT(convertLineEditText(bool)));
		connect(ui.wep0HexCheck, SIGNAL(toggled(bool)), this, SLOT(convertLineEditText(bool)));
		connect(ui.wep1HexCheck, SIGNAL(toggled(bool)), this, SLOT(convertLineEditText(bool)));
		connect(ui.wep2HexCheck, SIGNAL(toggled(bool)), this, SLOT(convertLineEditText(bool)));
		connect(ui.wep3HexCheck, SIGNAL(toggled(bool)), this, SLOT(convertLineEditText(bool)));
		
		connect(ui.pskEdit, SIGNAL(textChanged(QString)), this, SLOT(countPskChars(QString)));
		connect(ui.showPlainPSKCheck, SIGNAL(toggled(bool)), this, SLOT(togglePlainPSK(bool)));
		connect(ui.keyManagementCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setAuthConfig(int)));
		connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(verifyConfiguration()));
		
		connect(ui.rsnCheck, SIGNAL(toggled(bool)), this, SLOT(setWEPDisabled(bool)));
		setAuthConfig(0);
	}
	
	CAccessPointConfig::~CAccessPointConfig() {
		delete m_HexValidator;
	}
	
	void CAccessPointConfig::setAuthConfig(int type) {
		ui.confTabs->setTabEnabled(4, type == 2);
		ui.confTabs->setTabEnabled(3, type == 2);
		ui.confTabs->setTabEnabled(2, type == 1);
		
		ui.prwCipGroup->setEnabled(type > 0);
		ui.rsnCheck->setEnabled(type > 0);
		
		m_WEPEnabled = (type == 0) || (!ui.rsnCheck->isChecked());
		
		ui.grpCipTKIPCheck->setEnabled(type > 0);
		ui.grpCipCCMPCheck->setEnabled(type > 0);
		ui.grpCipWEP40Check->setEnabled(m_WEPEnabled);
		ui.grpCipWEP104Check->setEnabled(m_WEPEnabled);
		
		ui.confTabs->setTabEnabled(1, m_WEPEnabled);
	}
	
	void CAccessPointConfig::setWEPDisabled(bool value) {
		m_WEPEnabled = not value;
		ui.grpCipWEP40Check->setEnabled(m_WEPEnabled);
		ui.grpCipWEP104Check->setEnabled(m_WEPEnabled);
		ui.confTabs->setTabEnabled(1, m_WEPEnabled);
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
		
		ui.prwCipCCMPCheck->setChecked(scanResult.pairwise & PCI_CCMP);
		ui.prwCipTKIPCheck->setChecked(scanResult.pairwise & PCI_TKIP);
		
		ui.grpCipWEP40Check->setChecked(scanResult.group & GCI_WEP40);
		ui.grpCipWEP104Check->setChecked(scanResult.group & GCI_WEP104);
		ui.grpCipTKIPCheck->setChecked(scanResult.group & GCI_TKIP);
		ui.grpCipCCMPCheck->setChecked(scanResult.group & GCI_CCMP);
		
		m_CurrentID = -1;
		
		return exec();
	}
	
	bool CAccessPointConfig::execute(int id) {
		m_Config = m_Supplicant->getNetworkConfig(id);
		
		m_OldConfig.pairwise = m_Config.get_pairwise();
		m_OldConfig.group = m_Config.get_group();
		m_OldConfig.protocols = m_Config.get_protocols();
		
		ui.ssidHexCheck->setChecked(setTextAutoHex(ui.ssidEdit, m_Config.get_ssid()));
		
		ui.anyBSSIDCheck->setChecked(m_Config.get_bssid().zero());
		if (!m_Config.get_bssid().zero())
			ui.bssidEdit->setText(m_Config.get_bssid().toString());
		
		if ((m_Config.get_key_mgmt() & KM_WPA_EAP) || (m_Config.get_key_mgmt() & KM_IEEE8021X)) {
			ui.keyManagementCombo->setCurrentIndex(2);
			readEAPConfig(m_Config);
			ui.rsnCheck->setChecked(m_Config.get_protocols() & PROTO_RSN);
		}
		else if (m_Config.get_key_mgmt() & KM_WPA_PSK) {
			ui.keyManagementCombo->setCurrentIndex(1);
			ui.pskLeaveButton->setVisible(true);
			ui.pskLeaveButton->setChecked(true);
			ui.rsnCheck->setChecked(m_Config.get_protocols() & PROTO_RSN);
		}
		else
			ui.keyManagementCombo->setCurrentIndex(0);
		
		ui.prwCipGroup->setChecked(!(m_Config.get_pairwise() & PCI_NONE));
		ui.prwCipCCMPCheck->setChecked(m_Config.get_pairwise() & PCI_CCMP);
		ui.prwCipTKIPCheck->setChecked(m_Config.get_pairwise() & PCI_TKIP);
		
		ui.grpCipGroup->setChecked(!(m_Config.get_group() & GCI_NONE));
		ui.grpCipWEP40Check->setChecked(m_Config.get_group() & GCI_WEP40);
		ui.grpCipWEP104Check->setChecked(m_Config.get_group() & GCI_WEP104);
		ui.grpCipTKIPCheck->setChecked(m_Config.get_group() & GCI_TKIP);
		ui.grpCipCCMPCheck->setChecked(m_Config.get_group() & GCI_CCMP);
		
		ui.proativeCheck->setChecked(toBool(m_Config.get_proactive_key_caching()));
		
		ui.wep0LeaveButton->setVisible(true);
		ui.wep1LeaveButton->setVisible(true);
		ui.wep2LeaveButton->setVisible(true);
		ui.wep3LeaveButton->setVisible(true);
		ui.wep0LeaveButton->setChecked(true);
		ui.wep1LeaveButton->setChecked(true);
		ui.wep2LeaveButton->setChecked(true);
		ui.wep3LeaveButton->setChecked(true);
		
		switch (m_Config.get_wep_tx_keyidx()) {
		default: ui.wep0Radio->setChecked(true); break;
		case 1:  ui.wep1Radio->setChecked(true); break;
		case 2:  ui.wep2Radio->setChecked(true); break;
		case 3:  ui.wep3Radio->setChecked(true); break;
		}
		
		ui.scanCheck->setChecked(m_Config.get_scan_ssid());
		ui.autoEnableCheck->setChecked(m_Config.get_disabled());
		
		m_CurrentID = id;
		
		return exec();
	}
	
	void CAccessPointConfig::verifyConfiguration() {
		NetconfigStatus status;
// 		CNetworkConfig config;
		
		if (!ui.ssidEdit->text().isEmpty())
			m_Config.set_ssid(ui.ssidHexCheck->isChecked() ? ui.ssidEdit->text() : '\"' + ui.ssidEdit->text() + '\"');
		
		m_Config.set_scan_ssid(toQOOL(ui.scanCheck->isChecked()));
		m_Config.set_disabled(toQOOL(!ui.autoEnableCheck->isChecked()));
		
		if (ui.anyBSSIDCheck->isChecked())
			m_Config.set_bssid(MacAddress());
		else
			m_Config.set_bssid(MacAddress(ui.bssidEdit->text()));
		
		if (ui.confTabs->isTabEnabled(3)) {
			if (!(ui.wep0LeaveButton->isChecked() || ui.wep0Edit->text().isEmpty()))
				m_Config.set_wep_key0(ui.wep0Edit->text(), !ui.wep0HexCheck->isChecked());
			if (!(ui.wep1LeaveButton->isChecked() || ui.wep1Edit->text().isEmpty()))
				m_Config.set_wep_key1(ui.wep1Edit->text(), !ui.wep1HexCheck->isChecked());
			if (!(ui.wep2LeaveButton->isChecked() || ui.wep2Edit->text().isEmpty()))
				m_Config.set_wep_key2(ui.wep2Edit->text(), !ui.wep2HexCheck->isChecked());
			if (!(ui.wep3LeaveButton->isChecked() || ui.wep3Edit->text().isEmpty()))
				m_Config.set_wep_key3(ui.wep3Edit->text(), !ui.wep3HexCheck->isChecked());
			
			if (ui.wep0Radio->isChecked())
				m_Config.set_wep_tx_keyidx(0);
			else if (ui.wep1Radio->isChecked())
				m_Config.set_wep_tx_keyidx(1);
			else if (ui.wep2Radio->isChecked())
				m_Config.set_wep_tx_keyidx(2);
			else if (ui.wep3Radio->isChecked())
				m_Config.set_wep_tx_keyidx(3);
		}
		
		// group ciphers
		if (ui.grpCipGroup->isChecked()) {
			m_Config.set_group(GroupCiphers(
				(ui.grpCipWEP40Check->isChecked() ? GCI_WEP40 : 0) |
				(ui.grpCipWEP104Check->isChecked() ? GCI_WEP104 : 0) |
				(ui.grpCipCCMPCheck->isChecked() ? GCI_CCMP : 0) |
				(ui.grpCipTKIPCheck->isChecked() ? GCI_TKIP : 0))
			);
		}
		else
			m_Config.set_group(GCI_NONE);
		
		if (m_Config.get_group() & m_OldConfig.group) {
			m_Config.set_group(m_OldConfig.group);
		}
		
		// pairwise ciphers
		if (ui.prwCipGroup->isChecked()) {
			m_Config.set_pairwise(PairwiseCiphers(
				(ui.prwCipCCMPCheck->isChecked() ? PCI_CCMP : 0) |
				(ui.prwCipTKIPCheck->isChecked() ? PCI_TKIP : 0))
			);
		}
		else
			m_Config.set_pairwise(PCI_NONE);
		
		if (m_Config.get_pairwise() & m_OldConfig.pairwise) {
			m_Config.set_pairwise(m_OldConfig.pairwise);
		}
		
		
		switch (ui.keyManagementCombo->currentIndex()) {
		case 0:
			m_Config.set_key_mgmt(KM_NONE);
			break;
		case 1:
			m_Config.set_key_mgmt(KM_WPA_PSK);
			
			if (ui.rsnCheck->isChecked())
				m_Config.set_proto(PROTO_RSN);
			else
				m_Config.set_proto(PROTO_WPA);
			
			if (m_Config.get_protocols() & m_OldConfig.protocols) {
				m_Config.set_proto(m_OldConfig.protocols);
			}
			
			if (!(ui.pskLeaveButton->isChecked() || ui.pskEdit->text().isEmpty()))
				m_Config.set_psk('\"' + ui.pskEdit->text() + '\"');
			
			break;
		case 2:
			if (m_Config.get_group() & (GCI_WEP40 | GCI_WEP104))
				m_Config.set_key_mgmt(KM_IEEE8021X);
			else
				m_Config.set_key_mgmt(KM_WPA_EAP);
			
			if (ui.rsnCheck->isChecked())
				m_Config.set_proto(PROTO_RSN);
			else
				m_Config.set_proto(PROTO_WPA);
			
			if (m_Config.get_protocols() & m_OldConfig.protocols) {
				m_Config.set_proto(m_OldConfig.protocols);
			}
			
			writeEAPConfig(m_Config);
			break;
		default:
			break;
		}
		
		m_Config.set_proactive_key_caching(ui.proativeCheck->isChecked());
		
		if (m_CurrentID == -1)
			status = m_Supplicant->addNetwork(m_Config);
		else
			status = m_Supplicant->editNetwork(m_CurrentID, m_Config);
		
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
	
	inline void CAccessPointConfig::writeEAPPhaseConfig(CNetworkConfig & eap_config, int phase) {
		if (phase == 2) {
			eap_config.set_phase2           (ui.phaseParamEdit->text(), true);
			eap_config.set_dh_file2         (ui.dhFileEdit->text(), true);
			eap_config.set_ca_cert2         (ui.caFileEdit->text(), true);
			eap_config.set_client_cert2     (ui.clientFileEdit->text(), true);
			eap_config.set_private_key2     (ui.keyFileEdit->text(), true);
			eap_config.set_subject_match2   (ui.subjectMatchEdit->text(), true);
			eap_config.set_altsubject_match2(ui.altSubjectMatchEdit->text(), true);
			
			if (!ui.keyPasswordLeaveButton->isChecked())
				eap_config.set_private_key2_passwd(ui.keyPasswordEdit->text(), true);
		}
		else {
			eap_config.set_phase1          (ui.phaseParamEdit->text(), true);
			eap_config.set_dh_file         (ui.dhFileEdit->text(), true);
			eap_config.set_ca_cert         (ui.caFileEdit->text(), true);
			eap_config.set_client_cert     (ui.clientFileEdit->text(), true);
			eap_config.set_private_key     (ui.keyFileEdit->text(), true);
			eap_config.set_subject_match   (ui.subjectMatchEdit->text(), true);
			eap_config.set_altsubject_match(ui.altSubjectMatchEdit->text(), true);
			
			if (!ui.keyPasswordLeaveButton->isChecked())
				eap_config.set_private_key_passwd(ui.keyPasswordEdit->text(), true);
		}
	}
	
	inline void CAccessPointConfig::writeEAPConfig(CNetworkConfig & eap_config) {
		ui.eapMethodAKACheck->setChecked     (eap_config.get_eap() & EAPM_AKA);
		ui.eapMethodFASTCheck->setChecked    (eap_config.get_eap() & EAPM_FAST);
		ui.eapMethodGPSKCheck->setChecked    (eap_config.get_eap() & EAPM_GPSK);
		ui.eapMethodGTCCheck->setChecked     (eap_config.get_eap() & EAPM_GTC);
		
		ui.eapMethodLEAPCheck->setChecked    (eap_config.get_eap() & EAPM_LEAP);
		ui.eapMethodMD5Check->setChecked     (eap_config.get_eap() & EAPM_MD5);
		ui.eapMethodMSCHAPV2Check->setChecked(eap_config.get_eap() & EAPM_MSCHAPV2);
		ui.eapMethodOTPCheck->setChecked     (eap_config.get_eap() & EAPM_OTP);
		
		ui.eapMethodPAXCheck->setChecked     (eap_config.get_eap() & EAPM_PAX);
		ui.eapMethodPEAPCheck->setChecked    (eap_config.get_eap() & EAPM_PEAP);
		ui.eapMethodPSKCheck->setChecked     (eap_config.get_eap() & EAPM_PSK);
		ui.eapMethodSAKECheck->setChecked    (eap_config.get_eap() & EAPM_SAKE);
		
		ui.eapMethodTLSCheck->setChecked     (eap_config.get_eap() & EAPM_TLS);
		ui.eapMethodTTLSCheck->setChecked    (eap_config.get_eap() & EAPM_TTLS);
		
		eap_config.set_eap(EapMethod(
			(ui.eapMethodAKACheck->isChecked() ? EAPM_AKA : 0) |
			(ui.eapMethodFASTCheck->isChecked() ? EAPM_FAST : 0) |
			(ui.eapMethodGPSKCheck->isChecked() ? EAPM_GPSK : 0) |
			(ui.eapMethodGTCCheck->isChecked() ? EAPM_GTC : 0) |
			
			(ui.eapMethodLEAPCheck->isChecked() ? EAPM_LEAP : 0) |
			(ui.eapMethodMD5Check->isChecked() ? EAPM_MD5 : 0) |
			(ui.eapMethodMSCHAPV2Check->isChecked() ? EAPM_MSCHAPV2 : 0) |
			(ui.eapMethodOTPCheck->isChecked() ? EAPM_OTP : 0) |
			
			(ui.eapMethodPAXCheck->isChecked() ? EAPM_PAX : 0) |
			(ui.eapMethodPEAPCheck->isChecked() ? EAPM_PEAP : 0) |
			(ui.eapMethodPSKCheck->isChecked() ? EAPM_PSK : 0) |
			(ui.eapMethodSAKECheck->isChecked() ? EAPM_SAKE : 0) |
			
			(ui.eapMethodTLSCheck->isChecked() ? EAPM_TLS : 0) |
			(ui.eapMethodTTLSCheck->isChecked() ? EAPM_TTLS : 0)
		));
		
		eap_config.set_identity(ui.identiyEdit->text(), true);
		eap_config.set_anonymous_identity(ui.anonymousIdentityEdit->text(), true);
		if (!ui.eapPasswordLeaveButton->isChecked())
			eap_config.set_password(ui.eapPasswordEdit->text(), true);
		
		eap_config.set_fragment_size(ui.fragmentSpin->value());
		
		eap_config.set_nai(ui.naiEdit->text(), false);
		eap_config.set_pac_file(ui.pacEdit->text(), false);
		
		writeEAPPhaseConfig(eap_config, ui.phase1Radio->isChecked() ? 1 : 2);
	}
	
	inline void CAccessPointConfig::readEAPPhaseConfig(CNetworkConfig & eap_config, int phase) {
		if (phase == 2) {
			setTextAutoHex(ui.phaseParamEdit,      eap_config.get_phase2());
			setTextAutoHex(ui.dhFileEdit,          eap_config.get_dh_file2());
			setTextAutoHex(ui.caFileEdit,          eap_config.get_ca_cert2());
			setTextAutoHex(ui.clientFileEdit,      eap_config.get_client_cert2());
			setTextAutoHex(ui.keyFileEdit,         eap_config.get_private_key2());
			setTextAutoHex(ui.subjectMatchEdit,    eap_config.get_subject_match2());
			setTextAutoHex(ui.altSubjectMatchEdit, eap_config.get_altsubject_match2());
			
			ui.keyPasswordLeaveButton->setVisible(eap_config.get_private_key2_passwd().isEmpty());
			ui.keyPasswordLeaveButton->setChecked(ui.keyPasswordLeaveButton->isVisible());
			setTextAutoHex(ui.keyPasswordEdit, eap_config.get_private_key2_passwd());
		}
		else {
			setTextAutoHex(ui.phaseParamEdit,      eap_config.get_phase1());
			setTextAutoHex(ui.dhFileEdit,          eap_config.get_dh_file());
			setTextAutoHex(ui.caFileEdit,          eap_config.get_ca_cert());
			setTextAutoHex(ui.clientFileEdit,      eap_config.get_client_cert());
			setTextAutoHex(ui.keyFileEdit,         eap_config.get_private_key());
			setTextAutoHex(ui.subjectMatchEdit,    eap_config.get_subject_match());
			setTextAutoHex(ui.altSubjectMatchEdit, eap_config.get_altsubject_match());
			
			ui.keyPasswordLeaveButton->setVisible(eap_config.get_private_key_passwd().isEmpty());
			ui.keyPasswordLeaveButton->setChecked(ui.keyPasswordLeaveButton->isVisible());
			setTextAutoHex(ui.keyPasswordEdit, eap_config.get_private_key_passwd());
		}
	}
	
	inline void CAccessPointConfig::readEAPConfig(CNetworkConfig & eap_config) {
		ui.eapMethodAKACheck->setChecked     (eap_config.get_eap() & EAPM_AKA);
		ui.eapMethodFASTCheck->setChecked    (eap_config.get_eap() & EAPM_FAST);
		ui.eapMethodGPSKCheck->setChecked    (eap_config.get_eap() & EAPM_GPSK);
		ui.eapMethodGTCCheck->setChecked     (eap_config.get_eap() & EAPM_GTC);
		ui.eapMethodLEAPCheck->setChecked    (eap_config.get_eap() & EAPM_LEAP);
		ui.eapMethodMD5Check->setChecked     (eap_config.get_eap() & EAPM_MD5);
		ui.eapMethodMSCHAPV2Check->setChecked(eap_config.get_eap() & EAPM_MSCHAPV2);
		ui.eapMethodOTPCheck->setChecked     (eap_config.get_eap() & EAPM_OTP);
		ui.eapMethodPAXCheck->setChecked     (eap_config.get_eap() & EAPM_PAX);
		ui.eapMethodPEAPCheck->setChecked    (eap_config.get_eap() & EAPM_PEAP);
		ui.eapMethodPSKCheck->setChecked     (eap_config.get_eap() & EAPM_PSK);
		ui.eapMethodSAKECheck->setChecked    (eap_config.get_eap() & EAPM_SAKE);
		ui.eapMethodTLSCheck->setChecked     (eap_config.get_eap() & EAPM_TLS);
		ui.eapMethodTTLSCheck->setChecked    (eap_config.get_eap() & EAPM_TTLS);
		
		setTextAutoHex(ui.identiyEdit, eap_config.get_identity());
		setTextAutoHex(ui.anonymousIdentityEdit, eap_config.get_anonymous_identity());
		
		ui.eapPasswordLeaveButton->setVisible(eap_config.get_password().isEmpty());
		ui.eapPasswordLeaveButton->setChecked(ui.eapPasswordLeaveButton->isVisible());
		setTextAutoHex(ui.eapPasswordEdit, eap_config.get_password());
		
		ui.eapPSKLeaveButton->setVisible(eap_config.get_eappsk().isEmpty());
		ui.eapPSKLeaveButton->setChecked(ui.eapPSKLeaveButton->isVisible());
		setTextAutoHex(ui.eapPSKEdit, eap_config.get_eappsk());
		
		ui.fragmentSpin->setValue(eap_config.get_fragment_size());
		
		setTextAutoHex(ui.naiEdit, eap_config.get_nai());
		setTextAutoHex(ui.pacEdit, eap_config.get_pac_file());
		
		readEAPPhaseConfig(eap_config, ui.phase1Radio->isChecked() ? 1 : 2);
	}
	
	inline void CAccessPointConfig::convertLineEditText(QLineEdit * lineEdit, bool hex) {
		if (hex) {
			lineEdit->setText(lineEdit->text().toAscii().toHex());
			lineEdit->setValidator(m_HexValidator);
		}
		else {
			lineEdit->setText(QByteArray::fromHex(lineEdit->text().toAscii()));
			lineEdit->setValidator(NULL);
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
	
	void CAccessPointConfig::convertLineEditText(bool hex) {
		QCheckBox * hexCheck = qobject_cast<QCheckBox *>(sender());
		if (hexCheck/* && m_HexEditMap.contains(hexCheck)*/)
			convertLineEditText(m_HexEditMap[hexCheck], hex);
	}
	
	void CAccessPointConfig::selectFile(QWidget * reciever) {
		QLineEdit * lineEdit = qobject_cast<QLineEdit *>(reciever);
		FileEditStrings strings = m_FileSelectStringMap[reciever];
		lineEdit->setText(QFileDialog::getOpenFileName(this, strings.title, "/", strings.filter));
	}
	
	void CAccessPointConfig::setUiEAPPhase(int phase) {
		writeEAPPhaseConfig(m_Config, phase == 1 ? 2 : 1);
		readEAPPhaseConfig(m_Config, phase);
	}
// 	
// 	void CAccessPointConfig::selectCAFile() {
// 		ui.caEdit->setText(QFileDialog::getOpenFileName(this,
// 			tr("Select CA certificate file"), "/", tr("Certificate files (%1)").arg("*.pem")));
// 	}
// 	
// 	void CAccessPointConfig::selectClientFile() {
// 		ui.clientEdit->setText(QFileDialog::getOpenFileName(this,
// 			tr("Select client certificate file"), "/", tr("Certificate files (%1)").arg("*.pem")));
// 	}
// 	
// 	void CAccessPointConfig::selectKeyFile() {
// 		ui.keyFileEdit->setText(QFileDialog::getOpenFileName(this,
// 			tr("Select key file"), "/", tr("Key files (%1)").arg("*.pem")));
// 	}
}
#endif
