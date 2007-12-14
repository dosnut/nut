#include "wpa_supplicant.h"
#include <QDebug>

namespace libnutwireless {

void CWpaSupplicant::setApScanDefault() {
	if (-1 != m_apScanDefault) {
		return;
	}
	QList<ShortNetworkInfo> netlist = listNetworks();
	//Assume ap_scan=1 as default:
	m_apScanDefault = 1;
	printMessage("Auto-setting ap_scan=1");
	m_lastWasAdHoc = false;
	//Check if active network is ad-hoc network:
	foreach(ShortNetworkInfo i, netlist) {
		if (NF_CURRENT == i.flags) {
			if (i.adhoc) {
				m_lastWasAdHoc = true;
				printMessage("auto-setting last was adhoc to true");
			}
			break;
		}
	}
}

//CWpa_supplicant
QList<quint8>& CWpaSupplicant::getSupportedChannels() {
	//create m_supportedChannels?
	if (m_supportedChannels.isEmpty() && !m_supportedFrequencies.isEmpty()) {
		foreach(quint32 freq, m_supportedFrequencies) {
			m_supportedChannels.append(frequencyToChannel(freq));
		}
	}
	return m_supportedChannels;
}

//Function to respond to ctrl requests from wpa_supplicant
void CWpaSupplicant::response(Request request, QString msg) {
	QString cmd = toString(request.type);
	if (!cmd.isEmpty()) {
		wpaCtrlCmd_CTRL_RSP(cmd,request.id,msg);
	}
}

bool CWpaSupplicant::selectNetwork(int id) {
	//TODO: Check if we need to set ap_scan first

	QOOL hasMode = toQOOL(getNetworkVariable(id,"mode"));
	//Let's hope wpa_supplicant returns fail only when network id is invalid
	if (QOOL_UNDEFINED == hasMode) {
		return false;
	}
	else if (QOOL_TRUE == hasMode) {
		ap_scan(2);
		printMessage(tr("auto-setting ap_scan=2."));
	}
	else {
		//Check if we have defaults, and if the last network was an ap-network
		if (m_lastWasAdHoc) {
			m_lastWasAdHoc = false;
			if (-1 != m_apScanDefault) {
				ap_scan(m_apScanDefault);
				printMessage(tr("Using your last ap_scan settings for auto-setting: %1").arg(QString::number(m_apScanDefault)));
			}
			else {
				printMessage(tr("You must set ap_scan to your needs!"));

			}
		}
	}
	if ("OK\n" == wpaCtrlCmd_SELECT_NETWORK(id)) {
		emit networkListUpdated();
		return true;
	}
	else {
		//Reset ap_scan:
		if (m_lastWasAdHoc) {
			ap_scan(m_apScanDefault);
		}
		return false;
	}
}
bool CWpaSupplicant::enableNetwork(int id) {
	if ("OK\n" == wpaCtrlCmd_ENABLE_NETWORK(id)) {
		emit networkListUpdated();
		return true;
	}
	else {
		return false;
	}
}
bool CWpaSupplicant::disableNetwork(int id) {
	if ("OK\n" == wpaCtrlCmd_DISABLE_NETWORK(id)) {
		emit networkListUpdated();
		return true;
	}
	else {
		return false;
	}
}

bool CWpaSupplicant::ap_scan(int type) {
	if ( (0 <= type and 2 >= type) ) {
		if ("FAIL\n" == wpaCtrlCmd_AP_SCAN(type)) {
			return false;
		}
		else {
			printMessage(QString("Setting ap_scan=%1").arg(type));
			//ap_scan variables accordingly
			if (2 == type) {
				m_lastWasAdHoc = true;
			}
			else {
				m_lastWasAdHoc = false;
				m_apScanDefault = type;
			}
			return true;
		}
	}
	return false;
}
bool CWpaSupplicant::save_config() {
	return !("FAIL\n" == wpaCtrlCmd_SAVE_CONFIG());
}
void CWpaSupplicant::disconnect_device() {
	wpaCtrlCmd_DISCONNECT();
}
void CWpaSupplicant::logon() {
	wpaCtrlCmd_LOGON();
}
void CWpaSupplicant::logoff() {
	wpaCtrlCmd_LOGOFF();
}
void CWpaSupplicant::reassociate() {
	wpaCtrlCmd_REASSOCIATE();
}
void CWpaSupplicant::debug_level(int level) {
	wpaCtrlCmd_LEVEL(level);
}
void CWpaSupplicant::reconfigure() {
	wpaCtrlCmd_RECONFIGURE();
}
void CWpaSupplicant::terminate() {
	wpaCtrlCmd_TERMINATE();
}
void CWpaSupplicant::preauth(libnutcommon::MacAddress bssid) {
	wpaCtrlCmd_PREAUTH(bssid.toString());
}
int CWpaSupplicant::addNetwork() {
	QString reply = wpaCtrlCmd_ADD_NETWORK();
	if ("FAIL\n" == reply) {
		return -1;
	}
	else {
		return reply.toInt();
	}
}
NetconfigStatus CWpaSupplicant::checkAdHocNetwork(NetworkConfig &config) {
	// Note: IBSS can only be used with key_mgmt NONE (plaintext and static WEP)
	// and key_mgmt=WPA-NONE (fixed group key TKIP/CCMP). In addition, ap_scan has
	// to be set to 2 for IBSS. WPA-None requires following network block options:
	// proto=WPA, key_mgmt=WPA-NONE, pairwise=NONE, group=TKIP (or CCMP, but not
	// both), and psk must also be set.
	// Furthermore the follwing fields need to be set:
	// SSID,freq,ap_scan=2
	// All other options schould be unchecked;
	// User setable:
	// SSID,freq,psk,group
	NetconfigStatus failures;
	failures.id = -1;
	failures.failures = NCF_NONE;
	failures.eap_failures = ENCF_NONE;

	//Options that need to be set:
	if (config.frequency == -1) {
		failures.failures = (NetconfigFailures) (failures.failures | NCF_FREQ);
	}
	if (config.ssid.isEmpty()) {
		failures.failures = (NetconfigFailures) (failures.failures | NCF_SSID);
	}

	if (PROTO_UNDEFINED == config.protocols) { //WEP or none
		//If wep_tx_keyidx is set then we have a wep-network else, plaintext
		if (config.wep_tx_keyidx != -1) {
			//wep key has to be set, keymgmt=NONE, ssid
			switch (config.wep_tx_keyidx) {
				case 0:
					if (config.wep_key0.isEmpty()) {
						failures.failures = (NetconfigFailures)(failures.failures | NCF_WEP_KEY0);
						failures.failures = (NetconfigFailures)(failures.failures | NCF_WEP_KEY_IDX);
					}
					break;
				case 1:
					if (config.wep_key1.isEmpty()) {
						failures.failures = (NetconfigFailures)(failures.failures | NCF_WEP_KEY1);
						failures.failures = (NetconfigFailures)(failures.failures | NCF_WEP_KEY_IDX);
					}
					break;
				case 2:
					if (config.wep_key2.isEmpty()) {
						failures.failures = (NetconfigFailures)(failures.failures | NCF_WEP_KEY2);
						failures.failures = (NetconfigFailures)(failures.failures | NCF_WEP_KEY_IDX);
					}
					break;
				case 3:
					if (config.wep_key3.isEmpty()) {
						failures.failures = (NetconfigFailures)(failures.failures | NCF_WEP_KEY3);
						failures.failures = (NetconfigFailures)(failures.failures | NCF_WEP_KEY_IDX);
					}
				break;
			}
			if (config.keyManagement != KM_NONE) {
				failures.failures = (NetconfigFailures)(failures.failures | NCF_KEYMGMT);
			}
		}
		else {
			if ( KM_OFF != config.keyManagement) {
				failures.failures = (NetconfigFailures)(failures.failures | NCF_KEYMGMT);
			}
		}
	}
	else {
		if (PROTO_WPA != config.protocols) {
			failures.failures = (NetconfigFailures)(failures.failures | NCF_PROTO);
		}
		if (KM_WPA_NONE != config.keyManagement) {
			failures.failures = (NetconfigFailures)(failures.failures | NCF_KEYMGMT);
		}
		if (PCI_NONE != config.pairwise) {
			failures.failures = (NetconfigFailures)(failures.failures | NCF_PAIRWISE);
		}
		if ( !( (GCI_TKIP == config.group) != (GCI_CCMP == config.group) ) ) {
			failures.failures = (NetconfigFailures)(failures.failures | NCF_GROUP);
		}
	}
	return failures;
}

NetconfigStatus CWpaSupplicant::addNetwork(NetworkConfig config) {
	NetconfigStatus status;
	status.failures = NCF_NONE;
	status.eap_failures = ENCF_NONE;
	int netid = addNetwork();
	if (-1 == netid) {
		status.failures = NCF_ALL;
		status.eap_failures = ENCF_ALL;
		status.id = -1;
		return status;
	}
	else {
		status = editNetwork(netid,config);
		if ( (status.eap_failures != ENCF_NONE) || (NCF_NONE != status.failures) ) {
			removeNetwork(netid);
			status.id = -1;
		}
		else {
			emit networkListUpdated();
		}
		return status;
	}
}


NetconfigStatus CWpaSupplicant::editNetwork(int netid, NetworkConfig config) {
	NetconfigStatus failStatus;
	//Check if we're adding an ad-hoc network:
	if (QOOL_TRUE == config.mode) {
		failStatus = checkAdHocNetwork(config);
		if (NCF_NONE != failStatus.failures) {
			failStatus.id = netid;
			return failStatus;
		}
		else {
		
		}
	}

	failStatus.failures = NCF_NONE;
	failStatus.eap_failures = ENCF_NONE;
	failStatus.id = netid;


	//Set the network
	if (!setNetworkVariable(netid,"ssid",config.ssid) ) {
		failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_SSID);
	}
	if (!config.bssid.zero()) {
		if (! setNetworkVariable(netid,"bssid",config.bssid.toString()) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_BSSID);
		}
	}
	if (config.disabled != QOOL_UNDEFINED) {
		if (!setNetworkVariable(netid,"disabled",toNumberString(config.disabled)) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_DISABLED);
		}
	}
	if (!config.id_str.isEmpty()) {
		if ( setNetworkVariable(netid,"id_str",config.id_str) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_ID_STR);
		}
	}
	if (config.scan_ssid != QOOL_UNDEFINED) {
		if (!setNetworkVariable(netid,"scan_ssid",toNumberString(config.scan_ssid)) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_SCAN_SSID);
		}
	}
	if (config.priority >= 0) {
		if (!setNetworkVariable(netid,"priority",QString::number(config.priority)) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_PRIORITY);
		}
	}
	if (config.mode != QOOL_UNDEFINED) {
		if (!setNetworkVariable(netid,"mode",toNumberString(config.mode)) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_MODE);
		}
	}
	if (-1 != config.frequency) {
		if (!setNetworkVariable(netid,"frequency",QString::number(config.frequency)) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_FREQ);
		}
	}
	if (! (PROTO_UNDEFINED == config.protocols) ) {
		if ( !setNetworkVariable(netid,"proto",toString(config.protocols)) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_PROTO);
		}
	}
	if (! (KM_UNDEFINED == config.keyManagement) ) {
		if ( !setNetworkVariable(netid,"key_mgmt",toString(config.keyManagement)) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_KEYMGMT);
		}
	}
	if (! (AUTHALG_UNDEFINED == config.auth_alg) ) {
		if ( !setNetworkVariable(netid,"auth_alg",toString(config.auth_alg) )) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_AUTH_ALG);
		}
	}
	if (! (PCI_UNDEFINED == config.pairwise) ) {
		if ( !setNetworkVariable(netid,"pairwise",toString(config.pairwise) )) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_PAIRWISE);
		}
	}
	if ( !(GCI_UNDEFINED == config.group) && !(GCI_NONE == config.group) ) {
		if ( !setNetworkVariable(netid,"group",toString(config.group) )) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_GROUP);
		}
	}
	if (!config.psk.isEmpty()) {
		if ( (config.psk.size() < 8 || config.psk.size() > 63) && '"' == config.psk[0]) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_PSK);
		}
		else if ( !setNetworkVariable(netid,"psk",config.psk)) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_PSK);
		}
	}
	if (! EAPF_UNDEFINED == config.eapol_flags ) {
		if ( !setNetworkVariable(netid,"eapol_flags",toString(config.eapol_flags))) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_EAPOL_FLAGS);
		}
	}
	if (config.mixed_cell != QOOL_UNDEFINED) {
		if ( !setNetworkVariable(netid,"mixed_cell",toNumberString(config.mixed_cell)) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_MIXED_CELL);
		}
	}
	if (config.proactive_key_caching != QOOL_UNDEFINED) {
		if ( !setNetworkVariable(netid,"proactive_key_caching",toNumberString(config.proactive_key_caching)) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_PROA_KEY_CACHING);
		}
	}
	if (!config.wep_key0.isEmpty()) {
		if ( !setNetworkVariable(netid,"wep_key0",config.wep_key0) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_WEP_KEY0);
		}	
	}
	if (!config.wep_key1.isEmpty()) {
		if ( !setNetworkVariable(netid,"wep_key1",config.wep_key1)) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_WEP_KEY1);
		}	
	}
	if (!config.wep_key2.isEmpty()) {
		if ( !setNetworkVariable(netid,"wep_key2",config.wep_key2) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_WEP_KEY2);
		}	
	}
	if (!config.wep_key3.isEmpty()) {
		if ( !setNetworkVariable(netid,"wep_key3",config.wep_key3) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_WEP_KEY3);
		}	
	}
	if (config.wep_tx_keyidx <= 3 && config.wep_tx_keyidx >= 0) {
		if ( !setNetworkVariable(netid,"wep_tx_keyidx",QString::number(config.wep_tx_keyidx)) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_WEP_KEY_IDX);
		}
	}
	else if (config.wep_tx_keyidx != -1) {
		failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_WEP_KEY_IDX);
	}
	if (config.peerkey != QOOL_UNDEFINED) {
		if ( !setNetworkVariable(netid,"peerkey",toNumberString(config.peerkey)) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_PEERKEY);
		}
	}
	//Check if we have an EAP network
	if ((config.keyManagement & (KM_WPA_EAP | KM_IEEE8021X) ) || config.keyManagement == KM_UNDEFINED) {
		failStatus.eap_failures = editEapNetwork(netid,config.eap_config);
	}

	if (NCF_NONE == failStatus.failures && ENCF_NONE == failStatus.eap_failures) {
		emit networkListUpdated();
	}
	return failStatus;
}

NetworkConfig CWpaSupplicant::getNetworkConfig(int id) {
	NetworkConfig config;
	QString response;

	response = wpaCtrlCmd_GET_NETWORK(id,"ssid");
	if ("FAIL\n" != response) {
		config.ssid = wpaCtrlCmd_GET_NETWORK(id,"ssid");
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"bssid");
	if ("FAIL\n" != response) {
		config.bssid = libnutcommon::MacAddress(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"disabled");
	if ("FAIL\n" != response) {
		config.disabled = toQOOL(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"id_str");
	if ("FAIL\n" != response) {
		config.id_str = response;
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"scan_ssid");
	if ("FAIL\n" != response) {
		config.scan_ssid = toQOOL(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"priority");
	if ("FAIL\n" != response) {
		config.priority = response.toInt();
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"mode");
	if ("FAIL\n" != response) {
		config.mode = toQOOL(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"frequency");
	if ("FAIL\n" != response) {
		config.frequency = response.toInt();
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"proto");
	if ("FAIL\n" != response) {
		config.protocols = parseProtocols(response); 
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"key_mgmt");
	if ("FAIL\n" != response) {
		config.keyManagement = parseKeyMgmt(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"auth_alg");
	if ("FAIL\n" != response) {
		config.auth_alg = parseAuthAlg(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"pairwise");
	if ("FAIL\n" != response) {
		config.pairwise = parsePairwiseCiphers(response);
	}
	
	response = wpaCtrlCmd_GET_NETWORK(id,"group");
	if ("FAIL\n" != response) {
		config.group = parseGroupCiphers(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"psk");
	if ("FAIL\n" != response) {
		config.psk = response;
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"eapol_flags");
	if ("FAIL\n" != response) {
		config.eapol_flags = parseEapolFlags(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"mixed_cell");
	if ("FAIL\n" != response) {
		config.mixed_cell = toQOOL(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"proactive_key_caching");
	if ("FAIL\n" != response) {
		config.proactive_key_caching = toQOOL(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"wep_key0");
	if ("FAIL\n" != response) {
		config.wep_key0 = response;
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"wep_key1");
	if ("FAIL\n" != response) {
		config.wep_key1 = response;
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"wep_key2");
	if ("FAIL\n" != response) {
		config.wep_key2 = response;
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"wep_key3");
	if ("FAIL\n" != response) {
		config.wep_key3 = response;
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"wep_tx_keyidx");
	if ("FAIL\n" != response) {
		config.wep_tx_keyidx = response.toInt();
		if ( (0 == config.wep_tx_keyidx) && config.wep_key0.isEmpty() ) {
			config.wep_tx_keyidx = -1;
		}
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"peerkey");
	if ("FAIL\n" != response) {
		config.peerkey = toQOOL(response);
	}
	//Check if we need to fetch wpa_settings
	if ( config.keyManagement & (KM_IEEE8021X | KM_WPA_EAP)) {
		config.eap_config = getEapNetworkConfig(id);
	}
	return config;
}

EapNetworkConfig CWpaSupplicant::getEapNetworkConfig(int id) {
	EapNetworkConfig config;
	bool ok;
	QString response;
	//Check if the network uses EAP
	response = wpaCtrlCmd_GET_NETWORK(id,"key_mgmt");
	if ("FAIL\n" != response) {
		if ( !(parseKeyMgmt(response) & (KM_WPA_EAP | KM_IEEE8021X) ) ) {
			return config;
		}
	}
	else {
		return config;
	}
	//Get eap network config
	response = wpaCtrlCmd_GET_NETWORK(id,"eap");
	if ("FAIL\n" != response) {
		config.eap = parseEapMethod(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"identity");
	if ("FAIL\n" != response) {
		config.identity = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"anonymous_identity");
	if ("FAIL\n" != response) {
		config.anonymous_identity = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"password");
	if ("FAIL\n" != response) {
		config.password = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"ca_cert");
	if ("FAIL\n" != response) {
		config.ca_cert = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"ca_path");
	if ("FAIL\n" != response) {
		config.ca_path = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"client_cert");
	if ("FAIL\n" != response) {
		config.client_cert = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"private_key");
	if ("FAIL\n" != response) {
		config.private_key = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"private_key_passwd");
	if ("FAIL\n" != response) {
		config.private_key_passwd = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"dh_file");
	if ("FAIL\n" != response) {
		config.dh_file = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"subject_match");
	if ("FAIL\n" != response) {
		config.subject_match = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"altsubject_match");
	if ("FAIL\n" != response) {
		config.altsubject_match = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"phase1");
	if ("FAIL\n" != response) {
		config.phase1 = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"phase2");
	if ("FAIL\n" != response) {
		config.phase2 = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"ca_cert2");
	if ("FAIL\n" != response) {
		config.ca_cert2 = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"ca_path2");
	if ("FAIL\n" != response) {
		config.ca_path2 = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"client_cert2");
	if ("FAIL\n" != response) {
		config.client_cert2 = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"private_key2");
	if ("FAIL\n" != response) {
		config.private_key2 = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"private_key2_passwd");
	if ("FAIL\n" != response) {
		config.private_key2_passwd = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"dh_file2");
	if ("FAIL\n" != response) {
		config.dh_file2 = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"subject_match2");
	if ("FAIL\n" != response) {
		config.subject_match2 = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"altsubject_match2");
	if ("FAIL\n" != response) {
		config.altsubject_match2 = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"fragment_size");
	if ("FAIL\n" != response) {
		config.fragment_size = response.toInt(&ok);
		if (!ok) {
			config.fragment_size = -1;
		}
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"eappsk");
	if ("FAIL\n" != response) {
		config.eappsk = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"nai");
	if ("FAIL\n" != response) {
		config.nai = response;
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"pac_file");
	if ("FAIL\n" != response) {
		config.pac_file = response;
	}
	return config;
}
EapNetconfigFailures CWpaSupplicant::editEapNetwork(int netid, EapNetworkConfig config) {
	EapNetconfigFailures eap_failures = ENCF_NONE;
	if (EAPM_UNDEFINED != config.eap) {
		if (!setNetworkVariable(netid,"eap",toString(config.eap)) ) {
			eap_failures= (EapNetconfigFailures) (eap_failures | ENCF_EAP);
		}
	}
	if (!config.identity.isEmpty()) {
		if (!setNetworkVariable(netid,"identity",config.identity) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_IDENTITY);
		}
	}
	if (!config.anonymous_identity.isEmpty()) {
		if (!setNetworkVariable(netid,"anonymous_identity",config.anonymous_identity) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_ANON_IDENTITY);
		}
	}
	if (!config.password.isEmpty()) {
		if (!setNetworkVariable(netid,"password",config.password) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_PASSWD);
		}
	}
	if (!config.ca_cert.isEmpty()) {
		if (!setNetworkVariable(netid,"ca_cert",config.ca_cert) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_CA_CERT);
		}
	}
	if (!config.ca_path.isEmpty()) {
		if (!setNetworkVariable(netid,"ca_path",config.ca_path) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_CA_PATH);
		}
	}
	if (!config.client_cert.isEmpty()) {
		if (!setNetworkVariable(netid,"client_cert",config.client_cert) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_CLIENT_CERT);
		}
	}
	if (!config.private_key.isEmpty()) {
		if (!setNetworkVariable(netid,"private_key",config.private_key) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_PRIVATE_KEY);
		}
	}
	if (!config.private_key_passwd.isEmpty()) {
		if (!setNetworkVariable(netid,"private_key_passwd",config.private_key_passwd) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_PRIVATE_KEY_PASSWD);
		}
	}
	if (!config.dh_file.isEmpty()) {
		if (!setNetworkVariable(netid,"dh_file",config.dh_file) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_DH_FILE);
		}
	}
	if (!config.subject_match.isEmpty()) {
		if (!setNetworkVariable(netid,"subject_match",config.subject_match) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_SUBJECT_MATCH);
		}
	}
	if (!config.altsubject_match.isEmpty()) {
		if (!setNetworkVariable(netid,"altsubject_match",config.altsubject_match) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_ALTSUBJECT_MATCH);
		}
	}
	if (!config.phase1.isEmpty()) {
		if (!setNetworkVariable(netid,"phase1",config.phase1) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_PHASE1);
		}
	}
	if (!config.phase2.isEmpty()) {
		if (!setNetworkVariable(netid,"phase2",config.phase2) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_PHASE2);
		}
	}
	if (!config.ca_cert2.isEmpty()) {
		if (!setNetworkVariable(netid,"ca_cert2",config.ca_cert2) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_CA_CERT2);
		}
	}
	if (!config.ca_path2.isEmpty()) {
		if (!setNetworkVariable(netid,"ca_path2",config.ca_path2) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_CA_PATH2);
		}
	}
	if (!config.client_cert2.isEmpty()) {
		if (!setNetworkVariable(netid,"client_cert2",config.client_cert2) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_CLIENT_CERT2);
		}
	}
	if (!config.private_key2.isEmpty()) {
		if (!setNetworkVariable(netid,"private_key2",config.private_key2) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_PRIVATE_KEY2);
		}
	}
	if (!config.private_key2_passwd.isEmpty()) {
		if (!setNetworkVariable(netid,"private_key2_passwd",config.private_key2_passwd) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_PRIVATE_KEY2_PASSWD);
		}
	}
	if (!config.dh_file2.isEmpty()) {
		if (!setNetworkVariable(netid,"dh_file2",config.dh_file2) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_DH_FILE2);
		}
	}
	if (!config.subject_match2.isEmpty()) {
		if (!setNetworkVariable(netid,"subject_match2",config.subject_match2) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_SUBJECT_MATCH);
		}
	}
	if (!config.altsubject_match2.isEmpty()) {
		if (!setNetworkVariable(netid,"altsubject_match2",config.altsubject_match2) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_ALTSUBJECT_MATCH);
		}
	}
	if (-1 != config.fragment_size) {
		if (!setNetworkVariable(netid,"altsubject_match2",QString::number(config.fragment_size)) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_FRAGMENT_SIZE);
		}
	}
	if (!config.eappsk.isEmpty()) {
		if (!setNetworkVariable(netid,"eappsk",config.eappsk) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_EAPPSK);
		}
	}
	if (!config.nai.isEmpty()) {
		if (!setNetworkVariable(netid,"nai",config.nai) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_NAI);
		}
	}
	if (!config.pac_file.isEmpty()) {
		if (!setNetworkVariable(netid,"pac_file",config.pac_file) ) {
			eap_failures = (EapNetconfigFailures) (eap_failures | ENCF_PAC_FILE);
		}
	}
	return eap_failures;
}

void CWpaSupplicant::removeNetwork(int id) {
	wpaCtrlCmd_REMOVE_NETWORK(id);
}

bool CWpaSupplicant::setBssid(int id, libnutcommon::MacAddress bssid) {
	if (0 == wpaCtrlCmd_BSSID(id,bssid.toString()).indexOf("OK")) {
		return true;
	}
	else {
		return false;
	}
}
//Plain setVaraiable functions
void CWpaSupplicant::setVariable(QString var, QString val) {
	wpaCtrlCmd_SET(var,val);
}
bool CWpaSupplicant::setNetworkVariable(int id, QString var, QString val) {
	QString ret = wpaCtrlCmd_SET_NETWORK(id,var,val);
	if (ret.contains("OK")) {
		return true;
	}
	else {
		return false;
	}
}
QString CWpaSupplicant::getNetworkVariable(int id, QString val) {
	return wpaCtrlCmd_GET_NETWORK(id,val);
}

//Functions with a lot more functionality  (in the parser functions :)
QList<ShortNetworkInfo> CWpaSupplicant::listNetworks() {
	QString reply = wpaCtrlCmd_LIST_NETWORKS();
	if (!reply.isEmpty()) {
		QList<ShortNetworkInfo> info = parseListNetwork(sliceMessage(reply));
		for (int i = 0; i < info.size(); i++)
			info[i].adhoc = toBool(getNetworkVariable(info[i].id,"mode"));
		return info;
	}
	else {
		return QList<ShortNetworkInfo>();
	}
}

Status CWpaSupplicant::status() {
	QString reply = wpaCtrlCmd_STATUS(true);
	if (!reply.isEmpty()) {
		return parseStatus(sliceMessage(reply));
	}
	else {
		Status dummy;
		return dummy;
	}
}
MIBVariables CWpaSupplicant::getMIBVariables() {
	QString reply = wpaCtrlCmd_MIB();
	if (!reply.isEmpty()) {
		return parseMIB(sliceMessage(reply));
	}
	else {
		return (MIBVariables) QList<MIBVariable>();
	}
}

Capabilities CWpaSupplicant::getCapabilities() {
	Capabilities caps;
	caps.eap = EAPM_UNDEFINED;
	caps.pairwise = PCI_UNDEFINED;
	caps.group = GCI_UNDEFINED;
	caps.keyManagement = KM_UNDEFINED;
	caps.proto = PROTO_UNDEFINED;
	caps.auth_alg = AUTHALG_UNDEFINED;
	QString response;
	response = wpaCtrlCmd_GET_CAPABILITY("eap",false);
	if ("FAIL\n" != response) {
		caps.eap = parseEapMethod(response);
	}
	response = wpaCtrlCmd_GET_CAPABILITY("pairwise",false);
	if ("FAIL\n" != response) {
		caps.pairwise = parsePairwiseCiphers(response);
	}
	response = wpaCtrlCmd_GET_CAPABILITY("group",false);
	if ("FAIL\n" != response) {
		caps.group = parseGroupCiphers(response);
	}
	response = wpaCtrlCmd_GET_CAPABILITY("key_mgmt",false);
	if ("FAIL\n" != response) {
		caps.keyManagement = parseKeyMgmt(response);
	}
	response = wpaCtrlCmd_GET_CAPABILITY("proto",false);
	if ("FAIL\n" != response) {
		caps.proto = parseProtocols(response);
	}
	response = wpaCtrlCmd_GET_CAPABILITY("auth_alg",false);
	if ("FAIL\n" != response) {
		caps.auth_alg = parseAuthAlg(response);
	}
	return caps;
}
}
