/*
        TRANSLATOR libnutwireless::QObject
*/
#include "types.h"
#include <QDebug>

namespace libnutwireless {


GroupCiphers toGroupCiphers(ScanCiphers cip) {
	//{GCI_CCMP=2, GCI_TKIP=4, GCI_WEP104=8, GCI_WEP40=16, GCI_ALL=31} GroupCiphers;
	//{CI_UNDEFINED=0, CI_NONE=1, CI_CCMP=2, CI_TKIP=4, CI_WEP104=8, CI_WEP40=16, CI_WEP=24} ScanCiphers;
	if ((int) cip >=2) {
		return ((GroupCiphers) cip);
	}
	else {
		return GCI_DEF;
	}
}
PairwiseCiphers toPairwiseCiphers(ScanCiphers cip) {
	//{PCI_NONE=1, PCI_CCMP=2, PCI_TKIP=4, PCI_DEF=6} PairwiseCiphers;
	if ((int) cip >= 1 && (int) cip <= 7) {
		return ((PairwiseCiphers) cip);
	}
	else {
		return PCI_DEF;
	}
}
KeyManagement toKeyManagment(ScanAuthentication auth) {
	//{AUTH_PLAIN=1,AUTH_WPA_PSK=2,AUTH_WPA2_PSK=4, AUTH_WPA_EAP=8, AUTH_WPA2_EAP=16, AUTH_IEEE8021X=32}
	//{KM_NONE=1, KM_WPA_PSK=2, KM_WPA_EAP=4, KM_IEEE8021X=8}
	int key = 0;
	if (AUTH_PLAIN & auth) {
		key = (key  | KM_NONE) ;
	}
	if (AUTH_WPA_PSK & auth) {
		key = (key | KM_WPA_PSK);
	}
	if (AUTH_WPA2_PSK & auth) {
		key = (key | KM_WPA_PSK);
	}
	if (AUTH_WPA_EAP & auth) {
		key = (key | KM_WPA_EAP);
	}
	if (AUTH_WPA2_EAP & auth) {
		key = (key | KM_WPA_EAP);
	}
	if (AUTH_IEEE8021X & auth) {
		key = (key | KM_IEEE8021X);
	}
	if (AUTH_WPA_NONE & auth) {
		key = (key | KM_WPA_NONE);
	}
	if (AUTH_WPA2_NONE & auth) {
		key = (key | KM_WPA_NONE);
	}
	return ((KeyManagement) key);
}
AuthenticationAlgs toAuthAlgs(ScanAuthentication auth) {
	//{AUTHALG_UNDEFINED=0, AUTHALG_OPEN=1, AUTHALG_SHARED=2, AUTHALG_LEAP=4} AuthenticationAlgs;
	int algs = 0;
	if (AUTH_PLAIN & auth) {
		algs = (algs  | AUTHALG_SHARED) ;
	}
	if (AUTH_WPA_PSK & auth) {
		algs = (algs | AUTHALG_OPEN);
	}
	if (AUTH_WPA2_PSK & auth) {
		algs = (algs | AUTHALG_OPEN);
	}
	if (AUTH_WPA_EAP & auth) {
		algs = (algs | (AUTHALG_OPEN | AUTHALG_LEAP) );
	}
	if (AUTH_WPA2_EAP & auth) {
		algs = (algs | (AUTHALG_OPEN | AUTHALG_LEAP) );
	}
	if (AUTH_IEEE8021X & auth) {
		algs = (algs | (AUTHALG_OPEN | AUTHALG_LEAP) );
	}
	return ((AuthenticationAlgs) algs);
	
}
Protocols toProtocols(ScanAuthentication auth) {
	//{WKI_UNDEFINED=-1, WKI_WPA=1, WKI_RSN=2,WKI_DEF=3} Protocols;
	int proto = 0;
	if (AUTH_PLAIN & auth) {
		proto = (proto  | PROTO_DEFAULT) ;
	}
	if (AUTH_WPA_PSK & auth) {
		proto = (proto | PROTO_WPA);
	}
	if (AUTH_WPA2_PSK & auth) {
		proto = (proto | PROTO_RSN);
	}
	if (AUTH_WPA_EAP & auth) {
		proto = (proto | PROTO_WPA);
	}
	if (AUTH_WPA2_EAP & auth) {
		proto = (proto | PROTO_RSN);
	}
	if (AUTH_IEEE8021X & auth) {
		proto = (proto | PROTO_DEFAULT);
	}
	return ((Protocols) proto);
}

QString toString(ScanCiphers cip) {
//{CI_UNDEFINED=0, CI_NONE=1, CI_CCMP=2, CI_TKIP=4, CI_WEP104=8, CI_WEP40=16, CI_WEP=32} CIPHERS;
	QStringList ret;;
	if (CI_NONE & cip) {
		ret.append("NONE");
	}
	if (CI_CCMP & cip) {
		ret.append("CCMP");
	}
	if (CI_TKIP & cip) {
		ret.append("TKIP");
	}
	if (CI_WEP104 & cip) {
		ret.append("WEP104");
	}
	if (CI_WEP40 & cip) {
		ret.append("WEP40");
	}
	return ret.join(" ");
}

QString toString(GroupCiphers cip) {
	//{GCI_CCMP=2, GCI_TKIP=4, GCI_WEP104=8, GCI_WEP40=16, GCI_ALL=31} GroupCiphers;
	if (cip) {
		QStringList ret;
		if (cip & GCI_CCMP) {
			ret.append("CCMP");
		}
		if (cip & GCI_TKIP) {
			ret.append("TKIP");
		}
		if (cip & GCI_WEP104) {
			ret.append("WEP104");
		}
		if (cip & GCI_WEP40) {
			ret.append("WEP40");
		}
		return ret.join(" ");
	}
	return QObject::tr("UNDEFINED");
}

QString toString(PairwiseCiphers cip) {
	//{PCI_NONE=1, PCI_CCMP=2, PCI_TKIP=4} PairwiseCiphers;
	if (cip) {
		QStringList ret;
		if (cip & PCI_NONE) {
			ret.append("NONE");
		}
		if (cip & PCI_CCMP) {
			ret.append("CCMP");
		}
		if(cip & PCI_TKIP) {
			ret.append("TKIP");
		}
		return ret.join(" ");
	}
	return QObject::tr("UNDEFINED");
}

QString toString(KeyManagement keym) {
	//{KM_NONE=1, KM_WPA_PSK=2, KM_WPA_EAP=4, KM_IEEE8021X=8}
	if (keym) {
		QStringList ret;
		if (keym & KM_OFF) { //For wpa_supplicant none/off is the same (none=wep, Off=plain)
			ret.append("NONE");
		}
		if (keym & KM_NONE) {
			ret.append("NONE");
		}
		if (keym & KM_WPA_PSK) {
			ret.append("WPA-PSK");
		}
		if (keym & KM_WPA_EAP) {
			ret.append("WPA-EAP");
		}
		if (keym & KM_IEEE8021X) {
			ret.append("IEEE8021X");
		}
		if (keym & KM_WPA_NONE) {
			ret.append("WPA-NONE");
		}
		return ret.join(" ");
	}
	return QObject::tr("UNDEFINED");
}

QString toString(AuthenticationAlgs algs) {
	//{AUTHALG_UNDEFINED=0, AUTHALG_OPEN=1, AUTHALG_SHARED=2, AUTHALG_LEAP=4} AuthenticationAlgs;
	if (algs) {
		QStringList ret;
		if (AUTHALG_OPEN & algs) {
			ret.append("OPEN");
		}
		if (AUTHALG_SHARED & algs) {
			ret.append("SHARED");
		}
		if (AUTHALG_LEAP & algs) {
			ret.append("LEAP");
		}
		return ret.join(" ");
	}
	return QObject::tr("UNDEFINED");
}

QString toString(Protocols proto) {
//{WKI_UNDEFINED=0, WKI_WPA=1, WKI_RSN=2} Protocols;
	if (PROTO_RSN & PROTO_WPA)
		return "WPA RSN";
	else if (PROTO_WPA == proto)
		return "WPA";
	else if (PROTO_RSN == proto)
		return "RSN";
	else
		return QObject::tr("UNDEFINED");
}

QString toString(EapolFlags flags) {
	return QString::number((int) flags);
}

QString toString(EapMethod method) {
	//{EAP_ALL=127, EAPM_MD5=1,EAPM_MSCHAPV2=2,EAPM_OTP=4,EAPM_GTC=8,EAPM_TLS=16,EAPM_PEAP=32,EAPM_TTLS=64} EAP_METHOD;
	if (method) {
		QStringList ret;
		if (EAPM_MD5 & method) {
			ret.append("MD5");
		}
		if (EAPM_MSCHAPV2 & method) {
			ret.append("MSCHAPV2");
		}
		if (EAPM_OTP & method) {
			ret.append("OTP");
		}
		if (EAPM_GTC & method) {
			ret.append("GTC");
		}
		if (EAPM_TLS & method) {
			ret.append("TLS");
		}
		if (EAPM_PEAP & method) {
			ret.append("PEAP");
		}
		if (EAPM_TTLS & method) {
			ret.append("TTLS");
		}
		return ret.join(" ");
	}
	return QObject::tr("UNDEFINED");
}


QString toString(RequestType reqt) {
	switch (reqt) {
		case (REQ_IDENTITY):
			return QString("IDENTITY");
			break;
		case (REQ_NEW_PASSWORD):
			return QString("NEW_PASSWORD");
			break;
		case (REQ_PIN):
			return QString("PIN");
			break;
		case (REQ_OTP):
			QString("OTP");
			break;
		case (REQ_PASSPHRASE):
			QString("PASSPHRASE");
			break;
		default:
			return QString();
	}
	return QString();
}

QString toNumberString(QOOL b) {
	return ( (b == QOOL_UNDEFINED) ? "-1" : ( (b == QOOL_TRUE) ? "1" : "0")); 
}
bool toBool(QOOL b) {
	return (b == QOOL_TRUE);
}
QOOL toQOOL(bool b) {
	return ( b ? QOOL_TRUE : QOOL_FALSE);
}


//Modified iw_print_stats function from iwlib.c
//We don't care whether information was updated or not. Just convert it
WextSignal convertValues(WextRawScan &scan) {
	WextSignal res;
	//Set all non-signalquality info
	res.frequency = scan.freq;
	res.bitrates = scan.bitrates;
	res.ssid = scan.ssid;
	res.bssid = scan.bssid;
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
QString signalQualityToString(WextRawScan scan) {
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

int frequencyToChannel(int freq) {
	switch (freq) {
		//2,4GHz part 
		case 2412: return 1;
		case 2417: return 2;
		case 2422: return 3;
		case 2427: return 4;
		case 2432: return 5;
		case 2437: return 6;
		case 2442: return 7;
		case 2447: return 8;
		case 2452: return 9;
		case 2457: return 10;
		case 2462: return 11;
		case 2467: return 12;
		case 2472: return 13;
		case 2484: return 14;
		//5 GHz part
		case 5180: return 36;
		case 5200: return 40;
		case 5220: return 44;
		case 5240: return 48;
		case 5260: return 52;
		case 5280: return 56;
		case 5300: return 60;
		case 5320: return 64;
		case 5500: return 100;
		case 5520: return 104;
		case 5540: return 108;
		case 5560: return 112;
		case 5580: return 116;
		case 5600: return 120;
		case 5620: return 124;
		case 5640: return 128;
		case 5660: return 132;
		case 5680: return 136;
		case 5700: return 140;
		case 5735: return 147;
		case 5755: return 151;
		case 5775: return 155;
		case 5835: return 167;
		default: return -1;
	}
}
int channelToFrequency(int channel) {
	switch(channel) {
		case (1): return 2412;
		case (2): return 2417;
		case (3): return 2422;
		case (4): return 2427;
		case (5): return 2432;
		case (6): return 2437;
		case (7): return 2442;
		case (8): return 2447;
		case (9): return 2452;
		case (10): return 2457;
		case (11): return 2462;
		case (12): return 2467;
		case (13): return 2472;
		case (14): return 2484;
		//(5) GHz part
		case (36): return 5180;
		case (40): return 5200;
		case (44): return 5220;
		case (48): return 5240;
		case (52): return 5260;
		case (56): return 5280;
		case (60): return 5300;
		case (64): return 5320;
		case (100): return 5500;
		case (104): return 5520;
		case (108): return 5540;
		case (112): return 5560;
		case (116): return 5580;
		case (120): return 5600;
		case (124): return 5620;
		case (128): return 5640;
		case (132): return 5660;
		case (136): return 5680;
		case (140): return 5700;
		case (147): return 5735;
		case (151): return 5755;
		case (155): return 5775;
		case (167): return 5835;
		default: return -1;
	}
}


NetworkConfig::NetworkConfig() {
			//Set default values
			ssid = QString();
			bssid = libnutcommon::MacAddress();
			disabled = QOOL_UNDEFINED;
			id_str = QString();
			scan_ssid = QOOL_UNDEFINED; // (do not) scan with SSID-specific Probe Request frames
			priority = -1;
			mode = QOOL_UNDEFINED; //0 = infrastructure (Managed) mode, i.e., associate with an AP (default) 1 = IBSS (ad-hoc, peer-to-peer)
			frequency = -1; //no default, but -1 is not a working value
			protocols = PROTO_UNDEFINED; //list of accepted protocols
			keyManagement = KM_UNDEFINED; // list of accepted authenticated key management protocols
			auth_alg = AUTHALG_UNDEFINED; //list of allowed IEEE 802.11 authentication algorithms
			pairwise = PCI_UNDEFINED; //list of accepted pairwise (unicast) ciphers for WPA (CCMP,TKIP,NONE)
			group = GCI_UNDEFINED; //list of accepted group (broadcast/multicast) ciphers for WPA (CCMP;TKIP;WEP104/40)
			QString psk = QString(); //WPA preshared key; 256-bit pre-shared key
			eapol_flags = EAPF_UNDEFINED;
			mixed_cell = QOOL_UNDEFINED; //This option can be used to configure whether so called mixed
			proactive_key_caching = QOOL_UNDEFINED; //Enable/disable opportunistic PMKSA caching for WPA2.
			wep_key0 = QString(); //Static WEP key (ASCII in double quotation, hex without)
			wep_key1 = QString();
			wep_key2 = QString();
			wep_key3 = QString();
			wep_tx_keyidx = -1; //Default WEP key index (TX) (0..3)
			peerkey = QOOL_UNDEFINED; //Whether PeerKey negotiation for direct links (IEEE 802.11e DLS) is allowed.
}
NetworkConfig::NetworkConfig(ScanResult scan) {
	NetworkConfig();
	bssid = scan.bssid;
	ssid = scan.ssid;
	group = scan.group;
	pairwise = scan.pairwise;
	keyManagement = scan.keyManagement;
	protocols =  scan.protocols;
	if (scan.opmode == OPM_ADHOC) {
		frequency = scan.freq;
		mode = QOOL_TRUE;
	}
	else {
		mode = QOOL_FALSE;
	}
}

NetworkConfig::~NetworkConfig() {
}
EapNetworkConfig::EapNetworkConfig() {
			eap = EAPM_UNDEFINED; //space-separated list of accepted EAP methods
			fragment_size = -1; //Maximum EAP fragment size in bytes (default 1398);
			nai = QString(); //user NAI
}
EapNetworkConfig::~EapNetworkConfig() {}

}
