//
// C++ Implementation: CAccessPointConfig
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef NUT_NO_WIRELESS
#include "caccesspointconfig.h"

#include <QFileDialog>
#include <QToolTip>
#include <QMessageBox>

#include <libnutwireless/cwireless.h>
#include <libnutwireless/wpa_supplicant.h>

#include "utils/cerrorcodeevaluator.h"

#define WRITE_BACK_AUTOQUOTE(f, t) f(t, !(t.isEmpty()))

namespace qnut {
	using namespace libnutcommon;
	using namespace libnutwireless;

	QString CAccessPointConfig::m_LastFileOpenDir = "/";

	//todo: Implement widget for lineedits with hexadecimal digit inputs instead of this ugly implementation
	inline bool setTextAutoHex(QLineEdit * target, QString text) {
		if (text.isEmpty()) {
			target->clear();
			return false;
		}

		if (text[0] == '\"') {
			target->setText(text.mid(1, text.length()-2));
			return false;
		}
		else {
			target->setText(text);
			return true;
		}
	}

	CAccessPointConfig::CAccessPointConfig(libnutwireless::CWireless * interface, QWidget * parent) : CAbstractWifiNetConfigDialog(interface, parent) {
		ui.setupUi(this);

		m_EapErrorCodeEvaluator = new CErrorCodeEvaluator();

		ui.eapPSKEdit->setValidator(m_HexValidator);

		m_EAPPhaseButtons = new QButtonGroup(this);
		m_EAPPhaseButtons->addButton(ui.phase1Radio, 1);
		m_EAPPhaseButtons->addButton(ui.phase2Radio, 2);
		connect(m_EAPPhaseButtons, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonPressed), this, &CAccessPointConfig::setUiEAPPhase);

		//prepare maps
		m_FileEditMapper = new QSignalMapper(this);
		FileEditStrings newFileEditStrings;
		connect(m_FileEditMapper, static_cast<void(QSignalMapper::*)(QWidget*)>(&QSignalMapper::mapped), this, &CAccessPointConfig::selectFile);

		newFileEditStrings.title = tr("Select Proxy Access Control (PAC) file");
		newFileEditStrings.filter = tr("Proxy Access Control (PAC) files (%1)").arg("*.pac");
		m_FileEditMapper->setMapping(ui.pacBrowse, ui.pacEdit);
		m_FileSelectStringMap.insert(ui.pacEdit, newFileEditStrings);
		connect(ui.pacBrowse, &QToolButton::pressed, m_FileEditMapper, static_cast<void(QSignalMapper::*)()>(&QSignalMapper::map));

		newFileEditStrings.title = tr("Select CA certificate file");
		newFileEditStrings.filter = tr("Certificate files (%1)").arg("*.pem");
		m_FileEditMapper->setMapping(ui.caFileBrowse, ui.caFileEdit);
		m_FileSelectStringMap.insert(ui.caFileEdit, newFileEditStrings);
		connect(ui.caFileBrowse, &QToolButton::pressed, m_FileEditMapper, static_cast<void(QSignalMapper::*)()>(&QSignalMapper::map));

		newFileEditStrings.title = tr("Select client certificate file");
		newFileEditStrings.filter = tr("Certificate files (%1)").arg("*.pem");
		m_FileEditMapper->setMapping(ui.clientFileBrowse, ui.clientFileEdit);
		m_FileSelectStringMap.insert(ui.clientFileEdit, newFileEditStrings);
		connect(ui.clientFileBrowse, &QToolButton::pressed, m_FileEditMapper, static_cast<void(QSignalMapper::*)()>(&QSignalMapper::map));

		newFileEditStrings.title = tr("Select key file");
		newFileEditStrings.filter = tr("Key files (%1)").arg("*.pem");
		m_FileEditMapper->setMapping(ui.keyFileBrowse, ui.keyFileEdit);
		m_FileSelectStringMap.insert(ui.keyFileEdit, newFileEditStrings);
		connect(ui.keyFileBrowse, &QToolButton::pressed, m_FileEditMapper, static_cast<void(QSignalMapper::*)()>(&QSignalMapper::map));

		newFileEditStrings.title = tr("Select DH/DSA file");
		newFileEditStrings.filter = tr("DH/DSA files (%1)").arg("*.pem");
		m_FileEditMapper->setMapping(ui.dhFileBrowse, ui.dhFileEdit);
		m_FileSelectStringMap.insert(ui.dhFileEdit, newFileEditStrings);
		connect(ui.dhFileBrowse, &QToolButton::pressed, m_FileEditMapper, static_cast<void(QSignalMapper::*)()>(&QSignalMapper::map));

		m_HexEditMap.insert(ui.ssidHexCheck, ui.ssidEdit);
		m_HexEditMap.insert(ui.wep0HexCheck, ui.wep0Edit);
		m_HexEditMap.insert(ui.wep1HexCheck, ui.wep1Edit);
		m_HexEditMap.insert(ui.wep2HexCheck, ui.wep2Edit);
		m_HexEditMap.insert(ui.wep3HexCheck, ui.wep3Edit);
		connect(ui.ssidHexCheck, &QCheckBox::toggled, this, &CAccessPointConfig::convertLineEditTextToggle);
		connect(ui.wep0HexCheck, &QCheckBox::toggled, this, &CAccessPointConfig::convertLineEditTextToggle);
		connect(ui.wep1HexCheck, &QCheckBox::toggled, this, &CAccessPointConfig::convertLineEditTextToggle);
		connect(ui.wep2HexCheck, &QCheckBox::toggled, this, &CAccessPointConfig::convertLineEditTextToggle);
		connect(ui.wep3HexCheck, &QCheckBox::toggled, this, &CAccessPointConfig::convertLineEditTextToggle);

		connect(ui.pskEdit, &QLineEdit::textChanged, this, &CAccessPointConfig::countPskChars);
		connect(ui.showPlainPSKCheck, &QCheckBox::toggled, this, &CAccessPointConfig::togglePlainPSK);
		connect(ui.keyManagementCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CAccessPointConfig::setAuthConfig);
		connect(ui.rsnCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CAccessPointConfig::handleRSNModeChanged);

		connect(ui.buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &CAccessPointConfig::applyConfiguration);
		connect(ui.buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &CAccessPointConfig::resetUi);
		setAuthConfig(0);

		populateErrorCodeEvaluator();
	}

	CAccessPointConfig::~CAccessPointConfig() {
		delete m_EapErrorCodeEvaluator;
	}

	void CAccessPointConfig::setAuthConfig(int type) {
		ui.confTabs->setTabEnabled(4, type >= 2);
		ui.confTabs->setTabEnabled(3, type >= 2);
		ui.confTabs->setTabEnabled(2, type == 1);

		ui.rsnCombo->setEnabled(type > 0 && type < 3);

		updateWEPState(type, ui.rsnCombo->currentIndex());
	}

	void CAccessPointConfig::handleRSNModeChanged(int value) {
		updateWEPState(ui.keyManagementCombo->currentIndex(), value);
	}

	inline void CAccessPointConfig::updateWEPState(int keyMode, int rsnMode) {
		bool wepDisabled = (keyMode == 1) || ((keyMode > 0) && (keyMode < 3) && (rsnMode > 0));

		ui.confTabs->setTabEnabled(1, !wepDisabled);
		ui.proativeCheck->setEnabled(wepDisabled );
	}

	#define CHECK_FLAG(a, b) (((a) & (b)) ? (a) : (b))

	bool CAccessPointConfig::applyConfiguration() {
		NetconfigStatus status;

		m_Config.set_mode(false);

		if (!ui.ssidEdit->text().isEmpty())
			m_Config.set_ssid(ui.ssidHexCheck->isChecked() ? ui.ssidEdit->text() : '\"' + ui.ssidEdit->text() + '\"');

		m_Config.set_scan_ssid(toQOOL(ui.scanCheck->isChecked()));

		m_Config.set_disabled(toQOOL(!ui.autoEnableCheck->isChecked()));
		m_Config.set_priority(ui.prioritySpin->value());

		if (ui.anyBSSIDCheck->isChecked())
			m_Config.set_bssid(MacAddress());
		else
			m_Config.set_bssid(MacAddress(ui.bssidEdit->text()));

		if (ui.confTabs->isTabEnabled(1)) {
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

		if (m_Config.get_group() & m_OldConfig.get_group()) {
			m_Config.set_group(m_OldConfig.get_group());
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

		if (m_Config.get_pairwise() & m_OldConfig.get_pairwise()) {
			m_Config.set_pairwise(m_OldConfig.get_pairwise());
		}

		switch (ui.keyManagementCombo->currentIndex()) {
		case 0:
			m_Config.set_key_mgmt(CHECK_FLAG(m_OldConfig.get_key_mgmt(), KM_NONE));
			break;
		case 1:
			m_Config.set_key_mgmt(CHECK_FLAG(m_OldConfig.get_key_mgmt(), KM_WPA_PSK));

			switch (ui.rsnCombo->currentIndex()) {
			case 0:
				m_Config.set_proto(CHECK_FLAG(m_OldConfig.get_protocols(), PROTO_WPA));
				break;
			case 1:
				m_Config.set_proto(CHECK_FLAG(m_OldConfig.get_protocols(), PROTO_RSN));
				break;
			case 2:
				m_Config.set_proto(PROTO_RSN);
				break;
			default:
				break;
			}

			if (!(ui.pskLeaveButton->isChecked() || ui.pskEdit->text().isEmpty()))
				m_Config.set_psk('\"' + ui.pskEdit->text() + '\"');
			break;
		case 2:
			m_Config.set_key_mgmt(CHECK_FLAG(m_OldConfig.get_key_mgmt(), KM_WPA_EAP));

			switch (ui.rsnCombo->currentIndex()) {
			case 0:
				m_Config.set_proto(CHECK_FLAG(m_OldConfig.get_protocols(), PROTO_WPA));
				break;
			case 1:
				m_Config.set_proto(CHECK_FLAG(m_OldConfig.get_protocols(), PROTO_RSN));
				break;
			case 2:
				m_Config.set_proto(PROTO_RSN);
				break;
			default:
				break;
			}

			writeEAPConfig(m_Config);
			break;
		case 3:
			m_Config.set_key_mgmt(CHECK_FLAG(m_OldConfig.get_key_mgmt(), KM_IEEE8021X));

			m_Config.set_proto(m_OldConfig.get_protocols());

			writeEAPConfig(m_Config);
			break;
		default:
			break;
		}

		m_Config.set_proactive_key_caching(ui.proativeCheck->isChecked());

		if (m_CurrentID == -1)
			status = m_WifiInterface->getWpaSupplicant()->addNetwork(m_Config);
		else {
			m_Config.setEqualsToUndefinded(m_OldConfig);
			status = m_WifiInterface->getWpaSupplicant()->editNetwork(m_CurrentID, m_Config);
		}

		if (status.failures || status.eap_failures) {
			unsigned long int errorCode;
			QString errormsg = tr("Please check the following settings:");

			// check for basic config errors
			if (status.failures) {
				errorCode = status.failures;
				m_ErrorCodeEvaluator->evaluate(errormsg, "\n - ", errorCode);
				status.failures = (libnutwireless::NetconfigFailures)(errorCode);
			}

			// check for eap config errors
			if (status.eap_failures) {
				errorCode = status.eap_failures;
				m_ErrorCodeEvaluator->evaluate(errormsg,  "\n - ", errorCode);
				status.eap_failures = (libnutwireless::EapNetconfigFailures)(errorCode);
			}

			// check for additional errors
			if (status.failures || status.eap_failures) {
				QStringList errors;

				if (!errormsg.isEmpty())
					errormsg += '\n';
				getConfigErrors(&status, errors);
				errormsg += tr("Additionally the following errors were reported:") + '\n' + errors.join(", ");
			}

			QMessageBox::critical(this, tr("Error on applying settings"), errormsg);
			return false;
		}

		m_CurrentID = status.id;
		return true;
	}

	inline void CAccessPointConfig::writeEAPPhaseConfig(CNetworkConfig & eap_config, int phase) {
		if (phase == 2) {
			WRITE_BACK_AUTOQUOTE(eap_config.set_phase2,              ui.phaseParamEdit->text());
			WRITE_BACK_AUTOQUOTE(eap_config.set_dh_file2,            ui.dhFileEdit->text());
			WRITE_BACK_AUTOQUOTE(eap_config.set_ca_cert2,            ui.caFileEdit->text());
			WRITE_BACK_AUTOQUOTE(eap_config.set_client_cert2,        ui.clientFileEdit->text());
			WRITE_BACK_AUTOQUOTE(eap_config.set_private_key2,        ui.keyFileEdit->text());
			WRITE_BACK_AUTOQUOTE(eap_config.set_subject_match2,      ui.subjectMatchEdit->text());
			WRITE_BACK_AUTOQUOTE(eap_config.set_altsubject_match2,   ui.altSubjectMatchEdit->text());
			WRITE_BACK_AUTOQUOTE(eap_config.set_private_key2_passwd, ui.keyPasswordEdit->text());
		}
		else {
			WRITE_BACK_AUTOQUOTE(eap_config.set_phase1,             ui.phaseParamEdit->text());
			WRITE_BACK_AUTOQUOTE(eap_config.set_dh_file,            ui.dhFileEdit->text());
			WRITE_BACK_AUTOQUOTE(eap_config.set_ca_cert,            ui.caFileEdit->text());
			WRITE_BACK_AUTOQUOTE(eap_config.set_client_cert,        ui.clientFileEdit->text());
			WRITE_BACK_AUTOQUOTE(eap_config.set_private_key,        ui.keyFileEdit->text());
			WRITE_BACK_AUTOQUOTE(eap_config.set_subject_match,      ui.subjectMatchEdit->text());
			WRITE_BACK_AUTOQUOTE(eap_config.set_altsubject_match,   ui.altSubjectMatchEdit->text());
			WRITE_BACK_AUTOQUOTE(eap_config.set_private_key_passwd, ui.keyPasswordEdit->text());
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

		WRITE_BACK_AUTOQUOTE(eap_config.set_identity, ui.identiyEdit->text());
		WRITE_BACK_AUTOQUOTE(eap_config.set_anonymous_identity, ui.anonymousIdentityEdit->text());
		WRITE_BACK_AUTOQUOTE(eap_config.set_password, ui.eapPasswordEdit->text());

		eap_config.set_fragment_size(ui.fragmentSpin->value());

		WRITE_BACK_AUTOQUOTE(eap_config.set_nai, ui.naiEdit->text());
		WRITE_BACK_AUTOQUOTE(eap_config.set_pac_file, ui.pacEdit->text());

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

		setTextAutoHex(ui.eapPasswordEdit, eap_config.get_password());

		setTextAutoHex(ui.eapPSKEdit, eap_config.get_eappsk());

		ui.fragmentSpin->setValue(eap_config.get_fragment_size());

		setTextAutoHex(ui.naiEdit, eap_config.get_nai());
		setTextAutoHex(ui.pacEdit, eap_config.get_pac_file());

		readEAPPhaseConfig(eap_config, ui.phase1Radio->isChecked() ? 1 : 2);
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

	void CAccessPointConfig::selectFile(QWidget * reciever) {
		QLineEdit * lineEdit = qobject_cast<QLineEdit *>(reciever);
		FileEditStrings strings = m_FileSelectStringMap[reciever];
		QFileDialog openDialog(this, strings.title, m_LastFileOpenDir, strings.filter);
		if (openDialog.exec()) {
			m_LastFileOpenDir = openDialog.directory().absolutePath();
			lineEdit->setText(openDialog.selectedFiles()[0]);
		}
	}

	void CAccessPointConfig::setUiEAPPhase(int phase) {
		writeEAPPhaseConfig(m_Config, phase == 1 ? 2 : 1);
		readEAPPhaseConfig(m_Config, phase);
	}

	void CAccessPointConfig::populateUi() {
		ui.ssidHexCheck->setChecked(setTextAutoHex(ui.ssidEdit, m_Config.get_ssid()));
		ui.scanCheck->setChecked(m_Config.get_scan_ssid());

		ui.anyBSSIDCheck->setChecked(m_Config.get_bssid().zero());
		if (!m_Config.get_bssid().zero())
			ui.bssidEdit->setText(m_Config.get_bssid().toString());

		if ((m_Config.get_key_mgmt() & KM_WPA_EAP) || (m_Config.get_key_mgmt() & KM_IEEE8021X)) {
			ui.keyManagementCombo->setCurrentIndex((m_Config.get_key_mgmt() & KM_IEEE8021X) ? 3 : 2);
			readEAPConfig(m_Config);

			if (m_Config.get_protocols() == PROTO_RSN)
				ui.rsnCombo->setCurrentIndex(2);
			else if (m_Config.get_protocols() & PROTO_RSN)
				ui.rsnCombo->setCurrentIndex(1);
			else
				ui.rsnCombo->setCurrentIndex(0);
		}
		else if (m_Config.get_key_mgmt() & KM_WPA_PSK) {
			ui.keyManagementCombo->setCurrentIndex(1);

			if (m_Config.get_protocols() == PROTO_RSN)
				ui.rsnCombo->setCurrentIndex(2);
			else if (m_Config.get_protocols() & PROTO_RSN)
				ui.rsnCombo->setCurrentIndex(1);
			else
				ui.rsnCombo->setCurrentIndex(0);
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

		bool isGlobalConfigured = (m_CurrentID != -1) && (!m_Config.hasValidNetworkId());

		ui.pskLeaveButton->setVisible(isGlobalConfigured);
		ui.eapPasswordLeaveButton->setVisible(isGlobalConfigured);
		ui.keyPasswordLeaveButton->setVisible(isGlobalConfigured);
		ui.eapPSKLeaveButton->setVisible(isGlobalConfigured);

		ui.wep0HexCheck->setChecked(setTextAutoHex(ui.wep0Edit, m_Config.get_wep_key0()));
		ui.wep1HexCheck->setChecked(setTextAutoHex(ui.wep1Edit, m_Config.get_wep_key1()));
		ui.wep2HexCheck->setChecked(setTextAutoHex(ui.wep2Edit, m_Config.get_wep_key2()));
		ui.wep3HexCheck->setChecked(setTextAutoHex(ui.wep3Edit, m_Config.get_wep_key3()));

		ui.wep0LeaveButton->setVisible(isGlobalConfigured);
		ui.wep1LeaveButton->setVisible(isGlobalConfigured);
		ui.wep2LeaveButton->setVisible(isGlobalConfigured);
		ui.wep3LeaveButton->setVisible(isGlobalConfigured);

		ui.pskLeaveButton->setChecked(isGlobalConfigured);
		ui.eapPasswordLeaveButton->setChecked(isGlobalConfigured);
		ui.keyPasswordLeaveButton->setChecked(isGlobalConfigured);
		ui.eapPSKLeaveButton->setChecked(isGlobalConfigured);

		ui.wep0LeaveButton->setChecked(isGlobalConfigured);
		ui.wep1LeaveButton->setChecked(isGlobalConfigured);
		ui.wep2LeaveButton->setChecked(isGlobalConfigured);
		ui.wep3LeaveButton->setChecked(isGlobalConfigured);

		switch (m_Config.get_wep_tx_keyidx()) {
		case 0: ui.wep0Radio->setChecked(true); break;
		case 1: ui.wep1Radio->setChecked(true); break;
		case 2: ui.wep2Radio->setChecked(true); break;
		case 3: ui.wep3Radio->setChecked(true); break;
		default: break;
		}

		ui.autoEnableCheck->setChecked(!m_Config.get_disabled());
		ui.prioritySpin->setValue(m_Config.get_priority());
	}

	inline void CAccessPointConfig::populateErrorCodeEvaluator() {
		// register general error codes
		m_ErrorCodeEvaluator->registerErrorCode(NCF_SSID, "SSID");
		m_ErrorCodeEvaluator->registerErrorCode(NCF_BSSID, "BSSID");
		m_ErrorCodeEvaluator->registerErrorCode(NCF_DISABLED, tr("Enable automatic selection"));
		m_ErrorCodeEvaluator->registerErrorCode(NCF_SCAN_SSID, tr("Enable SSID scanning (slower but needed if SSID is hidden)"));
		m_ErrorCodeEvaluator->registerErrorCode(NCF_PROTO, tr("WPA2 mode"));
		m_ErrorCodeEvaluator->registerErrorCode(NCF_KEYMGMT, tr("Key Management"));
		m_ErrorCodeEvaluator->registerErrorCode(NCF_PAIRWISE, tr("Pairwise algorithm (pairwise cipher)"));
		m_ErrorCodeEvaluator->registerErrorCode(NCF_GROUP, tr("General algorithm (group cipher)"));
		m_ErrorCodeEvaluator->registerErrorCode(NCF_PSK, tr("Pre Shared Key"));
		m_ErrorCodeEvaluator->registerErrorCode(NCF_PROA_KEY_CACHING, tr("Proactive Key Caching (for PMKSA)"));
		m_ErrorCodeEvaluator->registerErrorCode(NCF_WEP_KEY0, tr("WEP key 0"));
		m_ErrorCodeEvaluator->registerErrorCode(NCF_WEP_KEY1, tr("WEP key 1"));
		m_ErrorCodeEvaluator->registerErrorCode(NCF_WEP_KEY2, tr("WEP key 2"));
		m_ErrorCodeEvaluator->registerErrorCode(NCF_WEP_KEY3, tr("WEP key 3"));
		m_ErrorCodeEvaluator->registerErrorCode(NCF_WEP_KEY_IDX, tr("selected WEP key index"));
		// missing : NCF_PRIORITY
		// not needed : NCF_FREQ NCF_MODE

		// register eap error codes
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_EAP, tr("EAP method"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_IDENTITY, tr("Identity"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_ANON_IDENTITY, tr("Anonymous Identity"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_PASSWD, tr("Password"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_EAPPSK, tr("EAP Pre Shared Key"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_NAI, tr("Network Access Identifier"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_FRAGMENT_SIZE, tr("Fragment Size"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_PAC_FILE, tr("Proxy Access Control (PAC) File"));

		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_CA_CERT, tr("CA Certificate file in phase 1"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_CLIENT_CERT, tr("Client Certificate file in phase 1"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_PRIVATE_KEY, tr("Private Key file in phase 1"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_PRIVATE_KEY_PASSWD, tr("Private Key password in phase 1"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_PHASE1, tr("Phase parameters in phase 1"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_SUBJECT_MATCH, tr("Subject Match in phase 1"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_ALTSUBJECT_MATCH, tr("Alt. Subject Match in phase 1"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_DH_FILE, tr("DH/DSA file in phase 1"));

		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_CA_CERT2, tr("CA Certificate file in phase 1"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_CLIENT_CERT2, tr("Client Certificate file in phase 1"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_PRIVATE_KEY2, tr("Private Key file in phase 1"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_PRIVATE_KEY2_PASSWD, tr("Private Key password in phase 1"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_PHASE2, tr("Phase parameters in phase 1"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_SUBJECT_MATCH2, tr("Subject Match in phase 1"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_ALTSUBJECT_MATCH2, tr("Alt. Subject Match in phase 1"));
		m_EapErrorCodeEvaluator->registerErrorCode(ENCF_DH_FILE2, tr("DH/DSA file in phase 1"));
		// missing : ENCF_CA_PATH ENCF_CA_PATH2
	}
}
#endif
