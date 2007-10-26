#include "libnut_wpa_supplicant_types.h"

namespace libnut {

QString toString(wps_protocols proto) {
//{WKI_UNDEFINED=0, WKI_WPA=1, WKI_RSN=2} wps_protocols;
	return QString("%1 %2").arg(((WKI_WPA == proto) ? "WPA" : ""),((WKI_RSN == proto) ? "RSN" : ""));
}
QString toString(CIPHERS cip) {
//{CI_UNDEFINED=0, CI_NONE=1, CI_CCMP=2, CI_TKIP=4, CI_WEP104=8, CI_WEP40=16, CI_WEP=32} CIPHERS;
	QString ret = "";
	if (CI_NONE & cip) {
		ret.append("NONE ");
	}
	if (CI_CCMP & cip) {
		ret.append("CCMP ");
	}
	if (CI_TKIP & cip) {
		ret.append("TKIP ");
	}
	if (CI_WEP104 & cip) {
		ret.append("WEP104 ");
	}
	if (CI_WEP40 & cip) {
		ret.append("WEP40 ");
	}
	return ret;
}
QString toString(KEYMGMT keym) {
	//KEYMGMT_PLAIN=0, KEYMGMT_WPA_PSK=2, KEYMGMT_WPA2_PSK=4, KEYMGMT_WPA_EAP=8, KEYMGMT_WPA2_EAP=16, KEYMGMT_IEEE8021X=32, KEYMGMT_UNDEFINED=64} KEYMGMT;
	return QString();
}

QString toString(wps_auth_algs algs) {
	//{WAA_UNDEFINED=0, WAA_OPEN=1, WAA_SHARED=2, WAA_LEAP=4} wps_auth_algs;
	QString ret;
	if (WAA_OPEN & algs) {
		ret.append("OPEN ");
	}
	if (WAA_SHARED & algs) {
		ret.append("SHARED ");
	}
	if (WAA_LEAP & algs) {
		ret.append("LEAP ");
	}
	return ret;
}
QString toString(EAPOL_FLAGS flags) {
	return QString::number((int) flags);
}
QString toString(EAP_METHOD method) {
	//{EAP_ALL=127, EAPM_MD5=1,EAPM_MSCHAPV2=2,EAPM_OTP=4,EAPM_GTC=8,EAPM_TLS=16,EAPM_PEAP=32,EAPM_TTLS=64} EAP_METHOD;
	QString ret;
	if (EAPM_MD5 & method) {
		ret.append("MD5 ");
	}
	if (EAPM_MSCHAPV2 & method) {
		ret.append("MSCHAPV2 ");
	}
	if (EAPM_OTP & method) {
		ret.append("OTP ");
	}
	if (EAPM_GTC & method) {
		ret.append("GTC ");
	}
	if (EAPM_TLS & method) {
		ret.append("TLS ");
	}
	if (EAPM_PEAP & method) {
		ret.append("PEAP ");
	}
	if (EAPM_TTLS & method) {
		ret.append("TTLS ");
	}
	return ret;
}


QString toString(wps_req_type reqt) {
	switch (reqt) {
		case (WR_IDENTITY):
			return QString("IDENTITY");
			break;
		case (WR_NEW_PASSWORD):
			return QString("NEW_PASSWORD");
			break;
		case (WR_PIN):
			return QString("PIN");
			break;
		case (WR_OTP):
			QString("OTP");
			break;
		case (WR_PASSPHRASE):
			QString("PASSPHRASE");
			break;
		default:
			return QString();
	}
	return QString();
}

wps_network_config::wps_network_config() {
			//Set default values
			ssid = QString();
			bssid = nut::MacAddress();
			disabled = 0;
			id_str = QString();
			scan_ssid = 0; // (do not) scan with SSID-specific Probe Request frames
			priority = 0;
			mode = 0; //0 = infrastructure (Managed) mode, i.e., associate with an AP (default) 1 = IBSS (ad-hoc, peer-to-peer)
			frequency = 0; //no default, but 0 is not a working value
			proto = WKI_UNDEFINED; //list of accepted protocols TODO: implement
			key_mgmt = KEYMGMT_UNDEFINED; // list of accepted authenticated key management protocols
			auth_alg = WAA_UNDEFINED; //list of allowed IEEE 802.11 authentication algorithms TODO:implement
			pairwise = CI_UNDEFINED; //list of accepted pairwise (unicast) ciphers for WPA (CCMP,TKIP,NONE)
			group = CI_UNDEFINED; //list of accepted group (broadcast/multicast) ciphers for WPA (CCMP;TKIP;WEP104/40)
			QString psk = QString(); //WPA preshared key; 256-bit pre-shared key
			eapol_flags = (EAPOL_FLAGS) (EAP_DYN_UNICAST_WEP | EAP_BCAST_WEP);
			mixed_cell = false; //This option can be used to configure whether so called mixed
			proactive_key_caching = false; //Enable/disable opportunistic PMKSA caching for WPA2.
			wep_key0 = QString(); //Static WEP key (ASCII in double quotation, hex without)
			wep_key1 = QString();
			wep_key2 = QString();
			wep_key3 = QString();
			wep_tx_keyidx = WKI_UNDEFINED; //Default WEP key index (TX) (0..3) TODO: implement
			peerkey = false; //Whether PeerKey negotiation for direct links (IEEE 802.11e DLS) is allowed.
}
wps_network_config::~wps_network_config() {
}
}
