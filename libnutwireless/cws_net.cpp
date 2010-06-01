#include "wpa_supplicant.h"

namespace libnutwireless {

NetconfigStatus CWpaSupplicant::checkAdHocNetwork(CNetworkConfig &config) {
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
	if (config.get_frequency() == -1) {
		failures.failures = (NetconfigFailures) (failures.failures | NCF_FREQ);
	}
	if (config.get_ssid().isEmpty()) {
		failures.failures = (NetconfigFailures) (failures.failures | NCF_SSID);
	}

	if (PROTO_UNDEFINED == config.get_protocols()) { //WEP or none
		//If wep_tx_keyidx is set then we have a wep-network else, plaintext
		if (config.get_wep_tx_keyidx() != -1) {
			//wep key has to be set, keymgmt=NONE, ssid
			switch (config.get_wep_tx_keyidx()) {
				case 0:
					if (config.get_wep_key0().isEmpty()) {
						failures.failures = (NetconfigFailures)(failures.failures | NCF_WEP_KEY0);
						failures.failures = (NetconfigFailures)(failures.failures | NCF_WEP_KEY_IDX);
					}
					break;
				case 1:
					if (config.get_wep_key1().isEmpty()) {
						failures.failures = (NetconfigFailures)(failures.failures | NCF_WEP_KEY1);
						failures.failures = (NetconfigFailures)(failures.failures | NCF_WEP_KEY_IDX);
					}
					break;
				case 2:
					if (config.get_wep_key2().isEmpty()) {
						failures.failures = (NetconfigFailures)(failures.failures | NCF_WEP_KEY2);
						failures.failures = (NetconfigFailures)(failures.failures | NCF_WEP_KEY_IDX);
					}
					break;
				case 3:
					if (config.get_wep_key3().isEmpty()) {
						failures.failures = (NetconfigFailures)(failures.failures | NCF_WEP_KEY3);
						failures.failures = (NetconfigFailures)(failures.failures | NCF_WEP_KEY_IDX);
					}
				break;
			}
			if (config.get_key_mgmt() != KM_NONE) {
				failures.failures = (NetconfigFailures)(failures.failures | NCF_KEYMGMT);
			}
		}
		else {
			if ( KM_OFF != config.get_key_mgmt()) {
				failures.failures = (NetconfigFailures)(failures.failures | NCF_KEYMGMT);
			}
		}
	}
	else {
		if (PROTO_WPA != config.get_protocols()) {
			failures.failures = (NetconfigFailures)(failures.failures | NCF_PROTO);
		}
		if (KM_WPA_NONE != config.get_key_mgmt()) {
			failures.failures = (NetconfigFailures)(failures.failures | NCF_KEYMGMT);
		}
		if (PCI_NONE != config.get_pairwise()) {
			failures.failures = (NetconfigFailures)(failures.failures | NCF_PAIRWISE);
		}
		if ( !( (GCI_TKIP == config.get_group()) != (GCI_CCMP == config.get_group()) ) ) {
			failures.failures = (NetconfigFailures)(failures.failures | NCF_GROUP);
		}
	}
	return failures;
}
	
	
NetconfigStatus CWpaSupplicant::editNetwork(int netid, CNetworkConfig config) {
	NetconfigStatus failStatus;
	
	//Check if the network we edit is managed by us and if the NetworkIds are the same
	QString response = wpaCtrlCmd_GET_NETWORK(netid,"id_str");
	if ("FAIL\n" != response) {
		CNetworkConfig::NetworkId i = CNetworkConfig::toNetworkId(response);
		if (m_managedNetworks.contains(i)) { //managed by us, update our copy
			m_managedNetworks[i] = config;
		}
	} //else, not managed by us (at least not yet)

	//Check if we're adding an ad-hoc network:
	if (QOOL_TRUE == config.get_mode()) {
		failStatus = checkAdHocNetwork(config);
		if (NCF_NONE != failStatus.failures) {
			failStatus.id = netid;
			return failStatus;
		}
	}

	failStatus.failures = NCF_NONE;
	failStatus.eap_failures = ENCF_NONE;
	failStatus.id = netid;

	//Set the network
	if (!setNetworkVariable(netid,"ssid",config.get_ssid()) ) {
		failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_SSID);
	}
	if (!setBssid(netid,config.get_bssid().toString()) ) { //Always set bssid, otherwise wpa_supplicant doesn't care about "any" bssid
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_BSSID);
	}
	if (config.get_disabled() != QOOL_UNDEFINED) {
		if (!setNetworkVariable(netid,"disabled",toNumberString(config.get_disabled())) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_DISABLED);
		}
	}
	if (!config.get_id_str().isEmpty()) {
		if (!setNetworkVariable(netid,"id_str",config.get_id_str()) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_ID_STR);
		}
	}
	if (config.get_scan_ssid() != QOOL_UNDEFINED) {
		if (!setNetworkVariable(netid,"scan_ssid",toNumberString(config.get_scan_ssid())) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_SCAN_SSID);
		}
	}
	if (config.get_priority() >= 0) {
		if (!setNetworkVariable(netid,"priority",QString::number(config.get_priority())) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_PRIORITY);
		}
	}
	if (config.get_mode() != QOOL_UNDEFINED) {
		if (!setNetworkVariable(netid,"mode",toNumberString(config.get_mode())) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_MODE);
		}
	}
	if (-1 != config.get_frequency()) {
		if (!setNetworkVariable(netid,"frequency",QString::number(config.get_frequency())) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_FREQ);
		}
	}
	if (! (PROTO_UNDEFINED == config.get_protocols()) ) {
		if ( !setNetworkVariable(netid,"proto",toString(config.get_protocols())) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_PROTO);
		}
	}
	if (! (KM_UNDEFINED == config.get_key_mgmt()) ) {
		if ( !setNetworkVariable(netid,"key_mgmt",toString(config.get_key_mgmt())) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_KEYMGMT);
		}
	}
	if (! (AUTHALG_UNDEFINED == config.get_auth_alg()) ) {
		if ( !setNetworkVariable(netid,"auth_alg",toString(config.get_auth_alg()) )) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_AUTH_ALG);
		}
	}
	if (! (PCI_UNDEFINED == config.get_pairwise()) ) {
		if ( !setNetworkVariable(netid,"pairwise",toString(config.get_pairwise()) )) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_PAIRWISE);
		}
	}
	if ( !(GCI_UNDEFINED == config.get_group()) && !(GCI_NONE == config.get_group()) ) {
		if ( !setNetworkVariable(netid,"group",toString(config.get_group()) )) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_GROUP);
		}
	}
	if (!config.get_psk().isEmpty()) {
		if ( (config.get_psk().size() < 8 || config.get_psk().size() > 63) && '"' == config.get_psk()[0]) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_PSK);
		}
		else if ( !setNetworkVariable(netid,"psk",config.get_psk())) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_PSK);
		}
	}
	if (! EAPF_UNDEFINED == config.get_eapol_flags() ) {
		if ( !setNetworkVariable(netid,"eapol_flags",toString(config.get_eapol_flags()))) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_EAPOL_FLAGS);
		}
	}
	if (config.get_mixed_cell() != QOOL_UNDEFINED) {
		if ( !setNetworkVariable(netid,"mixed_cell",toNumberString(config.get_mixed_cell())) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_MIXED_CELL);
		}
	}
	if (config.get_proactive_key_caching() != QOOL_UNDEFINED) {
		if ( !setNetworkVariable(netid,"proactive_key_caching",toNumberString(config.get_proactive_key_caching())) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_PROA_KEY_CACHING);
		}
	}
	//Max. length of an wepkey in ascii is 16 (not documented, checked with wpa_supplicant 0.6.2)
	if (!config.get_wep_key0().isEmpty()) {
		if ((0 == config.get_wep_key0().indexOf("\"") && 16+2 < config.get_wep_key0().length()) || !setNetworkVariable(netid,"wep_key0",config.get_wep_key0())) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_WEP_KEY0);
		}	
	}
	if (!config.get_wep_key1().isEmpty()) {
		if ((0 == config.get_wep_key1().indexOf("\"") && 16+2 < config.get_wep_key1().length()) || !setNetworkVariable(netid,"wep_key1",config.get_wep_key1())) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_WEP_KEY1);
		}	
	}
	if (!config.get_wep_key2().isEmpty()) {
		if ((0 == config.get_wep_key2().indexOf("\"") && 16+2 < config.get_wep_key2().length()) || !setNetworkVariable(netid,"wep_key2",config.get_wep_key2())) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_WEP_KEY2);
		}	
	}
	if (!config.get_wep_key3().isEmpty()) {
		if ((0 == config.get_wep_key3().indexOf("\"") && 16+2 < config.get_wep_key3().length()) || !setNetworkVariable(netid,"wep_key3",config.get_wep_key3())) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_WEP_KEY3);
		}
	}
	if (config.get_wep_tx_keyidx() <= 3 && config.get_wep_tx_keyidx() >= 0) {
		if ( !setNetworkVariable(netid,"wep_tx_keyidx",QString::number(config.get_wep_tx_keyidx())) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_WEP_KEY_IDX);
		}
	}
	else if (config.get_wep_tx_keyidx() != -1) {
		failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_WEP_KEY_IDX);
	}
	if (config.get_peerkey() != QOOL_UNDEFINED) {
		if ( !setNetworkVariable(netid,"peerkey",toNumberString(config.get_peerkey())) ) {
			failStatus.failures = (NetconfigFailures) (failStatus.failures | NCF_PEERKEY);
		}
	}

	//Check if we have an EAP network
	if ((config.get_key_mgmt() & (KM_WPA_EAP | KM_IEEE8021X) ) || config.get_key_mgmt() == KM_UNDEFINED) {

		if (EAPM_UNDEFINED != config.get_eap()) {
			if (!setNetworkVariable(netid,"eap",toString(config.get_eap())) ) {
				failStatus.eap_failures= (EapNetconfigFailures) (failStatus.eap_failures | ENCF_EAP);
			}
		}
		if (!config.get_identity().isEmpty()) {
			if (!setNetworkVariable(netid,"identity",config.get_identity()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_IDENTITY);
			}
		}
		if (!config.get_anonymous_identity().isEmpty()) {
			if (!setNetworkVariable(netid,"anonymous_identity",config.get_anonymous_identity()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_ANON_IDENTITY);
			}
		}
		if (!config.get_password().isEmpty()) {
			if (!setNetworkVariable(netid,"password",config.get_password()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_PASSWD);
			}
		}
		if (!config.get_ca_cert().isEmpty()) {
			if (!setNetworkVariable(netid,"ca_cert",config.get_ca_cert()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_CA_CERT);
			}
		}
		if (!config.get_ca_path().isEmpty()) {
			if (!setNetworkVariable(netid,"ca_path",config.get_ca_path()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_CA_PATH);
			}
		}
		if (!config.get_client_cert().isEmpty()) {
			if (!setNetworkVariable(netid,"client_cert",config.get_client_cert()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_CLIENT_CERT);
			}
		}
		if (!config.get_private_key().isEmpty()) {
			if (!setNetworkVariable(netid,"private_key",config.get_private_key()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_PRIVATE_KEY);
			}
		}
		if (!config.get_private_key_passwd().isEmpty()) {
			if (!setNetworkVariable(netid,"private_key_passwd",config.get_private_key_passwd()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_PRIVATE_KEY_PASSWD);
			}
		}
		if (!config.get_dh_file().isEmpty()) {
			if (!setNetworkVariable(netid,"dh_file",config.get_dh_file()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_DH_FILE);
			}
		}
		if (!config.get_subject_match().isEmpty()) {
			if (!setNetworkVariable(netid,"subject_match",config.get_subject_match()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_SUBJECT_MATCH);
			}
		}
		if (!config.get_altsubject_match().isEmpty()) {
			if (!setNetworkVariable(netid,"altsubject_match",config.get_altsubject_match()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_ALTSUBJECT_MATCH);
			}
		}
		if (!config.get_phase1().isEmpty()) {
			if (!setNetworkVariable(netid,"phase1",config.get_phase1()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_PHASE1);
			}
		}
		if (!config.get_phase2().isEmpty()) {
			if (!setNetworkVariable(netid,"phase2",config.get_phase2()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_PHASE2);
			}
		}
		if (!config.get_ca_cert2().isEmpty()) {
			if (!setNetworkVariable(netid,"ca_cert2",config.get_ca_cert2()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_CA_CERT2);
			}
		}
		if (!config.get_ca_path2().isEmpty()) {
			if (!setNetworkVariable(netid,"ca_path2",config.get_ca_path2()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_CA_PATH2);
			}
		}
		if (!config.get_client_cert2().isEmpty()) {
			if (!setNetworkVariable(netid,"client_cert2",config.get_client_cert2()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_CLIENT_CERT2);
			}
		}
		if (!config.get_private_key2().isEmpty()) {
			if (!setNetworkVariable(netid,"private_key2",config.get_private_key2()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_PRIVATE_KEY2);
			}
		}
		if (!config.get_private_key2_passwd().isEmpty()) {
			if (!setNetworkVariable(netid,"private_key2_passwd",config.get_private_key2_passwd()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_PRIVATE_KEY2_PASSWD);
			}
		}
		if (!config.get_dh_file2().isEmpty()) {
			if (!setNetworkVariable(netid,"dh_file2",config.get_dh_file2()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_DH_FILE2);
			}
		}
		if (!config.get_subject_match2().isEmpty()) {
			if (!setNetworkVariable(netid,"subject_match2",config.get_subject_match2()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_SUBJECT_MATCH);
			}
		}
		if (!config.get_altsubject_match2().isEmpty()) {
			if (!setNetworkVariable(netid,"altsubject_match2",config.get_altsubject_match2()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_ALTSUBJECT_MATCH);
			}
		}
		if (-1 != config.get_fragment_size()) {
			if (!setNetworkVariable(netid,"fragment_size",QString::number(config.get_fragment_size())) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_FRAGMENT_SIZE);
			}
		}
		if (!config.get_eappsk().isEmpty()) {
			if (!setNetworkVariable(netid,"eappsk",config.get_eappsk()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_EAPPSK);
			}
		}
		if (!config.get_nai().isEmpty()) {
			if (!setNetworkVariable(netid,"nai",config.get_nai()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_NAI);
			}
		}
		if (!config.get_pac_file().isEmpty()) {
			if (!setNetworkVariable(netid,"pac_file",config.get_pac_file()) ) {
				failStatus.eap_failures = (EapNetconfigFailures) (failStatus.eap_failures | ENCF_PAC_FILE);
			}
		}
		//END OF THE EAP CONFIG PART!
	}

	if (NCF_NONE == failStatus.failures && ENCF_NONE == failStatus.eap_failures) {
		emit networkListUpdated();
	}
	return failStatus;
}

CNetworkConfig CWpaSupplicant::getNetworkConfig(int id) {
	CNetworkConfig config;
	QString response;
	
	//Check if this network is manged by us
	response = wpaCtrlCmd_GET_NETWORK(id,"id_str");
	if ("FAIL\n" != response) {
		CNetworkConfig::NetworkId netId = CNetworkConfig::toNetworkId(response);
		if (m_managedNetworks.contains(netId)) {
			config = m_managedNetworks.find(netId).value();
			return config;
		}
		config.set_id_str(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"ssid");
	if ("FAIL\n" != response) {
		config.set_ssid(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"bssid");
	if ("FAIL\n" != response) {
		config.set_bssid(libnutcommon::MacAddress(response));;
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"disabled");
	if ("FAIL\n" != response) {
		config.set_disabled(toQOOL(response));
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"scan_ssid");
	if ("FAIL\n" != response) {
		config.set_scan_ssid(toQOOL(response));
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"priority");
	if ("FAIL\n" != response) {
		config.set_priority(response.toInt());
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"mode");
	if ("FAIL\n" != response) {
		config.set_mode(toQOOL(response));
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"frequency");
	if ("FAIL\n" != response) {
		config.set_frequency(response.toInt());
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"proto");
	if ("FAIL\n" != response) {
		config.set_proto(response); 
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"key_mgmt");
	if ("FAIL\n" != response) {
		config.set_key_mgmt(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"auth_alg");
	if ("FAIL\n" != response) {
		config.set_auth_alg(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"pairwise");
	if ("FAIL\n" != response) {
		config.set_pairwise(response);
	}
	
	response = wpaCtrlCmd_GET_NETWORK(id,"group");
	if ("FAIL\n" != response) {
		config.set_group(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"psk");
	if ("FAIL\n" != response) {
		config.set_psk(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"eapol_flags");
	if ("FAIL\n" != response) {
		config.set_eapol_flags(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"mixed_cell");
	if ("FAIL\n" != response) {
		config.set_mixed_cell(toQOOL(response));
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"proactive_key_caching");
	if ("FAIL\n" != response) {
		config.set_proactive_key_caching(toQOOL(response));
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"wep_key0");
	if ("FAIL\n" != response) {
		config.set_wep_key0(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"wep_key1");
	if ("FAIL\n" != response) {
		config.set_wep_key1(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"wep_key2");
	if ("FAIL\n" != response) {
		config.set_wep_key2(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"wep_key3");
	if ("FAIL\n" != response) {
		config.set_wep_key3(response);
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"wep_tx_keyidx");
	if ("FAIL\n" != response) {
		config.set_wep_tx_keyidx(response.toInt());
		if ( (0 == config.get_wep_tx_keyidx()) && config.get_wep_key0().isEmpty() ) {
			config.set_wep_tx_keyidx(-1);
		}
	}

	response = wpaCtrlCmd_GET_NETWORK(id,"peerkey");
	if ("FAIL\n" != response) {
		config.set_peerkey(toQOOL(response));
	}

	//Check if we have Wep or plaintext:
	if ( (KM_NONE & config.get_key_mgmt()) && (-1 == config.get_wep_tx_keyidx()) ) {
		config.set_key_mgmt((KeyManagement) ((!KM_NONE & config.get_key_mgmt()) | KM_OFF));
	}

	//Check if we need to fetch wpa_settings //return now, otherwise, get the eap part
	if (! ( config.get_key_mgmt() & (KM_IEEE8021X | KM_WPA_EAP) ) ){
		return config;
	}
	
	//Get eap network config
	response = wpaCtrlCmd_GET_NETWORK(id,"eap");
	if ("FAIL\n" != response) {
		config.set_eap(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"identity");
	if ("FAIL\n" != response) {
		config.set_identity(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"anonymous_identity");
	if ("FAIL\n" != response) {
		config.set_anonymous_identity(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"password");
	if ("FAIL\n" != response) {
		config.set_password(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"ca_cert");
	if ("FAIL\n" != response) {
		config.set_ca_cert(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"ca_path");
	if ("FAIL\n" != response) {
		config.set_ca_path(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"client_cert");
	if ("FAIL\n" != response) {
		config.set_client_cert(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"private_key");
	if ("FAIL\n" != response) {
		config.set_private_key(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"private_key_passwd");
	if ("FAIL\n" != response) {
		config.set_private_key_passwd(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"dh_file");
	if ("FAIL\n" != response) {
		config.set_dh_file(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"subject_match");
	if ("FAIL\n" != response) {
		config.set_subject_match(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"altsubject_match");
	if ("FAIL\n" != response) {
		config.set_altsubject_match(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"phase1");
	if ("FAIL\n" != response) {
		config.set_phase1(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"phase2");
	if ("FAIL\n" != response) {
		config.set_phase2(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"ca_cert2");
	if ("FAIL\n" != response) {
		config.set_ca_cert2(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"ca_path2");
	if ("FAIL\n" != response) {
		config.set_ca_path2(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"client_cert2");
	if ("FAIL\n" != response) {
		config.set_client_cert2(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"private_key2");
	if ("FAIL\n" != response) {
		config.set_private_key2(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"private_key2_passwd");
	if ("FAIL\n" != response) {
		config.set_private_key2_passwd(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"dh_file2");
	if ("FAIL\n" != response) {
		config.set_dh_file2(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"subject_match2");
	if ("FAIL\n" != response) {
		config.set_subject_match2(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"altsubject_match2");
	if ("FAIL\n" != response) {
		config.set_altsubject_match2(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"fragment_size");
	if ("FAIL\n" != response) {
		bool ok;
		config.set_fragment_size(response.toInt(&ok));
		if (!ok) {
			config.set_fragment_size(-1);
		}
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"eappsk");
	if ("FAIL\n" != response) {
		config.set_eappsk(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"nai");
	if ("FAIL\n" != response) {
		config.set_nai(response);
	}
	response = wpaCtrlCmd_GET_NETWORK(id,"pac_file");
	if ("FAIL\n" != response) {
		config.set_pac_file(response);
	}
	return config;
}

}