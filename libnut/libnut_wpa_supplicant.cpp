#include "libnut_wpa_supplicant.h"

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
		printMessage(QString("(status=%2)PING COMMAND RESPONSE: %1").arg(QString::fromUtf8(reply, reply_len),QString::number(status)));
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
			printMessage(cmd + ":" + QString::fromUtf8(reply, reply_len) + "\nEOC");
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
		scanresult.level = line[2].toInt(&worked);
		if (!worked) {
			worked = true;
			continue;
		}
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
	if (str.contains("RSN")) {
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
				printMessage("Error while trying to receive messages from wpa_supplicant");
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
CWpa_Supplicant::CWpa_Supplicant(QObject * parent, QString wpa_supplicant_path) : QObject(parent), wpa_supplicant_path(wpa_supplicant_path), timerId(-1) {
	wps_connected = false;
	log_enabled = true;
	connect(QCoreApplication::instance(),SIGNAL(aboutToQuit ()),this,SLOT(wps_detach()));
}
CWpa_Supplicant::~CWpa_Supplicant() {
	wps_close("destructor");
}
void CWpa_Supplicant::wps_open(bool timer_call) {
	if (timerId != -1) {
		killTimer(timerId);
		timerId = -1;
	}
	if (wps_connected) return;
	int status;
	//Open wpa_supplicant control interface
	if (!QFile::exists(wpa_supplicant_path)) {
		printMessage(tr("Could not open wpa_supplicant socket"));
		if (! timer_call) {
			inConnectionPhase = true;
			timerId = startTimer(1000);
		}
		return;
	}
	cmd_ctrl = wpa_ctrl_open(wpa_supplicant_path.toAscii().constData());
	event_ctrl = wpa_ctrl_open(wpa_supplicant_path.toAscii().constData());
	if (cmd_ctrl == NULL and event_ctrl == NULL) {
		printMessage(tr("Could not open wpa_supplicant control interface"));
		if (! timer_call) {
			inConnectionPhase = true;
			timerId = startTimer(1000);
		}
		return;
	}
	if (cmd_ctrl == NULL) {
		wpa_ctrl_close(event_ctrl);
		printMessage(tr("Could not open wpa_supplicant control interface"));
		if (! timer_call) {
			inConnectionPhase = true;
			timerId = startTimer(1000);
		}
		return;
	}
	if (event_ctrl == NULL) {
		wpa_ctrl_close(cmd_ctrl);
		printMessage(tr("Could not open wpa_supplicant control interface"));
		if (! timer_call) {
			inConnectionPhase = true;
			timerId = startTimer(1000);
		}
		return;
	}
		
	//Atach event monitor
	status = wpa_ctrl_attach(event_ctrl);
	//Status : 0 = succ; -1 = fail, -2 = timeout
	if (status != 0) {
		wpa_ctrl_close(event_ctrl);
		wpa_ctrl_close(cmd_ctrl);
		printMessage(tr("Could not attach to wpa_supplicant"));
		if (! timer_call) {
			inConnectionPhase = true;
			timerId = startTimer(1000);
		}
		return;
	}
	inConnectionPhase = false;
	//Set socket notifier
	wps_fd = wpa_ctrl_get_fd(event_ctrl);
	event_sn  = new QSocketNotifier(wps_fd, QSocketNotifier::Read,NULL);
	connect(event_sn,SIGNAL(activated(int)),this,SLOT(wps_read(int)));
	event_sn->setEnabled(true);
	wps_connected = true;
	emit(opened());
	printMessage(tr("wpa_supplicant connection established"));
	return;
}
bool CWpa_Supplicant::wps_close(QString call_func, bool internal) {
	if (timerId != -1) {
		killTimer(timerId);
		timerId = -1;
		inConnectionPhase = false;
	}
	if (wps_connected) {
		if (event_sn != NULL) {
			disconnect(event_sn,SIGNAL(activated(int)),this,SLOT(wps_read(int)));
			delete event_sn;
			event_sn = NULL;
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
//Slot is executed when aplication is about to quit;
void CWpa_Supplicant::wps_detach() {
	if (event_ctrl != NULL) {
		wpa_ctrl_detach(event_ctrl);
	}
}

void CWpa_Supplicant::timerEvent(QTimerEvent *event) {
	if (event->timerId() == timerId) {
		if (!wps_connected) {
			wps_open(true);
		} else {
			killTimer(timerId);
			timerId = -1;
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
		emit(scanCompleted());
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

inline int toNumber(bool b) {
	return ((b)? 1 : 0);
}
inline bool toBool(QString str) {
	return ( ("0" == str) ? true : false);
}

wps_netconfig_status CWpa_Supplicant::addNetwork(wps_network_config config) {
	wps_netconfig_status status;
	int netid = addNetwork();
	if (-1 == netid) {
		status.failures = WCF_ALL;
		status.id = -1;
		return status;
	}
	else {
		status.failures = editNetwork(netid, config);
		if (status.failures == WCF_NONE) {
			status.id = netid;
		}
		else {
			removeNetwork(netid);
			status.id = -1;
		}
		return status;
	}
}


wps_netconfig_failures CWpa_Supplicant::editNetwork(int netid, wps_network_config config) { //return -1 if failed, otherwise return network id
	wps_netconfig_failures wps_fail = WCF_NONE;
	if (!setNetworkVariable(netid,"ssid",config.ssid) ) {
		wps_fail = (wps_netconfig_failures) (wps_fail | WCF_SSID);
	}
	if (!config.bssid.zero()) {
		if (! setNetworkVariable(netid,"bssid",config.bssid.toString()) ) {
			wps_fail = (wps_netconfig_failures) (wps_fail | WCF_BSSID);
		}
	}
	if (!setNetworkVariable(netid,"disabled",QString::number(toNumber(config.disabled))) ) {
		wps_fail = (wps_netconfig_failures) (wps_fail | WCF_DISABLED);
	}
	if (!config.id_str.isEmpty()) {
		if ( setNetworkVariable(netid,"id_str",config.id_str) ) {
			wps_fail = (wps_netconfig_failures) (wps_fail | WCF_ID_STR);
		}
	}
	if (!setNetworkVariable(netid,"scan_ssid",QString::number(toNumber(config.scan_ssid))) ) {
		wps_fail = (wps_netconfig_failures) (wps_fail | WCF_SCAN_SSID);
	}
	if (config.priority >= 0) {
		if (!setNetworkVariable(netid,"priority",QString::number(config.priority)) ) {
			wps_fail = (wps_netconfig_failures) (wps_fail | WCF_PRIORITY);
		}
	}
	if (!setNetworkVariable(netid,"mode",QString::number(toNumber(config.mode))) ) {
		wps_fail = (wps_netconfig_failures) (wps_fail | WCF_MODE);
	}
	if (0 != config.frequency) {
		if (!setNetworkVariable(netid,"frequency",QString::number(config.frequency)) ) {
			wps_fail = (wps_netconfig_failures) (wps_fail | WCF_FREQ);
		}
	}
	if (! (WP_UNDEFINED == config.protocols) ) {
		if ( !setNetworkVariable(netid,"proto",toString(config.protocols)) ) {
			wps_fail = (wps_netconfig_failures) (wps_fail | WCF_PROTO);
		}
	}
	if (! (WKM_UNDEFINED == config.keyManagement) ) {
		if ( !setNetworkVariable(netid,"key_mgmt",toString(config.keyManagement)) ) {
			wps_fail = (wps_netconfig_failures) (wps_fail | WCF_KEYMGMT);
		}
	}
	if (! (WAA_UNDEFINED == config.auth_alg) ) {
		if ( !setNetworkVariable(netid,"auth_alg",toString(config.auth_alg) )) {
			wps_fail = (wps_netconfig_failures) (wps_fail | WCF_AUTH_ALG);
		}
	}
	if (! (WPC_UNDEFINED == config.pairwise) ) {
		if ( !setNetworkVariable(netid,"pairwise",toString(config.pairwise) )) {
			wps_fail = (wps_netconfig_failures) (wps_fail | WCF_PAIRWISE);
		}
	}
	if (! (WGC_UNDEFINED == config.group) ) {
		if ( !setNetworkVariable(netid,"group",toString(config.group) )) {
			wps_fail = (wps_netconfig_failures) (wps_fail | WCF_GROUP);
		}
	}
	if (!config.psk.isEmpty()) {
		if ( !setNetworkVariable(netid,"psk",config.psk)) {
			wps_fail = (wps_netconfig_failures) (wps_fail | WCF_PSK);
		}
	}
	if (! EAPF_UNDEFINED == config.eapol_flags ) {
		if ( !setNetworkVariable(netid,"eapol_flags",toString(config.eapol_flags))) {
			wps_fail = (wps_netconfig_failures) (wps_fail | WCF_EAPOL_FLAGS);
		}
	}
	if ( !setNetworkVariable(netid,"mixed_cell",QString::number(toNumber(config.mixed_cell))) ) {
		wps_fail = (wps_netconfig_failures) (wps_fail | WCF_MIXED_CELL);
	}
	if ( !setNetworkVariable(netid,"proactive_key_caching",QString::number(toNumber(config.proactive_key_caching))) ) {
		wps_fail = (wps_netconfig_failures) (wps_fail | WCF_PROA_KEY_CACHING);
	}
	if (!config.wep_key0.isEmpty()) {
		if ( !setNetworkVariable(netid,"wep_key0",config.wep_key0) ) {
			wps_fail = (wps_netconfig_failures) (wps_fail | WCF_WEP_KEY0);
		}	
	}
	if (!config.wep_key1.isEmpty()) {
		if ( !setNetworkVariable(netid,"wep_key1",config.wep_key1)) {
			wps_fail = (wps_netconfig_failures) (wps_fail | WCF_WEP_KEY1);
		}	
	}
	if (!config.wep_key2.isEmpty()) {
		if ( !setNetworkVariable(netid,"wep_key2",config.wep_key2) ) {
			wps_fail = (wps_netconfig_failures) (wps_fail | WCF_WEP_KEY2);
		}	
	}
	if (!config.wep_key3.isEmpty()) {
		if ( !setNetworkVariable(netid,"wep_key3",config.wep_key3) ) {
			wps_fail = (wps_netconfig_failures) (wps_fail | WCF_WEP_KEY3);
		}	
	}
	if (config.wep_tx_keyidx <= 3 && config.wep_tx_keyidx >= 0) {
		if ( !setNetworkVariable(netid,"wep_tx_keyidx",QString::number(config.wep_tx_keyidx)) ) {
			wps_fail = (wps_netconfig_failures) (wps_fail | WCF_WEP_KEY_IDX);
		}
	}
	else if (config.wep_tx_keyidx != -1) {
		wps_fail = (wps_netconfig_failures) (wps_fail | WCF_WEP_KEY_IDX);
	}
	if ( !setNetworkVariable(netid,"peerkey",QString::number(toNumber(config.peerkey))) ) {
		wps_fail = (wps_netconfig_failures) (wps_fail | WCF_PEERKEY);
	}
	return wps_fail;
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
		config.disabled = toBool(response);
	}

	response = wps_cmd_GET_NETWORK(id,"id_str");
	if ("FAIL\n" != response) {
		config.id_str = response;
	}

	response = wps_cmd_GET_NETWORK(id,"scan_ssid");
	if ("FAIL\n" != response) {
		config.scan_ssid = toBool(response);
	}

	response = wps_cmd_GET_NETWORK(id,"priority");
	if ("FAIL\n" != response) {
		config.priority = response.toInt();
	}

	response = wps_cmd_GET_NETWORK(id,"mode");
	if ("FAIL\n" != response) {
		config.mode = toBool(response);
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
		config.mixed_cell = toBool(response);
	}

	response = wps_cmd_GET_NETWORK(id,"proactive_key_caching");
	if ("FAIL\n" != response) {
		config.proactive_key_caching = toBool(response);
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
	}

	response = wps_cmd_GET_NETWORK(id,"peerkey");
	if ("FAIL\n" != response) {
		config.peerkey = toBool(response);
	}
	return config;
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
	QString reply = wps_cmd_SCAN_RESULTS();
	if (!reply.isEmpty()) {
		return parseScanResult(sliceMessage(reply));
	}
	else {
		return QList<wps_scan>();
	}
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


}
