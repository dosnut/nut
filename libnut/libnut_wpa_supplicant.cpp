#include "libnut_wpa_supplicant.h"
#include <QDebug>

namespace libnut {

//CWpa_supplicant


//Wpa_supplicant control commands:
//WARNING!!!! THIS MAY HAVE SOME MEMORY LEAKS!!! Check if char * is deleted in wpa_ctrl_request
QString CWpa_Supplicant::wps_ctrl_command(QString cmd = "PING") {
	//int wpa_ctrl_request(struct wpa_ctrl *ctrl, const char *cmd, size_t cmd_len,
	//	     char *reply, size_t *reply_len,
	//	     void (*msg_cb)(char *msg, size_t len));
	//	     
	//Check if we have a control interface:
	if (cmd_ctrl == NULL || event_ctrl == NULL) {
		return QString();
	}
	if (!wps_connected or inConnectionPhase) {
		return QString();
	}
	//First Check if wpa_supplicant is running:
	size_t command_len;
	const char * command;
	char reply[4096];
	size_t reply_len = sizeof(reply);
	
	command = "PING";
	command_len = strlen(command);
	int status = wpa_ctrl_request(cmd_ctrl, command, command_len, reply, &reply_len,NULL);
	if ( (status != 0) or (QString::fromUtf8(reply, reply_len) != "PONG\n") ) {
		qDebug() << QString("(status=%2)PING COMMAND RESPONSE: %1").arg(QString::fromUtf8(reply, reply_len),QString::number(status));
		//TODO: Maybe we need to detach and close wpa_supplicant:
		wps_close("wps_ctrl_command/nopong");
		return QString();
	}
	if (cmd != "PING") {
		size_t reply_len = sizeof(reply);
		command = cmd.toAscii().data();
		command_len = cmd.toAscii().size();
		
		status = wpa_ctrl_request(cmd_ctrl, command, command_len, reply, &reply_len,NULL);
		if (0 == status) {
			qDebug() << cmd << ":" << QString::fromUtf8(reply, reply_len) << "\nEOC";
			if (reply_len > 0) {
				//TODO:Check if reply is \0 terminated
				return QString::fromUtf8(reply, reply_len);
			}
			else {
				return QString();
			}
		}
		else {
			return QString();
		}
	}
	else { //PING command requested
		qDebug() << cmd << ":" << QString::fromUtf8(reply, reply_len) << "EOC";
		return QString::fromUtf8(reply, reply_len);
	}
}

//parser Functions:
QStringList CWpa_Supplicant::sliceMessage(QString str) {
	return str.split('\n',QString::SkipEmptyParts);
}


//MIB Variables:
//(dot11|dot1x)VARIABLENAME=<value>
//<value> = (TRUE|FALSE) | <Integer> | <String> | <other?>
//so far we do not care about <other> => <other> <-> <string>
wps_MIB CWpa_Supplicant::parseMIB(QStringList list) {
	//First strip dot11 or dot1x
	//create new wps_variable
	//append to wps_MIB
	QList<wps_variable> wpsMIB;
	QStringList tmp_strlist;
	wps_variable var;
	foreach(QString str, list) {
		tmp_strlist = (str.remove(0,5)).split('=',QString::KeepEmptyParts);
		var.name = tmp_strlist[0];
		var.type = parseMIBType(tmp_strlist[1]);
		var.value.num = 0;
		if (wps_variable::PLAIN == var.type || wps_variable::STRING == var.type) {
			var.value.str = new QString(tmp_strlist[0]);
		}
		if (wps_variable::NUMBER == var.type) {
			bool ok = true;
			var.value.num = new qint32(tmp_strlist[1].toInt(&ok));
			if (!ok) {
				*(var.value.num) = -1;
			}
		}
		if (wps_variable::LOGIC == var.type) {
			var.value.logic = new bool((tmp_strlist[1] == "TRUE"));
		}
		wpsMIB.append(var);
	}
	return ((wps_MIB) wpsMIB);
}
wps_variable::wps_variable_type CWpa_Supplicant::parseMIBType(QString str) {
	bool ok;
	str.toInt(&ok);
	if (ok) {
		return wps_variable::NUMBER;
	}
	if ( (0 == str.indexOf("TRUE")) || (0 == str.indexOf("FALSE")) ) {
		return wps_variable::LOGIC;
	}
	if (str.contains(":") || str.contains("-")) {
		return wps_variable::PLAIN;
	}
	return wps_variable::STRING;
}

wps_network_flags CWpa_Supplicant::parseNetworkFlags(QString str) {
	if (str.contains("CURRENT",Qt::CaseInsensitive)) {
		return WNF_CURRENT;
	}
	return WNF_NONE;
}
//network id / ssid / bssid / flags
//0 example network	any	[CURRENT]
QList<wps_network> CWpa_Supplicant::parseListNetwork(QStringList list) {
	list.removeFirst();
	QList<wps_network> networks;
	QStringList line;
	wps_network net;
	bool worked = true;
	foreach(QString str, list) {
		line = str.split('\t',QString::KeepEmptyParts);
		net.id = line[0].toInt(&worked);
		if (!worked) {
			worked = true;
			continue;
		}
		net.ssid = line[1];
		net.bssid = nut::MacAddress(line[2]);
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
wps_ciphers CWpa_Supplicant::parseScanCiphers(QString str) {
	wps_ciphers cip = WC_NONE;
	if (str.contains("CCMP")) {
		cip = (wps_ciphers) (cip | WC_CCMP);
	}
	if (str.contains("TKIP")) {
		cip = (wps_ciphers) (cip | WC_TKIP);
	}
	if (str.contains("WEP104")) {
		cip = (wps_ciphers) (cip | WC_WEP104);
	}
	if (str.contains("WEP40")) {
		cip = (wps_ciphers) (cip | WC_WEP40);
	}
	if (str.contains("WEP")) {
		cip = (wps_ciphers) (cip | WC_WEP);
	}
	return cip;
}
wps_authentication CWpa_Supplicant::parseScanAuth(QString str) {
	wps_authentication key = WA_UNDEFINED;
	if (str.contains("WPA-PSK")) {
		key = (wps_authentication) (key | WA_WPA_PSK);
	}
	if (str.contains("WPA2-EAP")) {
		key = (wps_authentication) (key | WA_WPA2_EAP);
	}
	if (str.contains("WPA2-PSK")) {
		key = (wps_authentication) (key | WA_WPA2_PSK);
	}
	if (str.contains("WPA-EAP")) {
		key = (wps_authentication) (key | WA_WPA_EAP);
	}
	if (str.contains("IEEE8021X")) {
		key = (wps_authentication) (key | WA_IEEE8021X);
	}
	return key;
}

QList<wps_scan> CWpa_Supplicant::parseScanResult(QStringList list) {
	list.removeFirst();
	QList<wps_scan> scanresults;
	QStringList line;
	wps_scan scanresult;
	bool worked = true;
	wps_authentication scanAuth;
	foreach(QString str, list) {
		line = str.split('\t',QString::KeepEmptyParts);
		scanresult.bssid = nut::MacAddress(line[0]);
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
wps_protocols CWpa_Supplicant::parseProtocols(QString str) {
	int proto = WP_UNDEFINED;
	if (str.contains("RSN")) {
		proto = (proto | WP_RSN);
	}
	if (str.contains("WPA")) {
		proto = (proto | WP_WPA);
	}
	return (wps_protocols) proto;
}

wps_key_management CWpa_Supplicant::parseKeyMgmt(QString str) {
	int key = WKM_UNDEFINED;
	if (str.contains("NONE")) {
		key = (key | WKM_NONE);
	}
	if (str.contains("WPA-PSK")) {
		key = (key | WKM_WPA_PSK);
	}
	if (str.contains("WPA-EAP")) {
		key = (key | WKM_WPA_EAP);
	}
	if (str.contains("IEEE8021X")) {
		key = (key | WKM_IEEE8021X);
	}
	return (wps_key_management) key;
}

wps_auth_algs CWpa_Supplicant::parseAuthAlg(QString str) {
	int auth = WAA_UNDEFINED;
	if (str.contains("OPEN")) {
		auth = (auth | WAA_OPEN);
	}
	if (str.contains("SHARED")) {
		auth = (auth | WAA_SHARED);
	}
	if (str.contains("LEAP")) {
		auth = (auth | WAA_LEAP);
	}
	return (wps_auth_algs) auth;
}
wps_pairwise_ciphers CWpa_Supplicant::parsePairwiseCiphers(QString str) {
	int cip = WPC_UNDEFINED;
	if (str.contains("NONE")) {
		cip = (cip | WPC_NONE);
	}
	if (str.contains("CCMP")) {
		cip = (cip | WPC_CCMP);
	}
	if (str.contains("TKIP")) {
		cip = (cip | WPC_TKIP);
	}
	return (wps_pairwise_ciphers) cip;
}
wps_group_ciphers CWpa_Supplicant::parseGroupCiphers(QString str) {
	int cip = WGC_UNDEFINED;
	if (str.contains("CCMP")) {
		cip = (cip | WGC_CCMP);
	}
	if (str.contains("TKIP")) {
		cip = (cip | WGC_TKIP);
	}
	if (str.contains("WEP104")) {
		cip = (cip | WGC_WEP104);
	}
	if (str.contains("WEP40")) {
		cip = (cip | WGC_WEP40);
	}
	return (wps_group_ciphers) cip;
}

wps_eapol_flags CWpa_Supplicant::parseEapolFlags(QString str) {
	int num = str.toInt();
	if (num >= 0 && num <= 3) {
		return (wps_eapol_flags) num;
	}
	else {
		return EAPF_UNDEFINED;
	}
}
wps_eap_method CWpa_Supplicant::parseEapMethod(QString str) {
	//{EAPM_UNDEFINED=0, EAPM_MD5=1,EAPM_MSCHAPV2=2,EAPM_OTP=4,EAPM_GTC=8,EAPM_TLS=16,EAPM_PEAP=32,EAPM_TTLS=64,EAPM_ALL=127} wps_eap_method;
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
	return (wps_eap_method) method;
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
wps_status CWpa_Supplicant::parseStatus(QStringList list) {
	wps_status status;
	bool ok = true;
	foreach(QString str, list) {
		if (0 == str.indexOf("bssid=")) {
			status.bssid = nut::MacAddress(str.split('=',QString::KeepEmptyParts)[1]);
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
wps_status::WPA_STATE CWpa_Supplicant::parseWpaState(QString str) {
	wps_status::WPA_STATE dummy = str;
	return dummy;
}
wps_status::PAE_STATE CWpa_Supplicant::parsePaeState(QString str) {
	wps_status::PAE_STATE dummy = str;
	return dummy;
}
wps_status::PORT_STATUS CWpa_Supplicant::parsePortStatus(QString str) {
	wps_status::PORT_STATUS dummy = str;
	return dummy;
}
wps_status::PORT_CONTROL CWpa_Supplicant::parsePortControl(QString str) {
	wps_status::PORT_CONTROL dummy = str;
	return dummy;
}
wps_status::BACKEND_STATE CWpa_Supplicant::parseBackendState(QString str) {
	wps_status::BACKEND_STATE dummy = str;
	return dummy;
}
wps_status::EAP_STATE CWpa_Supplicant::parseEapState(QString str) {
	wps_status::EAP_STATE dummy = str;
	return dummy;
}
wps_status::METHOD_STATE CWpa_Supplicant::parseMethodState(QString str) {
	wps_status::METHOD_STATE dummy = str;
	return dummy;
}
wps_status::DECISION CWpa_Supplicant::parseDecision(QString str) {
	wps_status::DECISION dummy = str;
	return dummy;
}
//
wps_interact_type CWpa_Supplicant::parseInteract(QString str) {
	if (str.contains("CTRL-EVENT")) {
		return WI_EVENT;
	}
	else if (str.contains("CTRL-REQ")) {
		return WI_REQ;
	}
	else {
		return WI_MSG;
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
wps_req_type CWpa_Supplicant::parseReqType(QString str) {
	if (str.contains("IDENTITY")) {
		return WR_IDENTITY;
	}
	if (str.contains("PASSWORD")) {
		return WR_PASSWORD;
	}
	if (str.contains("NEW_PASSWORD")) {
		return WR_NEW_PASSWORD;
	}
	if (str.contains("PIN")) {
		return WR_PIN;
	}
	if (str.contains("OTP")) {
		return WR_OTP;
	}
	if (str.contains("PASSPHRASE")) {
		return WR_PASSPHRASE;
	}
	return WR_FAIL;
}
wps_req CWpa_Supplicant::parseReq(QString str) {
	bool ok = true;
	wps_req req;
	//Check request type:
	req.type = parseReqType(str);
	//get network id:
	req.id = ((str.split('-',QString::KeepEmptyParts))[2]).toInt(&ok);
	if (!ok) {
		req.id = -1;
	}
	return req;
}

wps_event_type CWpa_Supplicant::parseEvent(QString str) {
	if (str.contains("CONNECTED") ) {
		return WE_CONNECTED;
	}
	if (str.contains("DISCONNECTED") ) {
		return WE_DISCONNECTED;
	}
	if (str.contains("TERMINATING")) {
		return WE_TERMINATING;
	}
	if (str.contains("PASSWORD_CHANGED")) {
		return WE_PASSWORD_CHANGED;
	}
	if (str.contains("WEAP_NOTIFICATION") ){
		return WE_EAP_NOTIFICATION;
	}
	if (str.contains("EAP_STARTED") ) {
		return WE_EAP_STARTED;
	}
	if (str.contains("EAP_METHOD") ) {
		return WE_EAP_METHOD;
	}
	if (str.contains("EAP_SUCCESS") ) {
		return WE_EAP_SUCCESS;
	}
	if (str.contains("EAP_FAILURE") ) {
		return 	WE_EAP_FAILURE;
	}
	return WE_OTHER;
}

void CWpa_Supplicant::printMessage(QString msg) {
	if (log_enabled) {
		emit(message(msg));
	}
}



//Private slots:
//Reads messages from wpa_supplicant
////TODO:Check if we need to check if wpa_supplicant is still available
void CWpa_Supplicant::wps_read(int socket) {
	if (socket == wps_fd) {
		if (wps_connected) {
			//check if wpa_supplicant is still running (i.e. file exists)
			if (! QFile::exists(wpa_supplicant_path)) {
				wps_close("wps_read/no wps interface on event");
				return;
			}
			//status: 1 = msg available; 0 = no messages, -1 = error
			int status = wpa_ctrl_pending(event_ctrl);
			if (1 == status) {
				//Receive Messages
				char reply[512];
				size_t reply_len = sizeof(reply);
				wpa_ctrl_recv(event_ctrl, reply, &reply_len);
				Event_dispatcher(QString::fromUtf8(reply, reply_len));
			}
			else if (-1 == status) {
				qDebug() << "Error while trying to receive messages from wpa_supplicant";
			}
		}
	}
}


/*
//CTRL-RSP-<field name>-<network id>-<value>
*/
/*
CTRL-EVENT-DISCONNECTED
CTRL-EVENT-CONNECTED
*/
void CWpa_Supplicant::Event_dispatcher(wps_req req) {
	if (req.type != WR_FAIL) {
		emit(request(req));
	}
}
void CWpa_Supplicant::Event_dispatcher(wps_event_type event) {
	if (event == WE_CONNECTED) {
		emit(stateChanged(true));
	}
	else if (event == WE_DISCONNECTED) {
		emit(stateChanged(false));
	}
	else if (event == WE_TERMINATING) {
		wps_close("event-dispatcher/wpa-TERMINATING");
	}
	else {
		emit(eventMessage(event));
	}
}

void CWpa_Supplicant::Event_dispatcher(QString event) {
	QStringList str_list = event.split('\n',QString::KeepEmptyParts);
	wps_interact_type type;
	foreach(QString str, str_list) {
		type = parseInteract(str);
		switch (type) {
			case (WI_REQ):
				Event_dispatcher(parseReq(str));
				break;
			case (WI_EVENT):
				Event_dispatcher(parseEvent(str));
				break;
			default:
				printMessage(str);
				break;
		}
	}
}

//Public functions:
CWpa_Supplicant::CWpa_Supplicant(QObject * parent, QString ifname) : QObject(parent), cmd_ctrl(0), event_ctrl(0), wpa_supplicant_path("/var/run/wpa_supplicant/"+ifname), wps_fd(-1), wext_fd(-1), event_sn(0), timerId(-1), wextTimerId(-1), ScanTimerId(-1), wextTimerRate(10000), ifname(ifname), ScanTimeoutCount(0),wextPollTimeoutCount(0) {
	wps_connected = false;
	timerCount = 0;
	log_enabled = true;
	connect(QCoreApplication::instance(),SIGNAL(aboutToQuit ()),this,SLOT(wps_detach()));
}
CWpa_Supplicant::~CWpa_Supplicant() {
	wps_close("destructor");
}
void CWpa_Supplicant::wps_open(bool) {
	if (timerId != -1) {
		killTimer(timerId);
		timerId = -1;
	}
	if (wextTimerId != -1) {
		killTimer(wextTimerId);
		wextTimerId = -1;
	}
	if (wps_connected) return;
	int status;
	//Open wpa_supplicant control interface
	if (!QFile::exists(wpa_supplicant_path)) {
		qDebug() << tr("Could not open wpa_supplicant socket: %1").arg(QString::number(timerCount));
		inConnectionPhase = true;
		timerId = startTimer(wps_TimerTime(timerCount));
		return;
	}
	cmd_ctrl = wpa_ctrl_open(wpa_supplicant_path.toAscii().constData());
	event_ctrl = wpa_ctrl_open(wpa_supplicant_path.toAscii().constData());
	if (cmd_ctrl == NULL and event_ctrl == NULL) {
		qDebug() << tr("Could not open wpa_supplicant control interface");
		inConnectionPhase = true;
		timerId = startTimer(wps_TimerTime(timerCount));
		return;
	}
	if (cmd_ctrl == NULL) {
		wpa_ctrl_close(event_ctrl);
		event_ctrl = NULL;
		qDebug() << tr("Could not open wpa_supplicant control interface");
		inConnectionPhase = true;
		timerId = startTimer(wps_TimerTime(timerCount));
		return;
	}
	if (event_ctrl == NULL) {
		wpa_ctrl_close(cmd_ctrl);
		cmd_ctrl = NULL;
		qDebug() << tr("Could not open wpa_supplicant control interface");
		inConnectionPhase = true;
		timerId = startTimer(wps_TimerTime(timerCount));
		return;
	}
		
	//Atach event monitor
	status = wpa_ctrl_attach(event_ctrl);
	//Status : 0 = succ; -1 = fail, -2 = timeout
	if (status != 0) {
		wpa_ctrl_close(event_ctrl);
		wpa_ctrl_close(cmd_ctrl);
		event_ctrl = NULL;
		cmd_ctrl = NULL;
		qDebug() << tr("Could not attach to wpa_supplicant");
		inConnectionPhase = true;
		timerId = startTimer(wps_TimerTime(timerCount));
		return;
	}
	inConnectionPhase = false;
	timerCount = 0;
	//Set socket notifier
	wps_fd = wpa_ctrl_get_fd(event_ctrl);
	event_sn  = new QSocketNotifier(wps_fd, QSocketNotifier::Read,NULL);
	connect(event_sn,SIGNAL(activated(int)),this,SLOT(wps_read(int)));
	event_sn->setEnabled(true);
	wps_connected = true;

	//NEW FEATURE: Signal level retrieving:
	
	//Get file Descriptor to NET kernel
	if ( (wext_fd = iw_sockets_open()) < 0) {
		wext_fd = -1;
		qDebug() << "ERROR: Could not open socket to net kernel";
	}
	else { //Socket is set up, now set SocketNotifier
		qDebug() << QString("File Descriptor for Wext is: %1").arg(QString::number(wext_fd));
	}
	//Start timer for reading wireless info (like in /proc/net/wireless)
	wextTimerId = startTimer(wextTimerRate);
	
	//Continue of ol features:
	emit(opened());
	printMessage(tr("wpa_supplicant connection established"));
	return;
}

bool CWpa_Supplicant::wps_close(QString call_func, bool internal) {
	if (timerId != -1) {
		killTimer(timerId);
		timerId = -1;
		inConnectionPhase = false;
		timerCount = 0;
	}
	if (wextTimerId != -1) {
		killTimer(wextTimerId);
		wextTimerId = -1;
	}
	if (wps_connected) {
		if (event_sn != NULL) {
			disconnect(event_sn,SIGNAL(activated(int)),this,SLOT(wps_read(int)));
			delete event_sn;
			event_sn = NULL;
		}
		if (-1 != wext_fd) {
			iw_sockets_close(wext_fd);
			wext_fd = -1;
		}
		//Detaching takes place if program is about to quit
		//Close control connections
		wpa_ctrl_close(event_ctrl);
		wpa_ctrl_close(cmd_ctrl);
		event_ctrl = NULL;
		cmd_ctrl = NULL;
		wps_connected = false;
		emit(closed());
	}
	printMessage(tr("(%1)[%2] wpa_supplicant disconnected").arg(((internal) ? "internal" : "external"),call_func));
	return true;
}
int CWpa_Supplicant::wps_TimerTime(int timerCount) {
	if (timerCount > 0) { 
		if (timerCount <= 5) {
			return 1000;
		}
		if (timerCount <= 10) {
			return 3000;
		}
		if (timerCount <= 15) {
			return 10000;
		}
		return 30000;
	}
	else {
		return 0;
	}
}


//Slot is executed when aplication is about to quit;
void CWpa_Supplicant::wps_detach() {
	if (event_ctrl != NULL) {
		wpa_ctrl_detach(event_ctrl);
	}
}

void CWpa_Supplicant::wps_setScanResults(QList<wps_wext_scan> wextScanResults) {
	QString response = wps_cmd_SCAN_RESULTS();
	if (response.isEmpty()) {
		wpsScanResults = QList<wps_scan>();
		return;
	}
	else {
		//Set scan results from wpa_supplicant:
		wpsScanResults = parseScanResult(sliceMessage(response));
		//This may not be possible as qHash references an namespace that is unknown to qt TODO:CHECK!
		QHash<QString,wps_wext_scan> wextScanHash; //Namespace problem
		foreach(wps_wext_scan i, wextScanResults) {
			wextScanHash.insert(i.bssid.toString(), i);
		}
		wps_wext_scan dummy;
		dummy.bssid = nut::MacAddress();
		dummy.quality.level = 0;
		dummy.quality.qual = 0;
		dummy.quality.updated = 1;
		dummy.quality.noise = 0;
		int count = 0;
		//Set the signal quality
		for (QList<wps_scan>::Iterator i = wpsScanResults.begin(); i != wpsScanResults.end(); ++i ) {
			i->quality = wextScanHash.value(i->bssid.toString(), dummy).quality;
			if (wextScanHash.contains(i->bssid.toString())) {
				count++;
			}
		}
		qDebug() << "Found " << count << "BSSIDs; " << wextScanHash.size() << " in Hash; " << wpsScanResults.size() << " in wpa_supplicant list";
		//The complete list is done;
		emit(scanCompleted());
	}
}



void CWpa_Supplicant::timerEvent(QTimerEvent *event) {
	if (event->timerId() == wextTimerId) {
		if ( (wext_fd != -1) && wps_connected) {
			readWirelessInfo();
		}
	}
	else if (event->timerId() == timerId) {
		if (!wps_connected) {
			timerCount++;
			wps_open(true);
		} else {
			killTimer(timerId);
			timerId = -1;
			timerCount = 0;
		}
	}
	else if (event->timerId() == ScanTimerId) {
		if ( (wext_fd != -1) && wps_connected) {
			wps_tryScanResults();
		}
	}
}

bool CWpa_Supplicant::connected() {
	if (wps_cmd_PING() == "PONG\n") {
		return true;
	}
	else {
		return false;
	}
}

//Public slots


void CWpa_Supplicant::setSignalQualityPollRate(int msec) {
	wextPollTimeoutCount = 0;
	if (wextTimerId != -1) {
		qDebug() << "Setting signal quality polling (restarting timer)";
		killTimer(wextTimerId);
		wextTimerRate = msec;
		wextTimerId = startTimer(wextTimerRate);
	}
	else {
		qDebug() << "Setting signal quality polling (starting timer)";
		wextTimerRate = msec;
	}
}
int CWpa_Supplicant::getSignalQualityPollRate() {
	return wextTimerRate;
}
wps_wext_scan_readable CWpa_Supplicant::getSignalQuality() {
	return signalQuality;
}

void CWpa_Supplicant::setLog(bool enabled) {
	log_enabled = enabled;
}
//Function to respond to ctrl requests from wpa_supplicant
void CWpa_Supplicant::response(wps_req request, QString msg) {
	QString cmd = toString(request.type);
	if (!cmd.isEmpty()) {
		wps_cmd_CTRL_RSP(cmd,request.id,msg);
	}
}


void CWpa_Supplicant::selectNetwork(int id) {
	wps_cmd_SELECT_NETWORK(id);
}
void CWpa_Supplicant::enableNetwork(int id) {
	wps_cmd_ENABLE_NETWORK(id);
}
void CWpa_Supplicant::disableNetwork(int id) {
	wps_cmd_DISABLE_NETWORK(id);
}
void CWpa_Supplicant::scan() {
	if (0 == wps_cmd_SCAN().indexOf("OK")) {
		//Check if we're already scanning:
		if (ScanTimerId != -1) {
			killTimer(ScanTimerId);
			ScanTimerId = -1;
			printMessage("We're already scanning!");
		}
		ScanTimeoutCount = 0;
		ScanTimerId = startTimer(CWPA_SCAN_TIMER_TIME);
	}
}
void CWpa_Supplicant::ap_scan(int type) {
	if ( !(0 <= type and 2 >= type) ) {
		wps_cmd_AP_SCAN(1);
	}
	else {
		wps_cmd_AP_SCAN(1);
	}
}
void CWpa_Supplicant::save_config() {
	wps_cmd_SAVE_CONFIG();
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
void CWpa_Supplicant::preauth(nut::MacAddress bssid) {
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


wps_netconfig_status CWpa_Supplicant::addNetwork(wps_network_config config) {
	wps_netconfig_status status;
	status.failures = WCF_NONE;
	status.eap_failures = WECF_NONE;
	int netid = addNetwork();
	if (-1 == netid) {
		status.failures = WCF_ALL;
		status.eap_failures = WECF_ALL;
		status.id = -1;
		return status;
	}
	else {
		status = editNetwork(netid,config);
		if ( (status.eap_failures != WECF_NONE) || (WCF_NONE != status.failures) ) {
			removeNetwork(netid);
			status.id = -1;
		}
		return status;
	}
}


wps_netconfig_status CWpa_Supplicant::editNetwork(int netid, wps_network_config config) {
	wps_netconfig_status wps_fail_status;
	wps_fail_status.failures = WCF_NONE;
	wps_fail_status.eap_failures = WECF_NONE;
	wps_fail_status.id = netid;
	if (!setNetworkVariable(netid,"ssid",config.ssid) ) {
		wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_SSID);
	}
	if (!config.bssid.zero()) {
		if (! setNetworkVariable(netid,"bssid",config.bssid.toString()) ) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_BSSID);
		}
	}
	if (config.disabled != WB_UNDEFINED) {
		if (!setNetworkVariable(netid,"disabled",toNumberString(config.disabled)) ) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_DISABLED);
		}
	}
	if (!config.id_str.isEmpty()) {
		if ( setNetworkVariable(netid,"id_str",config.id_str) ) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_ID_STR);
		}
	}
	if (config.scan_ssid != WB_UNDEFINED) {
		if (!setNetworkVariable(netid,"scan_ssid",toNumberString(config.scan_ssid)) ) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_SCAN_SSID);
		}
	}
	if (config.priority >= 0) {
		if (!setNetworkVariable(netid,"priority",QString::number(config.priority)) ) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_PRIORITY);
		}
	}
	if (config.mode != WB_UNDEFINED) {
		if (!setNetworkVariable(netid,"mode",toNumberString(config.mode)) ) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_MODE);
		}
	}
	if (0 != config.frequency) {
		if (!setNetworkVariable(netid,"frequency",QString::number(config.frequency)) ) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_FREQ);
		}
	}
	if (! (WP_UNDEFINED == config.protocols) ) {
		if ( !setNetworkVariable(netid,"proto",toString(config.protocols)) ) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_PROTO);
		}
	}
	if (! (WKM_UNDEFINED == config.keyManagement) ) {
		if ( !setNetworkVariable(netid,"key_mgmt",toString(config.keyManagement)) ) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_KEYMGMT);
		}
	}
	if (! (WAA_UNDEFINED == config.auth_alg) ) {
		if ( !setNetworkVariable(netid,"auth_alg",toString(config.auth_alg) )) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_AUTH_ALG);
		}
	}
	if (! (WPC_UNDEFINED == config.pairwise) ) {
		if ( !setNetworkVariable(netid,"pairwise",toString(config.pairwise) )) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_PAIRWISE);
		}
	}
	if (! (WGC_UNDEFINED == config.group) ) {
		if ( !setNetworkVariable(netid,"group",toString(config.group) )) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_GROUP);
		}
	}
	if (!config.psk.isEmpty()) {
		if ( !setNetworkVariable(netid,"psk",config.psk)) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_PSK);
		}
	}
	if (! EAPF_UNDEFINED == config.eapol_flags ) {
		if ( !setNetworkVariable(netid,"eapol_flags",toString(config.eapol_flags))) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_EAPOL_FLAGS);
		}
	}
	if (config.mixed_cell != WB_UNDEFINED) {
		if ( !setNetworkVariable(netid,"mixed_cell",toNumberString(config.mixed_cell)) ) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_MIXED_CELL);
		}
	}
	if (config.proactive_key_caching != WB_UNDEFINED) {
		if ( !setNetworkVariable(netid,"proactive_key_caching",toNumberString(config.proactive_key_caching)) ) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_PROA_KEY_CACHING);
		}
	}
	if (!config.wep_key0.isEmpty()) {
		if ( !setNetworkVariable(netid,"wep_key0",config.wep_key0) ) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_WEP_KEY0);
		}	
	}
	if (!config.wep_key1.isEmpty()) {
		if ( !setNetworkVariable(netid,"wep_key1",config.wep_key1)) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_WEP_KEY1);
		}	
	}
	if (!config.wep_key2.isEmpty()) {
		if ( !setNetworkVariable(netid,"wep_key2",config.wep_key2) ) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_WEP_KEY2);
		}	
	}
	if (!config.wep_key3.isEmpty()) {
		if ( !setNetworkVariable(netid,"wep_key3",config.wep_key3) ) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_WEP_KEY3);
		}	
	}
	if (config.wep_tx_keyidx <= 3 && config.wep_tx_keyidx >= 0) {
		if ( !setNetworkVariable(netid,"wep_tx_keyidx",QString::number(config.wep_tx_keyidx)) ) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_WEP_KEY_IDX);
		}
	}
	else if (config.wep_tx_keyidx != -1) {
		wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_WEP_KEY_IDX);
	}
	if (config.peerkey != WB_UNDEFINED) {
		if ( !setNetworkVariable(netid,"peerkey",toNumberString(config.peerkey)) ) {
			wps_fail_status.failures = (wps_netconfig_failures) (wps_fail_status.failures | WCF_PEERKEY);
		}
	}
	//Check if we have an EAP network
	if ((config.keyManagement & (WKM_WPA_EAP | WKM_IEEE8021X) ) || config.keyManagement == WKM_UNDEFINED) {
		wps_fail_status.eap_failures = wps_editEapNetwork(netid,config.eap_config);
	}
	return wps_fail_status;
}

wps_network_config CWpa_Supplicant::getNetworkConfig(int id) {
	wps_network_config config;
	QString response;

	response = wps_cmd_GET_NETWORK(id,"ssid");
	if ("FAIL\n" != response) {
		config.ssid = wps_cmd_GET_NETWORK(id,"ssid");
	}

	response = wps_cmd_GET_NETWORK(id,"bssid");
	if ("FAIL\n" != response) {
		config.bssid = nut::MacAddress(response);
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
	if ( config.keyManagement & (WKM_IEEE8021X | WKM_WPA_EAP)) {
		config.eap_config = wps_getEapNetworkConfig(id);
	}
	return config;
}

wps_eap_network_config CWpa_Supplicant::wps_getEapNetworkConfig(int id) {
	wps_eap_network_config config;
	bool ok;
	QString response;
	//Check if the network uses EAP
	response = wps_cmd_GET_NETWORK(id,"key_mgmt");
	if ("FAIL\n" != response) {
		if ( !(parseKeyMgmt(response) & (WKM_WPA_EAP | WKM_IEEE8021X) ) ) {
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
wps_eap_netconfig_failures CWpa_Supplicant::wps_editEapNetwork(int netid, wps_eap_network_config config) {
	wps_eap_netconfig_failures eap_failures = WECF_NONE;
	if (EAPM_UNDEFINED != config.eap) {
		if (!setNetworkVariable(netid,"eap",toString(config.eap)) ) {
			eap_failures= (wps_eap_netconfig_failures) (eap_failures | WECF_EAP);
		}
	}
	if (!config.identity.isEmpty()) {
		if (!setNetworkVariable(netid,"identity",config.identity) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_IDENTITY);
		}
	}
	if (!config.anonymous_identity.isEmpty()) {
		if (!setNetworkVariable(netid,"anonymous_identity",config.anonymous_identity) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_ANON_IDENTITY);
		}
	}
	if (!config.password.isEmpty()) {
		if (!setNetworkVariable(netid,"password",config.password) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_PASSWD);
		}
	}
	if (!config.ca_cert.isEmpty()) {
		if (!setNetworkVariable(netid,"ca_cert",config.ca_cert) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_CA_CERT);
		}
	}
	if (!config.ca_path.isEmpty()) {
		if (!setNetworkVariable(netid,"ca_path",config.ca_path) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_CA_PATH);
		}
	}
	if (!config.client_cert.isEmpty()) {
		if (!setNetworkVariable(netid,"client_cert",config.client_cert) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_CLIENT_CERT);
		}
	}
	if (!config.private_key.isEmpty()) {
		if (!setNetworkVariable(netid,"private_key",config.private_key) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_PRIVATE_KEY);
		}
	}
	if (!config.private_key_passwd.isEmpty()) {
		if (!setNetworkVariable(netid,"private_key_passwd",config.private_key_passwd) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_PRIVATE_KEY_PASSWD);
		}
	}
	if (!config.dh_file.isEmpty()) {
		if (!setNetworkVariable(netid,"dh_file",config.dh_file) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_DH_FILE);
		}
	}
	if (!config.subject_match.isEmpty()) {
		if (!setNetworkVariable(netid,"subject_match",config.subject_match) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_SUBJECT_MATCH);
		}
	}
	if (!config.altsubject_match.isEmpty()) {
		if (!setNetworkVariable(netid,"altsubject_match",config.altsubject_match) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_ALTSUBJECT_MATCH);
		}
	}
	if (!config.phase1.isEmpty()) {
		if (!setNetworkVariable(netid,"phase1",config.phase1) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_PHASE1);
		}
	}
	if (!config.phase2.isEmpty()) {
		if (!setNetworkVariable(netid,"phase2",config.phase2) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_PHASE2);
		}
	}
	if (!config.ca_cert2.isEmpty()) {
		if (!setNetworkVariable(netid,"ca_cert2",config.ca_cert2) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_CA_CERT2);
		}
	}
	if (!config.ca_path2.isEmpty()) {
		if (!setNetworkVariable(netid,"ca_path2",config.ca_path2) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_CA_PATH2);
		}
	}
	if (!config.client_cert2.isEmpty()) {
		if (!setNetworkVariable(netid,"client_cert2",config.client_cert2) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_CLIENT_CERT2);
		}
	}
	if (!config.private_key2.isEmpty()) {
		if (!setNetworkVariable(netid,"private_key2",config.private_key2) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_PRIVATE_KEY2);
		}
	}
	if (!config.private_key2_passwd.isEmpty()) {
		if (!setNetworkVariable(netid,"private_key2_passwd",config.private_key2_passwd) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_PRIVATE_KEY2_PASSWD);
		}
	}
	if (!config.dh_file2.isEmpty()) {
		if (!setNetworkVariable(netid,"dh_file2",config.dh_file2) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_DH_FILE2);
		}
	}
	if (!config.subject_match2.isEmpty()) {
		if (!setNetworkVariable(netid,"subject_match2",config.subject_match2) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_SUBJECT_MATCH);
		}
	}
	if (!config.altsubject_match2.isEmpty()) {
		if (!setNetworkVariable(netid,"altsubject_match2",config.altsubject_match2) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_ALTSUBJECT_MATCH);
		}
	}
	if (-1 != config.fragment_size) {
		if (!setNetworkVariable(netid,"altsubject_match2",QString::number(config.fragment_size)) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_FRAGMENT_SIZE);
		}
	}
	if (!config.eappsk.isEmpty()) {
		if (!setNetworkVariable(netid,"eappsk",config.eappsk) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_EAPPSK);
		}
	}
	if (!config.nai.isEmpty()) {
		if (!setNetworkVariable(netid,"nai",config.nai) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_NAI);
		}
	}
	if (!config.pac_file.isEmpty()) {
		if (!setNetworkVariable(netid,"pac_file",config.pac_file) ) {
			eap_failures = (wps_eap_netconfig_failures) (eap_failures | WECF_PAC_FILE);
		}
	}
	return eap_failures;
}

void CWpa_Supplicant::wps_tryScanResults() {
	if (wext_fd == -1) {
		return;
	}
	//Kill Timer:
	if (ScanTimerId != -1) {
		killTimer(ScanTimerId);
		ScanTimerId = -1;
	}
	else { //This is not a timer call; this should not happen
		qDebug() << "Warning, no timer present while trying to get scan results";
		return;
	}
	struct iwreq wrq;
	unsigned char * buffer = NULL;		/* Results */
	int buflen = IW_SCAN_MAX_DATA; /* Min for compat WE<17 */
	struct iw_range range;
	int has_range;

	QList<wps_wext_scan> res = QList<wps_wext_scan>();

	/* Get range stuff */
	has_range = (iw_get_range_info(wext_fd, ifname.toAscii().data(), &range) >= 0);
	if (errno == EAGAIN) {
		qDebug() << "Scan results not available yet";
		ScanTimeoutCount++;
		if (ScanTimeoutCount == 5) {
			wps_setScanResults(res);
			return;
		}
		ScanTimerId = startTimer(CWPA_SCAN_RETRY_TIMER_TIME);
		return;
	}
	else if (errno != 0) {
		qDebug() << "Error occured while trying to get Scanresults: " << strerror(errno);
	}
	else {
		qDebug() << "Range stuff got";
	}
	
	unsigned char * newbuf;
	for (int i=0; i <= 16; i++) { //realloc (don't do this forever)
		//Allocate newbuffer 
		newbuf = (uchar*) realloc(buffer, buflen);
		if(newbuf == NULL) {
			if (buffer) {
				free(buffer);
				buffer = NULL;
			}
			qDebug() << "Allocating buffer for Wext failed";
			return;
		}
		buffer = newbuf;
		
		//Set Request variables:
		wrq.u.data.pointer = buffer;
		wrq.u.data.flags = 0;
		wrq.u.data.length = buflen;
		
		//Get the data:
		if (iw_get_ext(wext_fd, ifname.toAscii().data(), SIOCGIWSCAN, &wrq) < 0) {
			//Buffer is too small
			if((errno == E2BIG) && (range.we_version_compiled > 16)) {
				
				//check if driver gives any hints about scan length:
				if (wrq.u.data.length > buflen) {
					buflen = wrq.u.data.length;
				}
				else { //If not, double the length
					buflen *= 2;
				}
				//Start from the beginning:
				qDebug() << "Buffer was too small";
				continue;
			}
			//No range error occured or kernel has wireless extension < 16
			if (errno == EAGAIN) {
				qDebug() << "Scan results not available yet";
				ScanTimerId = startTimer(CWPA_SCAN_RETRY_TIMER_TIME);
				return;
			}
			
			//Bad error occured
			if (buffer) {
				free(buffer);
				buffer = NULL;
			}
			qDebug() << QString("(%1) Failed to read scan data : %2").arg(ifname, QString(strerror(errno)));
		}
		else { //Results are there
			break;
		}
	}

	//Now read the data:
	if (wrq.u.data.length) {

		struct iw_event iwe;
		struct stream_descr stream;
		int ret;
		
		wps_wext_scan singleres;
		singleres.bssid = nut::MacAddress();
		singleres.quality.level = -1;
		singleres.quality.qual = -1;
		singleres.quality.noise = -1;
		singleres.quality.updated = -1;
		singleres.hasRange = has_range;
		if (has_range) {
			singleres.maxquality.level = (quint8) range.max_qual.level;
			singleres.maxquality.qual = (quint8) range.max_qual.qual;
			singleres.maxquality.noise = (quint8) range.max_qual.noise;
			singleres.maxquality.updated = (quint8) range.max_qual.updated;
			singleres.avgquality.level = (quint8) range.avg_qual.level;
			singleres.avgquality.qual = (quint8) range.avg_qual.qual;
			singleres.avgquality.noise = (quint8) range.avg_qual.noise;
			singleres.avgquality.updated = (quint8) range.avg_qual.updated;
			singleres.we_version_compiled = range.we_version_compiled;
		}

		//Init event stream
		QByteArray test;
		char buffer2[128];
		nut::MacAddress tmpMac;
		iw_init_event_stream(&stream, (char *) buffer, wrq.u.data.length);
		do {
			/* Extract an event and parse it*/
			ret = iw_extract_event_stream(&stream, &iwe, range.we_version_compiled);
			if(ret > 0) {
				//Now parse our scan event:
				switch(iwe.cmd) {
					case SIOCGIWAP:
						//ap_addr has type socketaddr
						iw_saether_ntop(&(iwe.u.ap_addr), buffer2);
						tmpMac = nut::MacAddress(QString::fromAscii(buffer2,128));
						break;
					case IWEVQUAL: //Quality event:
						singleres.quality.qual = (quint8) iwe.u.qual.qual;
						singleres.quality.level = (quint8) iwe.u.qual.level;
						singleres.quality.noise = (quint8) iwe.u.qual.noise;
						singleres.quality.updated = (quint8) iwe.u.qual.updated;

						// if our last bssid is different from the actual one, then we have to append it to our scanlist
						if ( (singleres.bssid != tmpMac) )  {
							singleres.bssid = tmpMac;
							res.append(singleres);
						}
					default: //Ignore all other event types. Maybe we need them later?
						break;
				}
			}
		} while(ret > 0);
		if (buffer) {
			free(buffer);
			buffer = NULL;
		}
		//We have the data, now construct complete wps_scan
		wps_setScanResults(res);
	}
	else {
		qDebug() << "No Scanresults available";
	}
	if (buffer) {
		free(buffer);
		buffer = NULL;
	}
}

void CWpa_Supplicant::readWirelessInfo() {
	if ( (wext_fd == -1) || !wps_connected) {
		return;
	}
	qDebug() << "Executing readWirelessInfo()";
	struct iw_range range;
	int hasRange = 0;
	iwstats stats;
	wps_wext_scan res;

	/* workaround */
	struct wireless_config b;
	/* Get basic information */ 
	if(iw_get_basic_config(wext_fd, ifname.toAscii().data(), &b) < 0) {
		/* If no wireless name : no wireless extensions */ 
		/* But let's check if the interface exists at all */ 
		struct ifreq ifr; 
	
		strncpy(ifr.ifr_name, ifname.toAscii().data(), IFNAMSIZ); 
		if(ioctl(wext_fd, SIOCGIFFLAGS, &ifr) < 0) 
			qDebug() << "no device";
		else
			qDebug() << "not supported";
		return;
	}
	qDebug() << "Fetched basic config.";
	/* workaround */

	/* Get range stuff */
	qDebug() << QString("Getting range stuff for %1").arg(ifname.toAscii().data());
	if (iw_get_range_info(wext_fd, ifname.toAscii().data(), &range) >= 0)
		hasRange = 1;
	else
		qDebug() << QString("Error \"hasRange == 0\" (%1)").arg(strerror(errno));
	res.hasRange = hasRange;
	if (errno == EAGAIN) {
		wextPollTimeoutCount++;
		qDebug() << QString("Getting range stuff failed (%1)").arg(strerror(errno));
		if ( (wextPollTimeoutCount > 10) && wextTimerRate < 1000) {
			setSignalQualityPollRate(10000);
		}
		else if (wextPollTimeoutCount > 20) { //Fast polling disabled, but still EAGAIN errors
			//Seems the kernel does not care about our calls
			killTimer(wextTimerId);
			wextTimerId = -1;
		}
		return;
	}
	
	qDebug() << "Got range stuff";

	/* workaround */
	struct iwreq wrq;
	/* Get AP address */
	if(iw_get_ext(wext_fd, ifname.toAscii().data(), SIOCGIWAP, &wrq) >= 0) {
		qDebug() << "Got AP";
	}
	
	/* Get bit rate */
	if(iw_get_ext(wext_fd, ifname.toAscii().data(), SIOCGIWRATE, &wrq) >= 0) {
		qDebug() << "Got bit rate";
	}
	
	/* Get Power Management settings */
	wrq.u.power.flags = 0;
	if(iw_get_ext(wext_fd, ifname.toAscii().data(), SIOCGIWPOWER, &wrq) >= 0) {
		qDebug() << "Got power";
	}
	/* workaround */

	if (hasRange) {
		res.maxquality.level = (quint8) range.max_qual.level;
		res.maxquality.qual = (quint8) range.max_qual.qual;
		res.maxquality.noise = (quint8) range.max_qual.noise;
		res.maxquality.updated = (quint8) range.max_qual.updated;
	}
	else {
		res.maxquality.level = 0;
		res.maxquality.qual = 0;
		res.maxquality.noise = 0;
		res.maxquality.updated = 0;
		qDebug() << "Range information are not available";
	}
	if ( (hasRange) && (range.we_version_compiled > 11) ) {
		struct iwreq wrq;
		wrq.u.data.pointer = (caddr_t) &stats;
		wrq.u.data.length = sizeof(struct iw_statistics);
		wrq.u.data.flags = 1; // Clear updated flag
		strncpy(wrq.ifr_name, ifname.toAscii().data(), IFNAMSIZ);
		
		qDebug() << "Getting wireless stats";
		if(iw_get_ext(wext_fd, ifname.toAscii().data(), SIOCGIWSTATS, &wrq) < 0) {
			qDebug() << "Error occured while fetching wireless info" << strerror(errno);
		}
		else { //Stats fetched
			qDebug() << "Stats fetched";
			res.quality.level = (quint8) stats.qual.level;
			res.quality.qual = (quint8) stats.qual.qual;
			res.quality.noise = (quint8) stats.qual.noise;
			res.quality.updated = (quint8) stats.qual.updated;
			qDebug() << res.quality.level << res.quality.qual << res.quality.noise << res.quality.updated;
			signalQuality = convertValues(res);
			qDebug() << "Emittig signalQualityUpdated()";
			if (wextTimerRate < 1000) {
				wextTimerRate = 10000;
				qDebug() << "Auto-resetting timer to 10 seconds";
			}
			emit signalQualityUpdated(signalQuality);
		}
	}
	else if (range.we_version_compiled <= 11) {
		printMessage(tr("Cannot fetch wireless information as your wireless extension is too old"));
		printMessage(tr("Think about updating your kernel (it's way too old)"));
	}
	else {
		qDebug() << "Error while trying to fetch wireless information" << strerror(errno);
		qDebug() << "Wireless Extension socket file descriptor was: " << wext_fd;
	}
}


void CWpa_Supplicant::removeNetwork(int id) {
	wps_cmd_REMOVE_NETWORK(id);
}

//TODO:Check is id is in range
void CWpa_Supplicant::setBssid(int id, nut::MacAddress bssid) {
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
QList<wps_network> CWpa_Supplicant::listNetworks() {
	QString reply = wps_cmd_LIST_NETWORKS();
	if (!reply.isEmpty()) {
		return parseListNetwork(sliceMessage(reply));
	}
	else {
		return QList<wps_network>();
	}
}

QList<wps_scan> CWpa_Supplicant::scanResults() {
	return wpsScanResults;
}
wps_status CWpa_Supplicant::status() {
	QString reply = wps_cmd_STATUS(true);
	if (!reply.isEmpty()) {
		return parseStatus(sliceMessage(reply));
	}
	else {
		wps_status dummy;
		return dummy;
	}
}
wps_MIB CWpa_Supplicant::getMIBVariables() {
	QString reply = wps_cmd_MIB();
	if (!reply.isEmpty()) {
		return parseMIB(sliceMessage(reply));
	}
	else {
		return (wps_MIB) QList<wps_variable>();
	}
}

wps_capabilities CWpa_Supplicant::getCapabilities() {
	wps_capabilities caps;
	caps.eap = EAPM_UNDEFINED;
	caps.pairwise = WPC_UNDEFINED;
	caps.group = WGC_UNDEFINED;
	caps.keyManagement = WKM_UNDEFINED;
	caps.proto = WP_UNDEFINED;
	caps.auth_alg = WAA_UNDEFINED;
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
