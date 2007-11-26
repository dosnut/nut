#include "wpa_supplicant.h"
#include <QDebug>

namespace libnutwireless {

//CWpa_supplicant
QList<quint8>& CWpa_Supplicant::getSupportedChannels() {
	//create supportedChannels?
	if (supportedChannels.isEmpty() && !supportedFrequencies.isEmpty()) {
		foreach(quint32 freq, supportedFrequencies) {
			supportedChannels.append(frequencyToChannel(freq));
		}
	}
	return supportedChannels;
}

//Function to respond to ctrl requests from wpa_supplicant
void CWpa_Supplicant::response(Request request, QString msg) {
	QString cmd = toString(request.type);
	if (!cmd.isEmpty()) {
		wps_cmd_CTRL_RSP(cmd,request.id,msg);
	}
}

bool CWpa_Supplicant::selectNetwork(int id) {
	//TODO: Check if we need to set ap_scan first

	BOOL hasMode = toWpsBool(getNetworkVariable(id,"mode"));
	//Let's hope wpa_supplicant returns fail only when network id is invalid
	if (BOOL_UNDEFINED == hasMode) {
		return false;
	}
	else if (BOOL_TRUE == hasMode) {
		ap_scan(2);
		printMessage(tr("auto-setting ap_scan=2."));
	}
	else {
		//Check if we have defaults, and if the last network was an ap-network
		if (lastWasAdHoc) {
			lastWasAdHoc = false;
			if (-1 != apScanDefault) {
				ap_scan(apScanDefault);
				printMessage(tr("Using your last ap_scan settings for auto-setting: %1").arg(QString::number(apScanDefault)));
			}
			else {
				printMessage(tr("You must set ap_scan to your needs!"));

			}
		}
	}
	if ("OK\n" == wps_cmd_SELECT_NETWORK(id)) {
		return true;
	}
	else {
		//Reset ap_scan:
		if (lastWasAdHoc) {
			ap_scan(apScanDefault);
		}
		return false;
	}
}
bool CWpa_Supplicant::enableNetwork(int id) {
	if ("OK\n" == wps_cmd_ENABLE_NETWORK(id)) {
		emit networkListUpdated();
		return true;
	}
	else {
		return false;
	}
}
bool CWpa_Supplicant::disableNetwork(int id) {
	if ("OK\n" == wps_cmd_DISABLE_NETWORK(id)) {
		emit networkListUpdated();
		return true;
	}
	else {
		return false;
	}
}

bool CWpa_Supplicant::ap_scan(int type) {
	if ( (0 <= type and 2 >= type) ) {
		if ("FAIL\n" == wps_cmd_AP_SCAN(type)) {
			return false;
		}
		else {
			printMessage(QString("Setting ap_scan=%1").arg(type));
			//ap_scan variables accordingly
			if (2 == type) {
				lastWasAdHoc = true;
			}
			else {
				lastWasAdHoc = false;
				apScanDefault = type;
			}
			return true;
		}
	}
	return false;
}
bool CWpa_Supplicant::save_config() {
	return !("FAIL\n" == wps_cmd_SAVE_CONFIG());
}
void CWpa_Supplicant::disconnect_device() {
	wps_cmd_DISCONNECT();
}
void CWpa_Supplicant::logon() {
	wps_cmd_LOGON();
}
void CWpa_Supplicant::logoff() {
	wps_cmd_LOGOFF();
}
void CWpa_Supplicant::reassociate() {
	wps_cmd_REASSOCIATE();
}
void CWpa_Supplicant::debug_level(int level) {
	wps_cmd_LEVEL(level);
}
void CWpa_Supplicant::reconfigure() {
	wps_cmd_RECONFIGURE();
}
void CWpa_Supplicant::terminate() {
	wps_cmd_TERMINATE();
}
void CWpa_Supplicant::preauth(libnutcommon::MacAddress bssid) {
	wps_cmd_PREAUTH(bssid.toString());
}
int CWpa_Supplicant::addNetwork() {
	QString reply = wps_cmd_ADD_NETWORK();
	if ("FAIL\n" == reply) {
		return -1;
	}
	else {
		return reply.toInt();
	}
}
NetconfigStatus CWpa_Supplicant::checkAdHocNetwork(NetworkConfig &config) {
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
	//Options that need to be set:
	if (config.frequency == -1) {
		failures.failures = (NetconfigFailures) (failures.failures | NCF_FREQ);
	}
	if (config.ssid.isEmpty()) {
		failures.failures = (NetconfigFailures) (failures.failures | NCF_SSID);
	}
	return failures;
}

NetconfigStatus CWpa_Supplicant::addNetwork(NetworkConfig config) {
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


NetconfigStatus CWpa_Supplicant::editNetwork(int netid, NetworkConfig config) {
	NetconfigStatus wps_fail_status;
	//Check if we're adding an ad-hoc network:
	if (BOOL_TRUE == config.mode) {
		wps_fail_status = checkAdHocNetwork(config);
		if (NCF_NONE != wps_fail_status.failures) {
			wps_fail_status.id = netid;
			return wps_fail_status;
		}
		else {
		
		}
	}

	wps_fail_status.failures = NCF_NONE;
	wps_fail_status.eap_failures = ENCF_NONE;
	wps_fail_status.id = netid;


	//Set the network
	if (!setNetworkVariable(netid,"ssid",config.ssid) ) {
		wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_SSID);
	}
	if (!config.bssid.zero()) {
		if (! setNetworkVariable(netid,"bssid",config.bssid.toString()) ) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_BSSID);
		}
	}
	if (config.disabled != BOOL_UNDEFINED) {
		if (!setNetworkVariable(netid,"disabled",toNumberString(config.disabled)) ) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_DISABLED);
		}
	}
	if (!config.id_str.isEmpty()) {
		if ( setNetworkVariable(netid,"id_str",config.id_str) ) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_ID_STR);
		}
	}
	if (config.scan_ssid != BOOL_UNDEFINED) {
		if (!setNetworkVariable(netid,"scan_ssid",toNumberString(config.scan_ssid)) ) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_SCAN_SSID);
		}
	}
	if (config.priority >= 0) {
		if (!setNetworkVariable(netid,"priority",QString::number(config.priority)) ) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_PRIORITY);
		}
	}
	if (config.mode != BOOL_UNDEFINED) {
		if (!setNetworkVariable(netid,"mode",toNumberString(config.mode)) ) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_MODE);
		}
	}
	if (-1 != config.frequency) {
		if (!setNetworkVariable(netid,"frequency",QString::number(config.frequency)) ) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_FREQ);
		}
	}
	if (! (PROTO_UNDEFINED == config.protocols) ) {
		if ( !setNetworkVariable(netid,"proto",toString(config.protocols)) ) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_PROTO);
		}
	}
	if (! (KM_UNDEFINED == config.keyManagement) ) {
		if ( !setNetworkVariable(netid,"key_mgmt",toString(config.keyManagement)) ) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_KEYMGMT);
		}
	}
	if (! (AUTHALG_UNDEFINED == config.auth_alg) ) {
		if ( !setNetworkVariable(netid,"auth_alg",toString(config.auth_alg) )) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_AUTH_ALG);
		}
	}
	if (! (PCI_UNDEFINED == config.pairwise) ) {
		if ( !setNetworkVariable(netid,"pairwise",toString(config.pairwise) )) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_PAIRWISE);
		}
	}
	if ( !(GCI_UNDEFINED == config.group) && !(GCI_NONE == config.group) ) {
		if ( !setNetworkVariable(netid,"group",toString(config.group) )) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_GROUP);
		}
	}
	if (!config.psk.isEmpty()) {
		if ( (config.psk.size() < 8 || config.psk.size() > 63) && '"' == config.psk[0]) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_PSK);
		}
		else if ( !setNetworkVariable(netid,"psk",config.psk)) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_PSK);
		}
	}
	if (! EAPF_UNDEFINED == config.eapol_flags ) {
		if ( !setNetworkVariable(netid,"eapol_flags",toString(config.eapol_flags))) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_EAPOL_FLAGS);
		}
	}
	if (config.mixed_cell != BOOL_UNDEFINED) {
		if ( !setNetworkVariable(netid,"mixed_cell",toNumberString(config.mixed_cell)) ) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_MIXED_CELL);
		}
	}
	if (config.proactive_key_caching != BOOL_UNDEFINED) {
		if ( !setNetworkVariable(netid,"proactive_key_caching",toNumberString(config.proactive_key_caching)) ) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_PROA_KEY_CACHING);
		}
	}
	if (!config.wep_key0.isEmpty()) {
		if ( !setNetworkVariable(netid,"wep_key0",config.wep_key0) ) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_WEP_KEY0);
		}	
	}
	if (!config.wep_key1.isEmpty()) {
		if ( !setNetworkVariable(netid,"wep_key1",config.wep_key1)) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_WEP_KEY1);
		}	
	}
	if (!config.wep_key2.isEmpty()) {
		if ( !setNetworkVariable(netid,"wep_key2",config.wep_key2) ) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_WEP_KEY2);
		}	
	}
	if (!config.wep_key3.isEmpty()) {
		if ( !setNetworkVariable(netid,"wep_key3",config.wep_key3) ) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_WEP_KEY3);
		}	
	}
	if (config.wep_tx_keyidx <= 3 && config.wep_tx_keyidx >= 0) {
		if ( !setNetworkVariable(netid,"wep_tx_keyidx",QString::number(config.wep_tx_keyidx)) ) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_WEP_KEY_IDX);
		}
	}
	else if (config.wep_tx_keyidx != -1) {
		wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_WEP_KEY_IDX);
	}
	if (config.peerkey != BOOL_UNDEFINED) {
		if ( !setNetworkVariable(netid,"peerkey",toNumberString(config.peerkey)) ) {
			wps_fail_status.failures = (NetconfigFailures) (wps_fail_status.failures | NCF_PEERKEY);
		}
	}
	//Check if we have an EAP network
	if ((config.keyManagement & (KM_WPA_EAP | KM_IEEE8021X) ) || config.keyManagement == KM_UNDEFINED) {
		wps_fail_status.eap_failures = wps_editEapNetwork(netid,config.eap_config);
	}
	return wps_fail_status;
}

NetworkConfig CWpa_Supplicant::getNetworkConfig(int id) {
	NetworkConfig config;
	QString response;

	response = wps_cmd_GET_NETWORK(id,"ssid");
	if ("FAIL\n" != response) {
		config.ssid = wps_cmd_GET_NETWORK(id,"ssid");
	}

	response = wps_cmd_GET_NETWORK(id,"bssid");
	if ("FAIL\n" != response) {
		config.bssid = libnutcommon::MacAddress(response);
	}

	response = wps_cmd_GET_NETWORK(id,"disabled");
	if ("FAIL\n" != response) {
		config.disabled = toWpsBool(response);
	}

	response = wps_cmd_GET_NETWORK(id,"id_str");
	if ("FAIL\n" != response) {
		config.id_str = response;
	}

	response = wps_cmd_GET_NETWORK(id,"scan_ssid");
	if ("FAIL\n" != response) {
		config.scan_ssid = toWpsBool(response);
	}

	response = wps_cmd_GET_NETWORK(id,"priority");
	if ("FAIL\n" != response) {
		config.priority = response.toInt();
	}

	response = wps_cmd_GET_NETWORK(id,"mode");
	if ("FAIL\n" != response) {
		config.mode = toWpsBool(response);
	}

	response = wps_cmd_GET_NETWORK(id,"frequency");
	if ("FAIL\n" != response) {
		config.frequency = response.toInt();
	}

	response = wps_cmd_GET_NETWORK(id,"proto");
	if ("FAIL\n" != response) {
		config.protocols = parseProtocols(response); // TODO: implement
	}

	response = wps_cmd_GET_NETWORK(id,"key_mgmt");
	if ("FAIL\n" != response) {
		config.keyManagement = parseKeyMgmt(response); // TODO: implement
	}

	response = wps_cmd_GET_NETWORK(id,"auth_alg");
	if ("FAIL\n" != response) {
		config.auth_alg = parseAuthAlg(response); // TODO: implement
	}

	response = wps_cmd_GET_NETWORK(id,"pairwise");
	if ("FAIL\n" != response) {
		config.pairwise = parsePairwiseCiphers(response); // TODO: implement
	}
	
	response = wps_cmd_GET_NETWORK(id,"group");
	if ("FAIL\n" != response) {
		config.group = parseGroupCiphers(response); // TODO: implement
	}

	response = wps_cmd_GET_NETWORK(id,"psk");
	if ("FAIL\n" != response) {
		config.psk = response;
	}

	response = wps_cmd_GET_NETWORK(id,"eapol_flags");
	if ("FAIL\n" != response) {
		config.eapol_flags = parseEapolFlags(response); // TODO: implement
	}

	response = wps_cmd_GET_NETWORK(id,"mixed_cell");
	if ("FAIL\n" != response) {
		config.mixed_cell = toWpsBool(response);
	}

	response = wps_cmd_GET_NETWORK(id,"proactive_key_caching");
	if ("FAIL\n" != response) {
		config.proactive_key_caching = toWpsBool(response);
	}

	response = wps_cmd_GET_NETWORK(id,"wep_key0");
	if ("FAIL\n" != response) {
		config.wep_key0 = response;
	}

	response = wps_cmd_GET_NETWORK(id,"wep_key1");
	if ("FAIL\n" != response) {
		config.wep_key1 = response;
	}

	response = wps_cmd_GET_NETWORK(id,"wep_key2");
	if ("FAIL\n" != response) {
		config.wep_key2 = response;
	}

	response = wps_cmd_GET_NETWORK(id,"wep_key3");
	if ("FAIL\n" != response) {
		config.wep_key3 = response;
	}

	response = wps_cmd_GET_NETWORK(id,"wep_tx_keyidx");
	if ("FAIL\n" != response) {
		config.wep_tx_keyidx = response.toInt();
		if ( (0 == config.wep_tx_keyidx) && config.wep_key0.isEmpty() ) {
			config.wep_tx_keyidx = -1;
		}
	}

	response = wps_cmd_GET_NETWORK(id,"peerkey");
	if ("FAIL\n" != response) {
		config.peerkey = toWpsBool(response);
	}
	//Check if we need to fetch wpa_settings
	if ( config.keyManagement & (KM_IEEE8021X | KM_WPA_EAP)) {
		config.eap_config = wps_getEapNetworkConfig(id);
	}
	return config;
}

EapNetworkConfig CWpa_Supplicant::wps_getEapNetworkConfig(int id) {
	EapNetworkConfig config;
	bool ok;
	QString response;
	//Check if the network uses EAP
	response = wps_cmd_GET_NETWORK(id,"key_mgmt");
	if ("FAIL\n" != response) {
		if ( !(parseKeyMgmt(response) & (KM_WPA_EAP | KM_IEEE8021X) ) ) {
			return config;
		}
	}
	else {
		return config;
	}
	//Get eap network config
	response = wps_cmd_GET_NETWORK(id,"eap");
	if ("FAIL\n" != response) {
		config.eap = parseEapMethod(response); //space-separated list of accepted EAP methods TODO: implement
	}
	response = wps_cmd_GET_NETWORK(id,"identity");
	if ("FAIL\n" != response) {
		config.identity = response;
	}
	response = wps_cmd_GET_NETWORK(id,"anonymous_identity");
	if ("FAIL\n" != response) {
		config.anonymous_identity = response;
	}
	response = wps_cmd_GET_NETWORK(id,"password");
	if ("FAIL\n" != response) {
		config.password = response;
	}
	response = wps_cmd_GET_NETWORK(id,"ca_cert");
	if ("FAIL\n" != response) {
		config.ca_cert = response;
	}
	response = wps_cmd_GET_NETWORK(id,"ca_path");
	if ("FAIL\n" != response) {
		config.ca_path = response;
	}
	response = wps_cmd_GET_NETWORK(id,"client_cert");
	if ("FAIL\n" != response) {
		config.client_cert = response;
	}
	response = wps_cmd_GET_NETWORK(id,"private_key");
	if ("FAIL\n" != response) {
		config.private_key = response;
	}
	response = wps_cmd_GET_NETWORK(id,"private_key_passwd");
	if ("FAIL\n" != response) {
		config.private_key_passwd = response;
	}
	response = wps_cmd_GET_NETWORK(id,"dh_file");
	if ("FAIL\n" != response) {
		config.dh_file = response;
	}
	response = wps_cmd_GET_NETWORK(id,"subject_match");
	if ("FAIL\n" != response) {
		config.subject_match = response;
	}
	response = wps_cmd_GET_NETWORK(id,"altsubject_match");
	if ("FAIL\n" != response) {
		config.altsubject_match = response;
	}
	response = wps_cmd_GET_NETWORK(id,"phase1");
	if ("FAIL\n" != response) {
		config.phase1 = response;
	}
	response = wps_cmd_GET_NETWORK(id,"phase2");
	if ("FAIL\n" != response) {
		config.phase2 = response;
	}
	response = wps_cmd_GET_NETWORK(id,"ca_cert2");
	if ("FAIL\n" != response) {
		config.ca_cert2 = response;
	}
	response = wps_cmd_GET_NETWORK(id,"ca_path2");
	if ("FAIL\n" != response) {
		config.ca_path2 = response;
	}
	response = wps_cmd_GET_NETWORK(id,"client_cert2");
	if ("FAIL\n" != response) {
		config.client_cert2 = response;
	}
	response = wps_cmd_GET_NETWORK(id,"private_key2");
	if ("FAIL\n" != response) {
		config.private_key2 = response;
	}
	response = wps_cmd_GET_NETWORK(id,"private_key2_passwd");
	if ("FAIL\n" != response) {
		config.private_key2_passwd = response;
	}
	response = wps_cmd_GET_NETWORK(id,"dh_file2");
	if ("FAIL\n" != response) {
		config.dh_file2 = response;
	}
	response = wps_cmd_GET_NETWORK(id,"subject_match2");
	if ("FAIL\n" != response) {
		config.subject_match2 = response;
	}
	response = wps_cmd_GET_NETWORK(id,"altsubject_match2");
	if ("FAIL\n" != response) {
		config.altsubject_match2 = response;
	}
	response = wps_cmd_GET_NETWORK(id,"fragment_size");
	if ("FAIL\n" != response) {
		config.fragment_size = response.toInt(&ok);
		if (!ok) {
			config.fragment_size = -1;
		}
	}
	response = wps_cmd_GET_NETWORK(id,"eappsk");
	if ("FAIL\n" != response) {
		config.eappsk = response;
	}
	response = wps_cmd_GET_NETWORK(id,"nai");
	if ("FAIL\n" != response) {
		config.nai = response;
	}
	response = wps_cmd_GET_NETWORK(id,"pac_file");
	if ("FAIL\n" != response) {
		config.pac_file = response;
	}
	return config;
}
EapNetconfigFailures CWpa_Supplicant::wps_editEapNetwork(int netid, EapNetworkConfig config) {
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

void CWpa_Supplicant::removeNetwork(int id) {
	wps_cmd_REMOVE_NETWORK(id);
}

//TODO:Check is id is in range
void CWpa_Supplicant::setBssid(int id, libnutcommon::MacAddress bssid) {
	wps_cmd_BSSID(id,bssid.toString());
}
//Plain setVaraiable functions
void CWpa_Supplicant::setVariable(QString var, QString val) {
	wps_cmd_SET(var,val);
}
bool CWpa_Supplicant::setNetworkVariable(int id, QString var, QString val) {
	QString ret = wps_cmd_SET_NETWORK(id,var,val);
	if (ret.contains("OK")) {
		return true;
	}
	else {
		return false;
	}
}
QString CWpa_Supplicant::getNetworkVariable(int id, QString val) {
	return wps_cmd_GET_NETWORK(id,val);
}

//Functions with a lot more functionality  (in the parser functions :)
QList<ShortNetworkInfo> CWpa_Supplicant::listNetworks() {
	QString reply = wps_cmd_LIST_NETWORKS();
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

Status CWpa_Supplicant::status() {
	QString reply = wps_cmd_STATUS(true);
	if (!reply.isEmpty()) {
		return parseStatus(sliceMessage(reply));
	}
	else {
		Status dummy;
		return dummy;
	}
}
MIBVariables CWpa_Supplicant::getMIBVariables() {
	QString reply = wps_cmd_MIB();
	if (!reply.isEmpty()) {
		return parseMIB(sliceMessage(reply));
	}
	else {
		return (MIBVariables) QList<MIBVariable>();
	}
}

Capabilities CWpa_Supplicant::getCapabilities() {
	Capabilities caps;
	caps.eap = EAPM_UNDEFINED;
	caps.pairwise = PCI_UNDEFINED;
	caps.group = GCI_UNDEFINED;
	caps.keyManagement = KM_UNDEFINED;
	caps.proto = PROTO_UNDEFINED;
	caps.auth_alg = AUTHALG_UNDEFINED;
	QString response;
	response = wps_cmd_GET_CAPABILITY("eap",false);
	if ("FAIL\n" != response) {
		caps.eap = parseEapMethod(response);
	}
	response = wps_cmd_GET_CAPABILITY("pairwise",false);
	if ("FAIL\n" != response) {
		caps.pairwise = parsePairwiseCiphers(response);
	}
	response = wps_cmd_GET_CAPABILITY("group",false);
	if ("FAIL\n" != response) {
		caps.group = parseGroupCiphers(response);
	}
	response = wps_cmd_GET_CAPABILITY("key_mgmt",false);
	if ("FAIL\n" != response) {
		caps.keyManagement = parseKeyMgmt(response);
	}
	response = wps_cmd_GET_CAPABILITY("proto",false);
	if ("FAIL\n" != response) {
		caps.proto = parseProtocols(response);
	}
	response = wps_cmd_GET_CAPABILITY("auth_alg",false);
	if ("FAIL\n" != response) {
		caps.auth_alg = parseAuthAlg(response);
	}
	return caps;
}
}
