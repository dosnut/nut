#include "libnut_wpa_supplicant_types.h"

namespace libnutws {


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
	//{AUTH_PLAIN=1,AUTH_WPA_PSK=2,AUTH_WPA2_PSK=4, AUTH_WPA_EAP=8, AUTH_WPA2_EAP=16, AUTH_IEEE8021X=32}
	//{WKM_NONE=1, WKM_WPA_PSK=2, WKM_WPA_EAP=4, WKM_IEEE8021X=8} wps_key_managment;
	int key = 0;
	if (AUTH_PLAIN & auth) {
		key = (key  | WKM_NONE) ;
	}
	if (AUTH_WPA_PSK & auth) {
		key = (key | WKM_WPA_PSK);
	}
	if (AUTH_WPA2_PSK & auth) {
		key = (key | WKM_WPA_PSK);
	}
	if (AUTH_WPA_EAP & auth) {
		key = (key | WKM_WPA_EAP);
	}
	if (AUTH_WPA2_EAP & auth) {
		key = (key | WKM_WPA_EAP);
	}
	if (AUTH_IEEE8021X & auth) {
		key = (key | WKM_IEEE8021X);
	}
	return ((wps_key_management) key);
}
wps_auth_algs toAuthAlgs(wps_authentication auth) {
	//{WAA_UNDEFINED=0, WAA_OPEN=1, WAA_SHARED=2, WAA_LEAP=4} wps_auth_algs;
	int algs = 0;
	if (AUTH_PLAIN & auth) {
		algs = (algs  | WAA_SHARED) ;
	}
	if (AUTH_WPA_PSK & auth) {
		algs = (algs | WAA_OPEN);
	}
	if (AUTH_WPA2_PSK & auth) {
		algs = (algs | WAA_OPEN);
	}
	if (AUTH_WPA_EAP & auth) {
		algs = (algs | (WAA_OPEN | WAA_LEAP) );
	}
	if (AUTH_WPA2_EAP & auth) {
		algs = (algs | (WAA_OPEN | WAA_LEAP) );
	}
	if (AUTH_IEEE8021X & auth) {
		algs = (algs | (WAA_OPEN | WAA_LEAP) );
	}
	return ((wps_auth_algs) algs);
	
}
wps_protocols toProtocols(wps_authentication auth) {
	//{WKI_UNDEFINED=-1, WKI_WPA=1, WKI_RSN=2,WKI_DEF=3} wps_protocols;
	int proto = 0;
	if (AUTH_PLAIN & auth) {
		proto = (proto  | WP_DEF) ;
	}
	if (AUTH_WPA_PSK & auth) {
		proto = (proto | WP_WPA);
	}
	if (AUTH_WPA2_PSK & auth) {
		proto = (proto | WP_RSN);
	}
	if (AUTH_WPA_EAP & auth) {
		proto = (proto | WP_WPA);
	}
	if (AUTH_WPA2_EAP & auth) {
		proto = (proto | WP_RSN);
	}
	if (AUTH_IEEE8021X & auth) {
		proto = (proto | WP_DEF);
	}
	return ((wps_protocols) proto);
}

QString toString(wps_ciphers cip) {
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

QString toNumberString(wps_bool b) {
	return ( (b == WB_UNDEFINED) ? "-1" : ( (b == WB_TRUE) ? "1" : "0")); 
}
bool toBool(wps_bool b) {
	return (b == WB_TRUE);
}
wps_bool toWpsBool(bool b) {
	return ( b ? WB_TRUE : WB_FALSE);
}

//Modified iw_print_stats function from iwlib.c
//We don't care whether information was updated or not. Just convert it
wps_wext_signal_readable convertValues(wps_wext_raw_scan scan) {
	wps_wext_signal_readable res;
// 	res.encoding = WSIG_QUALITY_ALLABS;
	qDebug() << "hasRange:" << scan.hasRange;
	if ( scan.hasRange && ((scan.quality.level != 0) || (scan.quality.updated & (IW_QUAL_DBM | IW_QUAL_RCPI))) ) {
		/* Deal with quality : always a relative value */
		if ( !(scan.quality.updated & IW_QUAL_QUAL_INVALID) ) {
			res.quality.value = scan.quality.qual;
			res.quality.maximum = scan.maxquality.qual;
			qDebug() << "Converting: Quality Relative:" << res.quality.value << "from" << scan.quality.qual;
		}

		/* Check if the statistics are in RCPI (IEEE 802.11k) */
		if (scan.quality.updated & IW_QUAL_RCPI) {
		/* Deal with signal level in RCPI */
		/* RCPI = int{(Power in dBm +110)*2} for 0dbm > Power > -110dBm */
			res.type = WSR_RCPI;
			if ( !(scan.quality.updated & IW_QUAL_LEVEL_INVALID) ) {
				res.level.rcpi = ((qreal) scan.quality.level / 2.0) - 110.0;
				qDebug() << "Converting: Level RCPI:" << res.level.rcpi << "from" << scan.quality.level;
			}

			/* Deal with noise level in dBm (absolute power measurement) */
			if ( !(scan.quality.updated & IW_QUAL_NOISE_INVALID) ) {
				res.noise.rcpi = ((qreal) scan.quality.noise / 2.0) - 110.0;
				qDebug() << "Converting: NOISE RCPI:" << res.noise.rcpi << "from" << scan.quality.noise;
			}
		}
		else {
			/* Check if the statistics are in dBm */
			if ( (scan.quality.updated & IW_QUAL_DBM) || (scan.quality.level > scan.maxquality.level) ) {
				res.type = WSR_ABSOLUTE;
				/* Deal with signal level in dBm  (absolute power measurement) */
				if ( !(scan.quality.updated & IW_QUAL_LEVEL_INVALID) ) {
					/* Implement a range for dBm [-192; 63] */
					res.level.nonrcpi.value = (scan.quality.level >= 64) ? scan.quality.level - 0x100 : scan.quality.level;
					qDebug() << "Converting: LEVEL ABS:" << res.level.nonrcpi.value << "from" << scan.quality.level;
				}
			
				/* Deal with noise level in dBm (absolute power measurement) */
				if ( !(scan.quality.updated & IW_QUAL_NOISE_INVALID) ) {

					res.noise.nonrcpi.value = (scan.quality.noise >= 64) ? scan.quality.noise - 0x100 : scan.quality.noise;
					qDebug() << "Converting: NOISE ABS:" << res.noise.nonrcpi.value << "from" << scan.quality.noise;
				}
			}
			else {
				/* Deal with signal level as relative value (0 -> max) */
				res.type = WSR_RELATIVE;
				if ( !(scan.quality.updated & IW_QUAL_LEVEL_INVALID) ) {

					res.level.nonrcpi.value = scan.quality.level;
					res.level.nonrcpi.maximum = scan.maxquality.level;
					qDebug() << "Converting: LEVEL REL:" << res.level.nonrcpi.value << "/" << res.level.nonrcpi.maximum;
				}

				/* Deal with noise level as relative value (0 -> max) */
				if ( !(scan.quality.updated & IW_QUAL_NOISE_INVALID) ) {
					res.noise.nonrcpi.value = scan.quality.noise;
					res.noise.nonrcpi.maximum = scan.maxquality.noise;
					qDebug() << "Converting: NOISE REL:" << res.noise.nonrcpi.value << "/" << res.noise.nonrcpi.maximum;
				}
			}
		}
	}
	else {
		/* We can't read the range, so we don't know... */
		res.type = WSR_UNKNOWN;
		res.quality.value = scan.quality.qual;
		res.quality.maximum = scan.maxquality.qual;
		res.level.nonrcpi.value = scan.quality.level;
		res.level.nonrcpi.maximum = scan.maxquality.level;
		res.noise.nonrcpi.value = scan.quality.noise;
		res.noise.nonrcpi.maximum = scan.maxquality.noise;
		qDebug() << "CONVERTING: ALL UNKNOWN";
	}
	return res;
}
QString signalQualityToString(wps_wext_raw_scan scan) {
	char buffer[128];
	iw_range range;
	iw_quality qual;
	QString ret;
	//Set range information for iw_print_stats
	//Only the following information are needed
	range.max_qual.qual = scan.maxquality.qual;
	range.max_qual.level = scan.maxquality.level;
	range.max_qual.noise = scan.maxquality.noise;
	range.max_qual.updated = scan.maxquality.updated;
	//Set quality stats:
	qual.qual = scan.quality.qual;
	qual.level = scan.quality.level;
	qual.noise = scan.quality.noise;
	qual.updated = scan.quality.updated;
	
	iw_print_stats(buffer, sizeof(buffer),&qual,&range,scan.hasRange);
	if (qstrlen(buffer) < 128) {
		ret = QString::fromAscii(buffer);
	}
	else {
		ret = QString::fromAscii(buffer,128);
	}
	return ret;
}
QStringList signalQualityToStringList(wps_wext_raw_scan scan) {
	QString sigstr = signalQualityToString(scan);
	QStringList tmp;
	QStringList ret;
	tmp = sigstr.split(' ');
	foreach(QString i, tmp) {
		if (i.contains('=')) {
			ret.append(i.split('=')[1]);
		}
	}
	return ret;
}



wps_network_config::wps_network_config() {
			//Set default values
			ssid = QString();
			bssid = nut::MacAddress();
			disabled = WB_UNDEFINED;
			id_str = QString();
			scan_ssid = WB_UNDEFINED; // (do not) scan with SSID-specific Probe Request frames
			priority = -1;
			mode = WB_UNDEFINED; //0 = infrastructure (Managed) mode, i.e., associate with an AP (default) 1 = IBSS (ad-hoc, peer-to-peer)
			frequency = 0; //no default, but 0 is not a working value
			protocols = WP_UNDEFINED; //list of accepted protocols TODO: implement
			keyManagement = WKM_UNDEFINED; // list of accepted authenticated key management protocols
			auth_alg = WAA_UNDEFINED; //list of allowed IEEE 802.11 authentication algorithms TODO:implement
			pairwise = WPC_UNDEFINED; //list of accepted pairwise (unicast) ciphers for WPA (CCMP,TKIP,NONE)
			group = WGC_UNDEFINED; //list of accepted group (broadcast/multicast) ciphers for WPA (CCMP;TKIP;WEP104/40)
			QString psk = QString(); //WPA preshared key; 256-bit pre-shared key
			eapol_flags = EAPF_UNDEFINED;
			mixed_cell = WB_UNDEFINED; //This option can be used to configure whether so called mixed
			proactive_key_caching = WB_UNDEFINED; //Enable/disable opportunistic PMKSA caching for WPA2.
			wep_key0 = QString(); //Static WEP key (ASCII in double quotation, hex without)
			wep_key1 = QString();
			wep_key2 = QString();
			wep_key3 = QString();
			wep_tx_keyidx = -1; //Default WEP key index (TX) (0..3) TODO: implement
			peerkey = WB_UNDEFINED; //Whether PeerKey negotiation for direct links (IEEE 802.11e DLS) is allowed.
}
wps_network_config::~wps_network_config() {
}
wps_eap_network_config::wps_eap_network_config() {
			eap = EAPM_UNDEFINED; //space-separated list of accepted EAP methods TODO: implement
			fragment_size = -1; //Maximum EAP fragment size in bytes (default 1398);
			nai = QString(); //user NAI
}
wps_eap_network_config::~wps_eap_network_config() {}

}
