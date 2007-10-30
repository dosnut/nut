#include "libnut_wpa_supplicant_types.h"

namespace libnut {


wps_group_ciphers toGroupCiphers(wps_ciphers cip) {
	//{GCI_CCMP=2, GCI_TKIP=4, GCI_WEP104=8, GCI_WEP40=16, GCI_ALL=31} wps_group_ciphers;
	//{CI_UNDEFINED=0, CI_NONE=1, CI_CCMP=2, CI_TKIP=4, CI_WEP104=8, CI_WEP40=16, CI_WEP=24} wps_ciphers;
	if ((int) cip >=2) {
		return ((wps_group_ciphers) cip);
	}
	else {
		return WGC_DEF;
	}
}
wps_pairwise_ciphers toPairwiseCiphers(wps_ciphers cip) {
	//{PCI_NONE=1, PCI_CCMP=2, PCI_TKIP=4, PCI_DEF=6} wps_pairwise_ciphers;
	if ((int) cip >= 1 && (int) cip <= 7) {
		return ((wps_pairwise_ciphers) cip);
	}
	else {
		return WPC_DEF;
	}
}
wps_key_management toKeyManagment(wps_authentication auth) {
	//{WA_PLAIN=1,WA_WPA_PSK=2,WA_WPA2_PSK=4, WA_WPA_EAP=8, WA_WPA2_EAP=16, WA_IEEE8021X=32}
	//{WKM_NONE=1, WKM_WPA_PSK=2, WKM_WPA_EAP=4, WKM_IEEE8021X=8} wps_key_managment;
	int key = 0;
	if (WA_PLAIN & auth) {
		key = (key  | WKM_NONE) ;
	}
	if (WA_WPA_PSK & auth) {
		key = (key | WKM_WPA_PSK);
	}
	if (WA_WPA2_PSK & auth) {
		key = (key | WKM_WPA_PSK);
	}
	if (WA_WPA_EAP & auth) {
		key = (key | WKM_WPA_EAP);
	}
	if (WA_WPA2_EAP & auth) {
		key = (key | WKM_WPA_EAP);
	}
	if (WA_IEEE8021X & auth) {
		key = (key | WKM_IEEE8021X);
	}
	return ((wps_key_management) key);
}
wps_auth_algs toAuthAlgs(wps_authentication auth) {
	//{WAA_UNDEFINED=0, WAA_OPEN=1, WAA_SHARED=2, WAA_LEAP=4} wps_auth_algs;
	int algs = 0;
	if (WA_PLAIN & auth) {
		algs = (algs  | WAA_SHARED) ;
	}
	if (WA_WPA_PSK & auth) {
		algs = (algs | WAA_OPEN);
	}
	if (WA_WPA2_PSK & auth) {
		algs = (algs | WAA_OPEN);
	}
	if (WA_WPA_EAP & auth) {
		algs = (algs | (WAA_OPEN | WAA_LEAP) );
	}
	if (WA_WPA2_EAP & auth) {
		algs = (algs | (WAA_OPEN | WAA_LEAP) );
	}
	if (WA_IEEE8021X & auth) {
		algs = (algs | (WAA_OPEN | WAA_LEAP) );
	}
	return ((wps_auth_algs) algs);
	
}
wps_protocols toProtocols(wps_authentication auth) {
	//{WKI_UNDEFINED=-1, WKI_WPA=1, WKI_RSN=2,WKI_DEF=3} wps_protocols;
	int proto = 0;
	if (WA_PLAIN & auth) {
		proto = (proto  | WP_DEF) ;
	}
	if (WA_WPA_PSK & auth) {
		proto = (proto | WP_WPA);
	}
	if (WA_WPA2_PSK & auth) {
		proto = (proto | WP_RSN);
	}
	if (WA_WPA_EAP & auth) {
		proto = (proto | WP_WPA);
	}
	if (WA_WPA2_EAP & auth) {
		proto = (proto | WP_RSN);
	}
	if (WA_IEEE8021X & auth) {
		proto = (proto | WP_DEF);
	}
	return ((wps_protocols) proto);
}

QString toString(wps_ciphers cip) {
//{CI_UNDEFINED=0, CI_NONE=1, CI_CCMP=2, CI_TKIP=4, CI_WEP104=8, CI_WEP40=16, CI_WEP=32} CIPHERS;
	QString ret = "";
	if (WC_NONE & cip) {
		ret.append("NONE ");
	}
	if (WC_CCMP & cip) {
		ret.append("CCMP ");
	}
	if (WC_TKIP & cip) {
		ret.append("TKIP ");
	}
	if (WC_WEP104 & cip) {
		ret.append("WEP104 ");
	}
	if (WC_WEP40 & cip) {
		ret.append("WEP40 ");
	}
	return ret;
}
QString toString(wps_group_ciphers cip) {
	//{GCI_CCMP=2, GCI_TKIP=4, GCI_WEP104=8, GCI_WEP40=16, GCI_ALL=31} wps_group_ciphers;
	QString ret;
	if (cip & WGC_CCMP) {
		ret.append("CCMP ");
	}
	if (cip & WGC_TKIP) {
		ret.append("TKIP ");
	}
	if (cip & WGC_WEP104) {
		ret.append("WEP104 ");
	}
	if (cip & WGC_WEP40) {
		ret.append("WEP40");
	}
	return ret;
}
QString toString(wps_pairwise_ciphers cip) {
	//{PCI_NONE=1, PCI_CCMP=2, PCI_TKIP=4} wps_pairwise_ciphers;
	QString ret;
	if (cip & WPC_NONE) {
		ret.append("NONE ");
	}
	if (cip & WPC_CCMP) {
		ret.append("CCMP");
	}
	if(cip & WPC_TKIP) {
		ret.append("TKIP");
	}
	return ret;
}
QString toString(wps_key_management keym) {
	//{WKM_NONE=1, WKM_WPA_PSK=2, WKM_WPA_EAP=4, WKM_IEEE8021X=8} wps_key_managment;
	QString ret;
	if (keym & WKM_NONE) {
		ret.append("NONE ");
	}
	if (keym & WKM_WPA_PSK) {
		ret.append("WPA-PSK ");
	}
	if (keym & WKM_WPA_EAP) {
		ret.append("WPA-EAP");
	}
	if (keym & WKM_IEEE8021X) {
		ret.append("IEEE8021X");
	}
	return ret;
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

QString toString(wps_protocols proto) {
//{WKI_UNDEFINED=0, WKI_WPA=1, WKI_RSN=2} wps_protocols;
	return QString("%1 %2").arg(((WP_WPA == proto) ? "WPA" : ""),((WP_RSN == proto) ? "RSN" : ""));
}

QString toString(wps_eapol_flags flags) {
	return QString::number((int) flags);
}

QString toString(wps_eap_method method) {
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
			scan_ssid = false; // (do not) scan with SSID-specific Probe Request frames
			priority = -1;
			mode = false; //0 = infrastructure (Managed) mode, i.e., associate with an AP (default) 1 = IBSS (ad-hoc, peer-to-peer)
			frequency = 0; //no default, but 0 is not a working value
			protocols = WP_UNDEFINED; //list of accepted protocols TODO: implement
			keyManagement = WKM_UNDEFINED; // list of accepted authenticated key management protocols
			auth_alg = WAA_UNDEFINED; //list of allowed IEEE 802.11 authentication algorithms TODO:implement
			pairwise = WPC_UNDEFINED; //list of accepted pairwise (unicast) ciphers for WPA (CCMP,TKIP,NONE)
			group = WGC_UNDEFINED; //list of accepted group (broadcast/multicast) ciphers for WPA (CCMP;TKIP;WEP104/40)
			QString psk = QString(); //WPA preshared key; 256-bit pre-shared key
			eapol_flags = EAPF_UNDEFINED;
			mixed_cell = false; //This option can be used to configure whether so called mixed
			proactive_key_caching = false; //Enable/disable opportunistic PMKSA caching for WPA2.
			wep_key0 = QString(); //Static WEP key (ASCII in double quotation, hex without)
			wep_key1 = QString();
			wep_key2 = QString();
			wep_key3 = QString();
			wep_tx_keyidx = -1; //Default WEP key index (TX) (0..3) TODO: implement
			peerkey = false; //Whether PeerKey negotiation for direct links (IEEE 802.11e DLS) is allowed.
}
wps_network_config::~wps_network_config() {
}
}
