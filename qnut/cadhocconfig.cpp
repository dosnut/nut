//
// C++ Implementation: adhocconfig
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_NO_WIRELESS
#include <QMessageBox>
#include <libnutwireless/cwireless.h>
#include <libnutwireless/conversion.h>
#include "cadhocconfig.h"
#include "cerrorcodeevaluator.h"

namespace qnut {
	using namespace libnutwireless;

	CAdhocConfig::CAdhocConfig(CWireless * supplicant, QWidget * parent) : CAbstractWifiNetConfigDialog(supplicant, parent) {
		ui.setupUi(this);
		
		quint32 chan;
		ui.channelCombo->addItem(tr("not set"), -1);
		foreach (quint32 i, m_WifiInterface->getHardware()->getSupportedFrequencies()) {
			chan = frequencyToChannel(i);
			ui.channelCombo->addItem(QString::number(chan), chan);
		}
		
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
		
		connect(ui.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked(bool)), this, SLOT(applyConfiguration()));
		connect(ui.buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked(bool)), this, SLOT(resetUi()));
		setAuthConfig(0);
		
		populateErrorCodeEvaluator();
	}
	
	void CAdhocConfig::setAuthConfig(int type) {
		ui.confTabs->setTabEnabled(2, type == 1);
		ui.confTabs->setTabEnabled(1, type == 0);
	}
	
	#define CHECK_FLAG(a, b) (((a) & (b)) ? (a) : (b))
	
	bool CAdhocConfig::applyConfiguration() {
		NetconfigStatus status;
		
		if (!ui.ssidEdit->text().isEmpty())
			m_Config.set_ssid(ui.ssidHexCheck->isChecked() ? ui.ssidEdit->text() : '\"' + ui.ssidEdit->text() + '\"');
		
		int selectedChan = ui.channelCombo->itemData(ui.channelCombo->currentIndex(), Qt::UserRole).toInt();
		m_Config.set_frequency(channelToFrequency(selectedChan));
		
		m_Config.set_mode(true);
		
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
		
		switch (ui.keyManagementCombo->currentIndex()) {
/*		case 0:
			m_Config.set_key_mgmt(CHECK_FLAG(m_OldConfig.get_key_mgmt(), KM_OFF));
			break;*/
		case 0:
			m_Config.set_key_mgmt(CHECK_FLAG(m_OldConfig.get_key_mgmt(), KM_NONE));
			break;
		case 1:
			m_Config.set_key_mgmt(CHECK_FLAG(m_OldConfig.get_key_mgmt(), KM_WPA_NONE));
			m_Config.set_proto(PROTO_WPA);
			
			m_Config.set_pairwise(PCI_NONE);
		default:
			break;
		}
		
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
			
			// eap failures are irrelevant here
			
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
	
	void CAdhocConfig::countPskChars(QString psk) {
		ui.charCountLabel->setText(tr("%1 chars").arg(psk.length()));
	}
	
	void CAdhocConfig::togglePlainPSK(bool show) {
		if (show)
			ui.pskEdit->setEchoMode(QLineEdit::Normal);
		else
			ui.pskEdit->setEchoMode(QLineEdit::Password);
	}
	
	void CAdhocConfig::populateUi() {
		ui.ssidHexCheck->setChecked(setTextAutoHex(ui.ssidEdit, m_Config.get_ssid()));
		
		if (m_Config.get_key_mgmt() & KM_WPA_NONE)
			ui.keyManagementCombo->setCurrentIndex(1);
		else
			ui.keyManagementCombo->setCurrentIndex(0);
		
		ui.grpCipGroup->setChecked(!(m_Config.get_group() & GCI_NONE));
		ui.grpCipWEP40Check->setChecked(m_Config.get_group() & GCI_WEP40);
		ui.grpCipWEP104Check->setChecked(m_Config.get_group() & GCI_WEP104);
		ui.grpCipTKIPCheck->setChecked(m_Config.get_group() & GCI_TKIP);
		ui.grpCipCCMPCheck->setChecked(m_Config.get_group() & GCI_CCMP);
		
		int channel = frequencyToChannel(m_Config.get_frequency());
		int channelIndex = m_WifiInterface->getHardware()->getSupportedChannels().indexOf(channel);
		ui.channelCombo->setCurrentIndex(channelIndex + 1);
		
		ui.pskLeaveButton->setVisible(true);
		ui.pskLeaveButton->setChecked(true);
		
		bool isGlobalConfigured = (m_CurrentID != -1) && (!m_Config.hasValidNetworkId());
		
		ui.pskLeaveButton->setVisible(isGlobalConfigured);
		
		ui.pskLeaveButton->setChecked(isGlobalConfigured);
		
		ui.wep0HexCheck->setChecked(setTextAutoHex(ui.wep0Edit, m_Config.get_wep_key0()));
		ui.wep1HexCheck->setChecked(setTextAutoHex(ui.wep1Edit, m_Config.get_wep_key1()));
		ui.wep2HexCheck->setChecked(setTextAutoHex(ui.wep2Edit, m_Config.get_wep_key2()));
		ui.wep3HexCheck->setChecked(setTextAutoHex(ui.wep3Edit, m_Config.get_wep_key3()));
		
		ui.wep0LeaveButton->setVisible(isGlobalConfigured);
		ui.wep1LeaveButton->setVisible(isGlobalConfigured);
		ui.wep2LeaveButton->setVisible(isGlobalConfigured);
		ui.wep3LeaveButton->setVisible(isGlobalConfigured);
		
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
	}
	
	inline void CAdhocConfig::populateErrorCodeEvaluator() {
		// register general error codes
		m_ErrorCodeEvaluator->registerErrorCode(NCF_SSID, "SSID");
		m_ErrorCodeEvaluator->registerErrorCode(NCF_KEYMGMT, tr("Key Management"));
		m_ErrorCodeEvaluator->registerErrorCode(NCF_GROUP, ui.grpCipGroup->title());
		m_ErrorCodeEvaluator->registerErrorCode(NCF_PSK, tr("Pre Shared Key"));
		m_ErrorCodeEvaluator->registerErrorCode(NCF_WEP_KEY0, tr("WEP key 0"));
		m_ErrorCodeEvaluator->registerErrorCode(NCF_WEP_KEY1, tr("WEP key 1"));
		m_ErrorCodeEvaluator->registerErrorCode(NCF_WEP_KEY2, tr("WEP key 2"));
		m_ErrorCodeEvaluator->registerErrorCode(NCF_WEP_KEY3, tr("WEP key 3"));
		m_ErrorCodeEvaluator->registerErrorCode(NCF_WEP_KEY_IDX, tr("selected WEP key index"));
		m_ErrorCodeEvaluator->registerErrorCode(NCF_FREQ, tr("Channel"));
		// not needed : NCF_PRIORITY NCF_PAIRWISE NCF_SCAN_SSID NCF_DISABLED NCF_PROA_KEY_CACHING NCF_PROTO NCF_BSSID NCF_MODE
	}
}
#endif
