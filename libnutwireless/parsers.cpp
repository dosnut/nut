#include "parsers.h"

namespace libnutwireless {
	
	
	//parser Functions:
	QStringList CWpa_SupplicantParsers::sliceMessage(QString str) {
		return str.split('\n',QString::SkipEmptyParts);
	}
	
	//MIB Variables:
	//(dot11|dot1x)VARIABLENAME=<value>
	//<value> = (TRUE|FALSE) | <Integer> | <String> | <other?>
	//so far we do not care about <other> => <other> <-> <string>
	MIBVariables CWpa_SupplicantParsers::parseMIB(QStringList list) {
		//First strip dot11 or dot1x
		//create new MIBVariable
		//append to MIBVariables
		QList<MIBVariable> wpsMIB;
		QStringList tmp_strlist;
		MIBVariable var;
		foreach(QString str, list) {
			tmp_strlist = (str.remove(0,5)).split('=',QString::KeepEmptyParts);
			var.name = tmp_strlist[0];
			var.type = parseMIBType(tmp_strlist[1]);
			var.value.num = 0;
			if (MIBVariable::PLAIN == var.type || MIBVariable::STRING == var.type) {
				var.value.str = new QString(tmp_strlist[0]);
			}
			if (MIBVariable::NUMBER == var.type) {
				bool ok = true;
				var.value.num = new qint32(tmp_strlist[1].toInt(&ok));
				if (!ok) {
					*(var.value.num) = -1;
				}
			}
			if (MIBVariable::LOGIC == var.type) {
				var.value.logic = new bool((tmp_strlist[1] == "TRUE"));
			}
			wpsMIB.append(var);
		}
		return ((MIBVariables) wpsMIB);
	}
	MIBVariable::MIBVariable_type CWpa_SupplicantParsers::parseMIBType(QString str) {
		bool ok;
		str.toInt(&ok);
		if (ok) {
			return MIBVariable::NUMBER;
		}
		if ( (0 == str.indexOf("TRUE")) || (0 == str.indexOf("FALSE")) ) {
			return MIBVariable::LOGIC;
		}
		if (str.contains(":") || str.contains("-")) {
			return MIBVariable::PLAIN;
		}
		return MIBVariable::STRING;
	}
	
	NetworkFlags CWpa_SupplicantParsers::parseNetworkFlags(QString str) {
		if (str.contains("CURRENT",Qt::CaseInsensitive)) {
			return NF_CURRENT;
		}
		if (str.contains("DISABLED",Qt::CaseInsensitive)) {
			return NF_DISABLED;
		}
		return NF_NONE;
	}
	//network id / ssid / bssid / flags
	//0 example network	any	[CURRENT]
	QList<ShortNetworkInfo> CWpa_SupplicantParsers::parseListNetwork(QStringList list) {
		list.removeFirst();
		QList<ShortNetworkInfo> networks;
		QStringList line;
		ShortNetworkInfo net;
		bool worked = true;
		foreach(QString str, list) {
			line = str.split('\t',QString::KeepEmptyParts);
			net.id = line[0].toInt(&worked);
			if (!worked) {
				worked = true;
				continue;
			}
			net.ssid = line[1];
			net.bssid = libnutcommon::MacAddress(line[2]);
			net.flags = parseNetworkFlags(line[3]);
			networks.append(net);
		}
		return networks;
	}
	
	
	/* Scan results:
	bssid / frequency / signal level / flags / ssid
	00:09:5b:95:e0:4e	2412	208	[WPA-PSK-CCMP]	jkm private
	02:55:24:33:77:a3	2462	187	[WPA-PSK-TKIP]	testing
	00:09:5b:95:e0:4f	2412	209		jkm guest
	*/
	ScanCiphers CWpa_SupplicantParsers::parseScanCiphers(QString str) {
		ScanCiphers cip = CI_NONE;
		if (str.contains("CCMP")) {
			cip = (ScanCiphers) (cip | CI_CCMP);
		}
		if (str.contains("TKIP")) {
			cip = (ScanCiphers) (cip | CI_TKIP);
		}
		if (str.contains("WEP104")) {
			cip = (ScanCiphers) (cip | CI_WEP104);
		}
		if (str.contains("WEP40")) {
			cip = (ScanCiphers) (cip | CI_WEP40);
		}
		if (str.contains("WEP")) {
			cip = (ScanCiphers) (cip | CI_WEP);
		}
		return cip;
	}
	ScanAuthentication CWpa_SupplicantParsers::parseScanAuth(QString str) {
		int key = AUTH_UNDEFINED;
		if (str.contains("WPA-PSK")) {
			key = (key | AUTH_WPA_PSK);
		}
		if (str.contains("WPA2-EAP")) {
			key = (key | AUTH_WPA2_EAP);
		}
		if (str.contains("WPA2-PSK")) {
			key = (key | AUTH_WPA2_PSK);
		}
		if (str.contains("WPA-EAP")) {
			key = (key | AUTH_WPA_EAP);
		}
		if (str.contains("IEEE8021X")) {
			key = (key | AUTH_IEEE8021X);
		}
		if (str.contains("WPA-NONE")) {
			key = (key | AUTH_WPA_NONE);
		}
		if (str.contains("WPA2-NONE")) {
			key = (key | AUTH_WPA2_NONE);
		}
		return (ScanAuthentication) key;
	}
	
	QList<ScanResult> CWpa_SupplicantParsers::parseScanResult(QStringList list) {
		list.removeFirst();
		QList<ScanResult> scanresults;
		QStringList line;
		ScanResult scanresult;
		bool worked = true;
		ScanAuthentication scanAuth;
		foreach(QString str, list) {
			line = str.split('\t',QString::KeepEmptyParts);
			scanresult.bssid = libnutcommon::MacAddress(line[0]);
			scanresult.freq = line[1].toInt(&worked);
			if (!worked) {
				worked = true;
				continue;
			}
			//Quality will come directly from wext
			scanAuth = parseScanAuth(line[3]);
			scanresult.ciphers = parseScanCiphers(line[3]);
			scanresult.protocols = toProtocols(scanAuth);
			scanresult.keyManagement = toKeyManagment(scanAuth);
			scanresult.ssid = line[4];
			scanresults.append(scanresult);
		}
		return scanresults;
	}
	
	//parse config
	Protocols CWpa_SupplicantParsers::parseProtocols(QString str) {
		int proto = PROTO_UNDEFINED;
		if (str.contains("RSN")) {
			proto = (proto | PROTO_RSN);
		}
		if (str.contains("WPA")) {
			proto = (proto | PROTO_WPA);
		}
		return (Protocols) proto;
	}
	
	KeyManagement CWpa_SupplicantParsers::parseKeyMgmt(QString str) {
		int key = KM_UNDEFINED;
		if (str.contains("NONE")) {
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
	
	AuthenticationAlgs CWpa_SupplicantParsers::parseAuthAlg(QString str) {
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
	PairwiseCiphers CWpa_SupplicantParsers::parsePairwiseCiphers(QString str) {
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
	GroupCiphers CWpa_SupplicantParsers::parseGroupCiphers(QString str) {
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
	
	EapolFlags CWpa_SupplicantParsers::parseEapolFlags(QString str) {
		int num = str.toInt();
		if (num >= 0 && num <= 3) {
			return (EapolFlags) num;
		}
		else {
			return EAPF_UNDEFINED;
		}
	}
	EapMethod CWpa_SupplicantParsers::parseEapMethod(QString str) {
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
	
	
	//Parse status:
	/*
	bssid=02:00:01:02:03:04
	ssid=test network
	id=0
	pairwise_cipher=CCMP
	group_cipher=CCMP
	key_mgmt=WPA-PSK
	wpa_state=COMPLETED
	ip_address=192.168.1.21
	Supplicant PAE state=AUTHENTICATED
	suppPortStatus=Authorized
	heldPeriod=60
	authPeriod=30
	startPeriod=30
	maxStart=3
	portControl=Auto
	Supplicant Backend state=IDLE
	EAP state=SUCCESS
	reqMethod=0
	methodState=NONE
	decision=COND_SUCC
	ClientTimeout=60
	*/
	//Always parse as if status verbose
	Status CWpa_SupplicantParsers::parseStatus(QStringList list) {
		Status status;
		bool ok = true;
		foreach(QString str, list) {
			if (0 == str.indexOf("bssid=")) {
				status.bssid = libnutcommon::MacAddress(str.split('=',QString::KeepEmptyParts)[1]);
				continue;
			}
			if (0 == str.indexOf("ssid=")) {
				status.ssid = str.split('=',QString::KeepEmptyParts)[1];
				continue;
			}
			if (0 == str.indexOf("id=")) {
				status.id = (str.split('=',QString::KeepEmptyParts)[1]).toInt(&ok);
				if (!ok)  {
					status.id = -1;
					ok = true;
				}
				continue;
			}
			if (0 == str.indexOf("pairwise_cipher=")) {
				status.pairwise_cipher = parsePairwiseCiphers(str.split('=',QString::KeepEmptyParts)[1]);
				continue;
			}
			if (0 == str.indexOf("group_cipher=")) {
				status.group_cipher = parseGroupCiphers((str.split('=',QString::KeepEmptyParts)[1]));
				continue;
			}
			if (0 == str.indexOf("key_mgmt=")) {
				status.key_mgmt = parseKeyMgmt(str.split('=',QString::KeepEmptyParts)[1]);
				continue;
			}
			if (0 == str.indexOf("wpa_state=")) {
				status.wpa_state = parseWpaState(str.split('=',QString::KeepEmptyParts)[1]);
				continue;
			}
			if (0 == str.indexOf("ip_address=")) {
				status.ip_address = QHostAddress(str.split('=',QString::KeepEmptyParts)[1]);
				continue;
			}
			if (0 == str.indexOf("Supplicant PAE state=")) {
				status.pae_state = parsePaeState(str.split('=',QString::KeepEmptyParts)[1]);
				continue;
			}
			if (0 == str.indexOf("suppPortStatus=")) {
				status.PortStatus = parsePortStatus(str.split('=',QString::KeepEmptyParts)[1]);
				continue;
			}
			if (0 == str.indexOf("heldPeriod=")) {
				status.heldPeriod = (str.split('=',QString::KeepEmptyParts)[1]).toInt(&ok);
				if (!ok)  {
					status.heldPeriod = -1;
					ok = true;
				}
				continue;
			}
			if (0 == str.indexOf("authPeriod=")) {
				status.authPeriod = (str.split('=',QString::KeepEmptyParts)[1]).toInt(&ok);
				if (!ok)  {
					status.authPeriod = -1;
					ok = true;
				}
				continue;
			}
			if (0 == str.indexOf("startPeriod=")) {
				status.startPeriod = (str.split('=',QString::KeepEmptyParts)[1]).toInt(&ok);
				if (!ok)  {
					status.startPeriod = -1;
					ok = true;
				}
				continue;
			}
			if (0 == str.indexOf("maxStart=")) {
				status.maxStart = (str.split('=',QString::KeepEmptyParts)[1]).toInt(&ok);
				if (!ok)  {
					status.maxStart = -1;
					ok = true;
				}
				continue;
			}
			if (0 == str.indexOf("portControl=")) {
				status.portControl = parsePortControl(str.split('=',QString::KeepEmptyParts)[1]);
				continue;
			}
			if (0 == str.indexOf("Supplicant Backend state=")) {
				status.backend_state = parseBackendState(str.split('=',QString::KeepEmptyParts)[1]);
				continue;
			}
			if (0 == str.indexOf("EAP state=")) {
				status.eap_state = parseEapState(str.split('=',QString::KeepEmptyParts)[1]);
				continue;
			}
			if (0 == str.indexOf("reqMethod=")) {
				status.reqMethod = (str.split('=',QString::KeepEmptyParts)[1]).toInt(&ok);
				if (!ok)  {
					status.reqMethod = -1;
					ok = true;
				}
				continue;
			}
			if (0 == str.indexOf("methodState=")) {
				status.methodState = parseMethodState(str.split('=',QString::KeepEmptyParts)[1]);
				continue;
			}
			if (0 == str.indexOf("decision=")) {
				status.decision = parseDecision(str.split('=',QString::KeepEmptyParts)[1]);
				continue;
			}
			if (0 == str.indexOf("ClientTimeout=")) {
				status.ClientTimeout = (str.split('=',QString::KeepEmptyParts)[1]).toInt(&ok);
				if (!ok)  {
					status.ClientTimeout = -1;
					ok = true;
				}
				continue;
			}
		}
		return status;
	}
	//parseStatus helper parsers (that's crazy)
	//So far they dont really parse
	Status::WPA_STATE CWpa_SupplicantParsers::parseWpaState(QString str) {
		Status::WPA_STATE dummy = str;
		return dummy;
	}
	Status::PAE_STATE CWpa_SupplicantParsers::parsePaeState(QString str) {
		Status::PAE_STATE dummy = str;
		return dummy;
	}
	Status::PORT_STATUS CWpa_SupplicantParsers::parsePortStatus(QString str) {
		Status::PORT_STATUS dummy = str;
		return dummy;
	}
	Status::PORT_CONTROL CWpa_SupplicantParsers::parsePortControl(QString str) {
		Status::PORT_CONTROL dummy = str;
		return dummy;
	}
	Status::BACKEND_STATE CWpa_SupplicantParsers::parseBackendState(QString str) {
		Status::BACKEND_STATE dummy = str;
		return dummy;
	}
	Status::EAP_STATE CWpa_SupplicantParsers::parseEapState(QString str) {
		Status::EAP_STATE dummy = str;
		return dummy;
	}
	Status::METHOD_STATE CWpa_SupplicantParsers::parseMethodState(QString str) {
		Status::METHOD_STATE dummy = str;
		return dummy;
	}
	Status::DECISION CWpa_SupplicantParsers::parseDecision(QString str) {
		Status::DECISION dummy = str;
		return dummy;
	}
	//
	InteractiveType CWpa_SupplicantParsers::parseInteract(QString str) {
		if (str.contains("CTRL-EVENT")) {
			return INTERACT_EVENT;
		}
		else if (str.contains("CTRL-REQ")) {
			return INTERACT_REQ;
		}
		else {
			return INTERACT_MSG;
		}
	}
	/*
	IDENTITY (EAP identity/user name)
	PASSWORD (EAP password)
	NEW_PASSWORD (New password if the server is requesting password change)
	PIN (PIN code for accessing a SIM or smartcard)
	OTP (one-time password; like password, but the value is used only once)
	PASSPHRASE (passphrase for a private key file)
	//CTRL-REQ-<field name>-<network id>-<human readable text>
	*/
	RequestType CWpa_SupplicantParsers::parseReqType(QString str) {
		if (str.contains("IDENTITY")) {
			return REQ_IDENTITY;
		}
		if (str.contains("PASSWORD")) {
			return REQ_PASSWORD;
		}
		if (str.contains("NEW_PASSWORD")) {
			return REQ_NEW_PASSWORD;
		}
		if (str.contains("PIN")) {
			return REQ_PIN;
		}
		if (str.contains("OTP")) {
			return REQ_OTP;
		}
		if (str.contains("PASSPHRASE")) {
			return REQ_PASSPHRASE;
		}
		return REQ_FAIL;
	}
	Request CWpa_SupplicantParsers::parseReq(QString str) {
		bool ok = true;
		Request req;
		//Check request type:
		req.type = parseReqType(str);
		//get network id:
		req.id = ((str.split('-',QString::KeepEmptyParts))[2]).toInt(&ok);
		if (!ok) {
			req.id = -1;
		}
		return req;
	}
	
	EventType CWpa_SupplicantParsers::parseEvent(QString str) {
		if (str.contains("CONNECTED") ) {
			return EVENT_CONNECTED;
		}
		if (str.contains("DISCONNECTED") ) {
			return EVENT_DISCONNECTED;
		}
		if (str.contains("TERMINATING")) {
			return EVENT_TERMINATING;
		}
		if (str.contains("PASSWORD_CHANGED")) {
			return EVENT_PASSWORD_CHANGED;
		}
		if (str.contains("WEAP_NOTIFICATION") ){
			return EVENT_EAP_NOTIFICATION;
		}
		if (str.contains("EAP_STARTED") ) {
			return EVENT_EAP_STARTED;
		}
		if (str.contains("EAP_METHOD") ) {
			return EVENT_EAP_METHOD;
		}
		if (str.contains("EAP_SUCCESS") ) {
			return EVENT_EAP_SUCCESS;
		}
		if (str.contains("EAP_FAILURE") ) {
			return 	EVENT_EAP_FAILURE;
		}
		return EVENT_OTHER;
	}
}
