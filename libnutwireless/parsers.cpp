#include "parsers.h"

#include <QDebug>

namespace libnutwireless {
	
	
	//parser Functions:
	QStringList CWpaSupplicantParsers::sliceMessage(QString str) {
		return str.split('\n',QString::SkipEmptyParts);
	}
	
	//MIB Variables:
	//(dot11|dot1x)VARIABLENAME=<value>
	//<value> = (TRUE|FALSE) | <Integer> | <String> | <other?>
	//so far we do not care about <other> => <other> <-> <string>
	MIBVariables CWpaSupplicantParsers::parseMIB(QStringList list) {
		//First strip dot11 or dot1x
		//create new MIBVariable
		//append to MIBVariables
		QList<MIBVariable> mibVariable;
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
			mibVariable.append(var);
		}
		return ((MIBVariables) mibVariable);
	}
	MIBVariable::MIBVariable_type CWpaSupplicantParsers::parseMIBType(QString str) {
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
	
	NetworkFlags CWpaSupplicantParsers::parseNetworkFlags(QString str) {
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
	QList<ShortNetworkInfo> CWpaSupplicantParsers::parseListNetwork(QStringList list) {
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

	//parse config
	Protocols CWpaSupplicantParsers::parseProtocols(QString str) {
		int proto = PROTO_UNDEFINED;
		if (str.contains("RSN")) {
			proto = (proto | PROTO_RSN);
		}
		if (str.contains("WPA")) {
			proto = (proto | PROTO_WPA);
		}
		return (Protocols) proto;
	}
	
	KeyManagement CWpaSupplicantParsers::parseKeyMgmt(QString str) {
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
	
	AuthenticationAlgs CWpaSupplicantParsers::parseAuthAlg(QString str) {
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
	PairwiseCiphers CWpaSupplicantParsers::parsePairwiseCiphers(QString str) {
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
	GroupCiphers CWpaSupplicantParsers::parseGroupCiphers(QString str) {
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
	
	EapolFlags CWpaSupplicantParsers::parseEapolFlags(QString str) {
		int num = str.toInt();
		if (num >= 0 && num <= 3) {
			return (EapolFlags) num;
		}
		else {
			return EAPF_UNDEFINED;
		}
	}
	EapMethod CWpaSupplicantParsers::parseEapMethod(QString str) {
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
	Status CWpaSupplicantParsers::parseStatus(QStringList list) {
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
	Status::WPA_STATE CWpaSupplicantParsers::parseWpaState(QString str) {
		Status::WPA_STATE dummy = str;
		return dummy;
	}
	Status::PAE_STATE CWpaSupplicantParsers::parsePaeState(QString str) {
		Status::PAE_STATE dummy = str;
		return dummy;
	}
	Status::PORT_STATUS CWpaSupplicantParsers::parsePortStatus(QString str) {
		Status::PORT_STATUS dummy = str;
		return dummy;
	}
	Status::PORT_CONTROL CWpaSupplicantParsers::parsePortControl(QString str) {
		Status::PORT_CONTROL dummy = str;
		return dummy;
	}
	Status::BACKEND_STATE CWpaSupplicantParsers::parseBackendState(QString str) {
		Status::BACKEND_STATE dummy = str;
		return dummy;
	}
	Status::EAP_STATE CWpaSupplicantParsers::parseEapState(QString str) {
		Status::EAP_STATE dummy = str;
		return dummy;
	}
	Status::METHOD_STATE CWpaSupplicantParsers::parseMethodState(QString str) {
		Status::METHOD_STATE dummy = str;
		return dummy;
	}
	Status::DECISION CWpaSupplicantParsers::parseDecision(QString str) {
		Status::DECISION dummy = str;
		return dummy;
	}
	//
	InteractiveType CWpaSupplicantParsers::parseInteract(QString str) {
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
	int CWpaSupplicantParsers::parseEventNetworkId(QString str) {
		//CTRL-EVENT-CONNECTED - Connection to 00:00:00:00:00:00 completed (reauth) [id=-1 id_str=]
		int id;
		bool worked;
		int first = str.indexOf(" [id=", Qt::CaseSensitive)+5;
		int last = str.indexOf(" ", first, Qt::CaseSensitive);
		qDebug() << "EVENT DISPATCHING " << str;
		qDebug() << "EVENT DISPATCHING: " << str.mid(first,last-first);
		id = str.mid(first,last-first).toInt(&worked);
		if (worked) {
			return id;
		}
		else {
			return -1;
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
	RequestType CWpaSupplicantParsers::parseReqType(QString str) {
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
	Request CWpaSupplicantParsers::parseReq(QString str) {
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
	
	EventType CWpaSupplicantParsers::parseEvent(QString str) {
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
