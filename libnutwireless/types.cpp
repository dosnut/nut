/*
        TRANSLATOR libnutwireless::QObject
*/
#include "types.h"
#include <QDebug>
namespace libnutwireless {

Protocols toProtocols(QString str) {
	int proto = PROTO_UNDEFINED;
	if (str.contains("RSN")) {
		proto = (proto | PROTO_RSN);
	}
	if (str.contains("WPA")) {
		proto = (proto | PROTO_WPA);
	}
	return (Protocols) proto;
}

KeyManagement toKeyMgmt(QString str) {
	int key = KM_UNDEFINED;
	//WPA-NONE has to checked first. If Set, othe options should not be possible
	if (str.contains("WPA-NONE")) {
		key = (key | KM_WPA_NONE);
	}
	int idx = str.indexOf("NONE");
	if (idx == 0) { //Check if it really is NONE at not WPA-NONE
		key = (key | KM_NONE);
	}
	else if (0 < idx && '-' != str[idx-1]) {
		key = (key | KM_NONE);
	}
	if (str.contains("WPA-PSK")) {
		key = (key | KM_WPA_PSK);
	}
	if (str.contains("WPA-EAP")) {
		key = (key | KM_WPA_EAP);
	}
	if (str.contains("IEEE8021X")) {
		key = (key | KM_IEEE8021X);
	}
	return (KeyManagement) key;
}

AuthenticationAlgs toAuthAlg(QString str) {
	int auth = AUTHALG_UNDEFINED;
	if (str.contains("OPEN")) {
		auth = (auth | AUTHALG_OPEN);
	}
	if (str.contains("SHARED")) {
		auth = (auth | AUTHALG_SHARED);
	}
	if (str.contains("LEAP")) {
		auth = (auth | AUTHALG_LEAP);
	}
	return (AuthenticationAlgs) auth;
}
PairwiseCiphers toPairwiseCiphers(QString str) {
	int cip = PCI_UNDEFINED;
	if (str.contains("NONE")) {
		cip = (cip | PCI_NONE);
	}
	if (str.contains("CCMP")) {
		cip = (cip | PCI_CCMP);
	}
	if (str.contains("TKIP")) {
		cip = (cip | PCI_TKIP);
	}
	return (PairwiseCiphers) cip;
}
GroupCiphers toGroupCiphers(QString str) {
	int cip = GCI_UNDEFINED;
	if (str.contains("CCMP")) {
		cip = (cip | GCI_CCMP);
	}
	if (str.contains("TKIP")) {
		cip = (cip | GCI_TKIP);
	}
	if (str.contains("WEP104")) {
		cip = (cip | GCI_WEP104);
	}
	if (str.contains("WEP40")) {
		cip = (cip | GCI_WEP40);
	}
	return (GroupCiphers) cip;
}

EapolFlags toEapolFlags(QString str) {
	int num = str.toInt();
	if (num >= 0 && num <= 3) {
		return (EapolFlags) num;
	}
	else {
		return EAPF_UNDEFINED;
	}
}
EapMethod toEapMethod(QString str) {
	//{EAPM_UNDEFINED=0, EAPM_MD5=1,EAPM_MSCHAPV2=2,EAPM_OTP=4,EAPM_GTC=8,EAPM_TLS=16,EAPM_PEAP=32,EAPM_TTLS=64,EAPM_ALL=127} EapMethod;
	int method = EAPM_UNDEFINED;
	if (str.contains("AKA")) {
		method = (method | EAPM_AKA);
	}
	if (str.contains("FAST")) {
		method = (method | EAPM_FAST);
	}
	if (str.contains("LEAP")) {
		method = (method | EAPM_LEAP);
	}
	if (str.contains("MSCHAPV2")) {
		method = (method | EAPM_MSCHAPV2);
	}
	if (str.contains("OTP")) {
		method = (method | EAPM_OTP);
	}
	if (str.contains("GTC")) {
		method = (method | EAPM_GTC);
	}
	if (str.contains("TLS")) {
		method = (method | EAPM_TLS);
	}
	if (str.contains("PEAP")) {
		method = (method | EAPM_PEAP);
	}
	if (str.contains("TTLS")) {
		method = (method | EAPM_TTLS);
	}
	return (EapMethod) method;
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
	return QString();
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
	return QString();
}

QString toString(Protocols proto) {
//{WKI_UNDEFINED=0, WKI_WPA=1, WKI_RSN=2} Protocols;
	if ((PROTO_RSN | PROTO_WPA) == proto)
		return "WPA RSN";
	else if (PROTO_WPA == proto)
		return "WPA";
	else if (PROTO_RSN == proto)
		return "RSN";
	else
		return QString();
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
	return QString();
}


QString toNumberString(QOOL b) {
	return ( (b == QOOL_UNDEFINED) ? "-1" : ( (b == QOOL_TRUE) ? "1" : "0")); 
}
QString toString(QOOL b) {
	return ( (b == QOOL_UNDEFINED) ? "undefined" : ( (b == QOOL_TRUE) ? "true" : "false")); 
}
bool toBool(QOOL b) {
	return (b == QOOL_TRUE);
}
QOOL toQOOL(bool b) {
	return ( b ? QOOL_TRUE : QOOL_FALSE);
}
QOOL toQOOL(int i) {
	return ( i < 0 ? QOOL_UNDEFINED : ( i == 0 ? QOOL_FALSE : QOOL_TRUE)); 
}

}