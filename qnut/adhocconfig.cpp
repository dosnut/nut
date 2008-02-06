//
// C++ Implementation: adhocconfig
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_NO_WIRELESS
#include "adhocconfig.h"

namespace qnut {
	using namespace libnutwireless;

	CAdhocConfig::CAdhocConfig(CWpaSupplicant * wpa_supplicant, QWidget * parent) : QDialog(parent) {
		m_Supplicant = wpa_supplicant;
		
		ui.setupUi(this);
		
		foreach (quint8 i, m_Supplicant->getSupportedChannels()) {
			ui.channelCombo->addItem(QString::number(i));
		}
		
		QRegExp regexp("[0123456789abcdefABCDEF]*");
		m_HexValidator = new QRegExpValidator(regexp, this);
		
		connect(ui.ssidHexCheck, SIGNAL(toggled(bool)), this, SLOT(convertSSID(bool)));
		connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(verifyConfiguration()));
	}
	
	CAdhocConfig::~CAdhocConfig() {
		delete m_HexValidator;
	}
	
	void CAdhocConfig::verifyConfiguration() {
		NetconfigStatus status;
		NetworkConfig config;
		
		config.ssid = ui.ssidHexCheck->isChecked() ? ui.ssidEdit->text() : '\"' + ui.ssidEdit->text() + '\"';
		config.frequency = channelToFrequency(ui.channelCombo->currentText().toInt());
		config.mode = QOOL_TRUE;
		
		switch (ui.encCombo->currentIndex()) {
		case 0:
			config.keyManagement = KM_OFF;
			break;
		case 1:
			config.wep_tx_keyidx = 0;
			config.keyManagement = KM_NONE;
			
			if ((!ui.pskLeaveButton->isChecked()) && (ui.pskEdit->text().length() != 0))
				config.wep_key0 = '\"' + ui.pskEdit->text() + '\"';
			break;
		default:
			if (ui.encCombo->currentText() == "CCMP")
				config.group = GCI_CCMP;
			else if (ui.encCombo->currentText() == "TKIP")
				config.group = GCI_TKIP;
			
			config.protocols = PROTO_WPA;
			config.pairwise = PCI_NONE;
			config.keyManagement = KM_WPA_NONE;
			
			if ((!ui.pskLeaveButton->isChecked()) && (ui.pskEdit->text().length() != 0))
				config.psk = '\"' + ui.pskEdit->text() + '\"';
			break;
		}
		
		if (m_CurrentID > -1) {
			status = m_Supplicant->editNetwork(m_CurrentID, config);
		}
		else {
			status = m_Supplicant->addNetwork(config);
		}
		
		QStringList errormsg;
		
		if (status.failures != NCF_NONE) {
			qDebug("general failures:");
			if (status.failures & NCF_ALL)              { qDebug("NCF_ALL");              errormsg << "NCF_ALL"; }
			if (status.failures & NCF_SSID)             { qDebug("NCF_SSID");             errormsg << "NCF_SSID"; }
			if (status.failures & NCF_BSSID)            { qDebug("NCF_BSSID");            errormsg << "NCF_BSSID"; }
			if (status.failures & NCF_DISABLED)         { qDebug("NCF_DISABLED");         errormsg << "NCF_DISABLED"; }
			if (status.failures & NCF_ID_STR)           { qDebug("NCF_ID_STR");           errormsg << "NCF_ID_STR"; }
			if (status.failures & NCF_SCAN_SSID)        { qDebug("NCF_SCAN_SSID");        errormsg << "NCF_SCAN_SSID"; }
			if (status.failures & NCF_PRIORITY)         { qDebug("NCF_PRIORITY");         errormsg << "NCF_PRIORITY"; }
			if (status.failures & NCF_MODE)             { qDebug("NCF_MODE");             errormsg << "NCF_MODE"; }
			if (status.failures & NCF_FREQ)             { qDebug("NCF_FREQ");             errormsg << "NCF_FREQ"; }
			if (status.failures & NCF_PROTO)            { qDebug("NCF_PROTO");            errormsg << "NCF_PROTO"; }
			if (status.failures & NCF_KEYMGMT)          { qDebug("NCF_KEYMGMT");          errormsg << "NCF_KEYMGMT"; }
			if (status.failures & NCF_AUTH_ALG)         { qDebug("NCF_AUTH_ALG");         errormsg << "NCF_AUTH_ALG"; }
			if (status.failures & NCF_PAIRWISE)         { qDebug("NCF_PAIRWISE");         errormsg << "NCF_PAIRWISE"; }
			if (status.failures & NCF_GROUP)            { qDebug("NCF_GROUP");            errormsg << "NCF_GROUP"; }
			if (status.failures & NCF_PSK)              { qDebug("NCF_PSK");              errormsg << "NCF_PSK"; }
			if (status.failures & NCF_EAPOL_FLAGS)      { qDebug("NCF_EAPOL_FLAGS");      errormsg << "NCF_EAPOL_FLAGS"; }
			if (status.failures & NCF_MIXED_CELL)       { qDebug("NCF_MIXED_CELL");       errormsg << "NCF_MIXED_CELL"; }
			if (status.failures & NCF_PROA_KEY_CACHING) { qDebug("NCF_PROA_KEY_CACHING"); errormsg << "NCF_PROA_KEY_CACHING"; }
			if (status.failures & NCF_WEP_KEY0)         { qDebug("NCF_WEP_KEY0");         errormsg << "NCF_WEP_KEY0"; }
			if (status.failures & NCF_WEP_KEY1)         { qDebug("NCF_WEP_KEY1");         errormsg << "NCF_WEP_KEY1"; }
			if (status.failures & NCF_WEP_KEY2)         { qDebug("NCF_WEP_KEY2");         errormsg << "NCF_WEP_KEY2"; }
			if (status.failures & NCF_WEP_KEY3)         { qDebug("NCF_WEP_KEY3");         errormsg << "NCF_WEP_KEY3"; }
			if (status.failures & NCF_WEP_KEY_IDX)      { qDebug("NCF_WEP_KEY_IDX");      errormsg << "NCF_WEP_KEY_IDX"; }
			if (status.failures & NCF_PEERKEY)          { qDebug("NCF_PEERKEY");          errormsg << "NCF_PEERKEY"; }
		}
		
		if (status.eap_failures != ENCF_NONE) {
			qDebug("eap failures:");
			if (status.eap_failures & ENCF_ALL)                 { qDebug("ENCF_ALL");                 errormsg << "ENCF_ALL"; }
			if (status.eap_failures & ENCF_EAP)                 { qDebug("ENCF_EAP");                 errormsg << "ENCF_EAP"; }
			if (status.eap_failures & ENCF_IDENTITY)            { qDebug("ENCF_IDENTITY");            errormsg << "ENCF_IDENTITY"; }
			if (status.eap_failures & ENCF_ANON_IDENTITY)       { qDebug("ENCF_ANON_IDENTITY");       errormsg << "ENCF_ANON_IDENTITY"; }
			if (status.eap_failures & ENCF_PASSWD)              { qDebug("ENCF_PASSWD");              errormsg << "ENCF_PASSWD"; }
			if (status.eap_failures & ENCF_CA_CERT)             { qDebug("ENCF_CA_CERT");             errormsg << "ENCF_CA_CERT"; }
			if (status.eap_failures & ENCF_CA_PATH)             { qDebug("ENCF_CA_PATH");             errormsg << "ENCF_CA_PATH"; }
			if (status.eap_failures & ENCF_CLIENT_CERT)         { qDebug("ENCF_CLIENT_CERT");         errormsg << "ENCF_CLIENT_CERT"; }
			if (status.eap_failures & ENCF_PRIVATE_KEY)         { qDebug("ENCF_PRIVATE_KEY");         errormsg << "ENCF_PRIVATE_KEY"; }
			if (status.eap_failures & ENCF_PRIVATE_KEY_PASSWD)  { qDebug("ENCF_PRIVATE_KEY_PASSWD");  errormsg << "ENCF_PRIVATE_KEY_PASSWD"; }
			if (status.eap_failures & ENCF_DH_FILE)             { qDebug("ENCF_DH_FILE");             errormsg << "ENCF_DH_FILE"; }
			if (status.eap_failures & ENCF_SUBJECT_MATCH)       { qDebug("ENCF_SUBJECT_MATCH");       errormsg << "ENCF_SUBJECT_MATCH"; }
			if (status.eap_failures & ENCF_ALTSUBJECT_MATCH)    { qDebug("ENCF_ALTSUBJECT_MATCH");    errormsg << "ENCF_ALTSUBJECT_MATCH"; }
			if (status.eap_failures & ENCF_PHASE1)              { qDebug("ENCF_PHASE1");              errormsg << "ENCF_PHASE1"; }
			if (status.eap_failures & ENCF_PHASE2)              { qDebug("ENCF_PHASE2");              errormsg << "ENCF_PHASE2"; }
			if (status.eap_failures & ENCF_CA_CERT2)            { qDebug("ENCF_CA_CERT2");            errormsg << "ENCF_CA_CERT2"; }
			if (status.eap_failures & ENCF_CA_PATH2)            { qDebug("ENCF_CA_PATH2");            errormsg << "ENCF_CA_PATH2"; }
			if (status.eap_failures & ENCF_CLIENT_CERT2)        { qDebug("ENCF_CLIENT_CERT2");        errormsg << "ENCF_CLIENT_CERT2"; }
			if (status.eap_failures & ENCF_PRIVATE_KEY2)        { qDebug("ENCF_PRIVATE_KEY2");        errormsg << "ENCF_PRIVATE_KEY2"; }
			if (status.eap_failures & ENCF_PRIVATE_KEY2_PASSWD) { qDebug("ENCF_PRIVATE_KEY2_PASSWD"); errormsg << "ENCF_PRIVATE_KEY2_PASSWD"; }
			if (status.eap_failures & ENCF_DH_FILE2)            { qDebug("ENCF_DH_FILE2");            errormsg << "ENCF_DH_FILE2"; }
			if (status.eap_failures & ENCF_SUBJECT_MATCH2)      { qDebug("ENCF_SUBJECT_MATCH2");      errormsg << "ENCF_SUBJECT_MATCH2"; }
			if (status.eap_failures & ENCF_ALTSUBJECT_MATCH2)   { qDebug("ENCF_ALTSUBJECT_MATCH2");   errormsg << "ENCF_ALTSUBJECT_MATCH2"; }
			if (status.eap_failures & ENCF_FRAGMENT_SIZE)       { qDebug("ENCF_FRAGMENT_SIZE");       errormsg << "ENCF_FRAGMENT_SIZE"; }
			if (status.eap_failures & ENCF_EAPPSK)              { qDebug("ENCF_EAPPSK");              errormsg << "ENCF_EAPPSK"; }
			if (status.eap_failures & ENCF_NAI)                 { qDebug("ENCF_NAI");                 errormsg << "ENCF_NAI"; }
			if (status.eap_failures & ENCF_PAC_FILE)            { qDebug("ENCF_PAC_FILE");            errormsg << "ENCF_PAC_FILE"; }
		}
		
		if (!errormsg.isEmpty()) {
			QMessageBox::critical(this, tr("Error on adding ad-hoc network"),
				tr("WPA supplicant reported the following errors:") + '\n' + errormsg.join(", "));
			return;
		}
		
		accept();
	}
	
	bool CAdhocConfig::execute() {
		ui.pskLeaveButton->setVisible(false);
		ui.pskLeaveButton->setChecked(false);
		
		m_CurrentID = -1;
		return exec();
	}
	
	bool CAdhocConfig::execute(int id) {
		NetworkConfig config = m_Supplicant->getNetworkConfig(id);
		
		if (config.ssid[0] == '\"') {
			ui.ssidHexCheck->setChecked(false);
			ui.ssidEdit->setText(config.ssid.mid(1, config.ssid.length()-2));
		}
		else {
			ui.ssidHexCheck->setChecked(true);
			ui.ssidEdit->setText(config.ssid);
		}
		
		if (config.group & PCI_CCMP)
			ui.encCombo->setCurrentIndex(3);
		else if (config.group & PCI_TKIP)
			ui.encCombo->setCurrentIndex(2);
		else if (config.keyManagement & KM_NONE)
			ui.encCombo->setCurrentIndex(1);
		else
			ui.encCombo->setCurrentIndex(0);
		
		int channel = frequencyToChannel(config.frequency);
		int channelIndex = m_Supplicant->getSupportedChannels().indexOf(channel);
		ui.channelCombo->setCurrentIndex(channelIndex);
		
		ui.pskLeaveButton->setVisible(true);
		ui.pskLeaveButton->setChecked(true);
		
		m_CurrentID = id;
		
		return exec();
	}
	
	bool CAdhocConfig::execute(ScanResult scanResult) {
		ui.ssidEdit->setText(scanResult.ssid);
		
		int channel = frequencyToChannel(scanResult.freq);
		int channelIndex = m_Supplicant->getSupportedChannels().indexOf(channel);
		ui.channelCombo->setCurrentIndex(channelIndex);
		
		if (scanResult.group & PCI_CCMP)
			ui.encCombo->setCurrentIndex(3);
		else if (scanResult.group & PCI_TKIP)
			ui.encCombo->setCurrentIndex(2);
		else if (scanResult.keyManagement & KM_NONE)
			ui.encCombo->setCurrentIndex(1);
		else
			ui.encCombo->setCurrentIndex(0);
		
		m_CurrentID = -1;
		
		return exec();
	}
	
	void CAdhocConfig::convertSSID(bool hex) {
		if (hex) {
			ui.ssidEdit->setText(ui.ssidEdit->text().toAscii().toHex());
			ui.ssidEdit->setValidator(m_HexValidator);
		}
		else {
			ui.ssidEdit->setText(QByteArray::fromHex(ui.ssidEdit->text().toAscii()));
			ui.ssidEdit->setValidator(0);
		}
	}
}
#endif
