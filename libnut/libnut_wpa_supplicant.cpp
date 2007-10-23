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
	//First Check if wpa_supplicant is running:
	size_t command_len;
	char * command;
	char reply[4096];
	size_t reply_len = sizeof(reply);
	
	command = new char[5];
	command = "PING";
	command_len = sizeof(command);
	int status = wpa_ctrl_request(cmd_ctrl, command, command_len, reply, &reply_len,NULL);
	if ( (status != 0) or (QString::fromUtf8(reply, reply_len) != "PONG") ) {
		//TODO: Maybe we need to detach and close wpa_supplicant:
		wps_close();
		return QString();
	}
	if (cmd != "PING") {
		size_t reply_len = sizeof(reply);
		command = cmd.toAscii().data();
		command_len = cmd.toAscii().size();
		
		status = wpa_ctrl_request(cmd_ctrl, command, command_len, reply, &reply_len,NULL);
		if (0 == status) {
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
		return QString::fromUtf8(reply, reply_len);
	}
}

//parser Functions:
QStringList CWpa_Supplicant::sliceMessage(QString str) {
	return str.split('\n',QString::SkipEmptyParts);
}
wps_MIB CWpa_Supplicant::parseMIB(QStringList list) {
	wps_MIB dummy;
	return dummy;
}

wps_network_flags parseNetworkFlags(QString str) {
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
	foreach(QString str, list) {
		line = str.split('\t',QString::KeepEmptyParts);
		net.id = line[0].toInt();
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
QList<wps_scan> CWpa_Supplicant::parseScanResult(QStringList list) {
	list.removeFirst();
	QList<wps_scan> scanresults;
	QStringList line;
	wps_scan scanresult;
	foreach(QString str, list) {
		line = str.split('\t',QString::KeepEmptyParts);
		scanresult.bssid = nut::MacAddress(line[0]);
		scanresult.freq = line[1].toInt();
		scanresult.level = line[2].toInt();
		//scanresult.ciphers = parseScanCiphers;
		//scanresult.KEYMGMT key_mgmt;
		scanresult.ssid = line[4];
		scanresults.append(scanresult);
	}
	return scanresults;
}
wps_status CWpa_Supplicant::parseStatus(QStringList list) {
	wps_status dummy;
	return dummy;
}


void CWpa_Supplicant::printMessage(QString msg) {
	if (log_enabled) {
		emit(message(msg));
	}
}



//Private slots:
//Reads messages from wpa_supplicant
void CWpa_Supplicant::wps_read(int socket) {
	if (socket == wps_fd) {
		//status: 1 = msg available; 0 = no messages, -1 = error
		int status = wpa_ctrl_pending(event_ctrl);
		if (1 == status) {
			//Receive Messages
			char reply[512];
			size_t reply_len = sizeof(reply);
			wpa_ctrl_recv(event_ctrl, reply, &reply_len);
			//
			printMessage(QString::fromUtf8(reply,reply_len));
			//Slice String into StringList
//			eventParser(QString::fromUtf8(reply, reply_len));
		}
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
//CTRL-RSP-<field name>-<network id>-<value>
*/
/*
CTRL-EVENT-DISCONNECTED
CTRL-EVENT-CONNECTED
*/
void CWpa_Supplicant::Event_dispatcher(wps_req request) {
	emit(wps_request(request));
}
void CWpa_Supplicant::Event_dispatcher(wps_event_type event) {
	if (event == WE_CONNECTED) {
		emit(wps_stateChange(true));
	}
	else if (event == WE_DISCONNECTED) {
		emit(wps_stateChange(false));
	}
}


//Public functions:
CWpa_Supplicant::CWpa_Supplicant(QObject * parent) : QObject(parent) {
	wpa_supplicant_path = "/var/run/wpa_supplicant";
}
CWpa_Supplicant::CWpa_Supplicant(QObject * parent, QString wpa_supplicant_path) : QObject(parent), wpa_supplicant_path(wpa_supplicant_path) {
}
CWpa_Supplicant::~CWpa_Supplicant() {
	wps_close();
}
bool CWpa_Supplicant::wps_open() {
	int status;
	//Open wpa_supplicant control interface
	cmd_ctrl = wpa_ctrl_open(wpa_supplicant_path.toAscii().constData());
	event_ctrl = wpa_ctrl_open(wpa_supplicant_path.toAscii().constData());
	if (cmd_ctrl == NULL and event_ctrl == NULL) {
		return false;
	}
	if (cmd_ctrl == NULL) {
		wpa_ctrl_close(event_ctrl);
		return false;
	}
	if (event_ctrl == NULL) {
		wpa_ctrl_close(cmd_ctrl);
		return false;
	}
		
	//Atach event monitor
	status = wpa_ctrl_attach(event_ctrl);
	//Status : 0 = succ; -1 = fail, -2 = timeout
	if (status != 0) {
		return false;
	}
	//Set socket notifier
	wps_fd = wpa_ctrl_get_fd(event_ctrl);
	event_sn  = new QSocketNotifier(wps_fd, QSocketNotifier::Read,this);
	connect(event_sn,SIGNAL(activated(int)),this,SLOT(wps_read(int)));
	event_sn->setEnabled(true);
	emit(opened());
	return true;
}
bool CWpa_Supplicant::wps_close() {
	disconnect(event_sn,SIGNAL(activated(int)),this,SLOT(wps_read(int)));
	delete event_sn;
	int status;
	status = wpa_ctrl_detach(event_ctrl);
	//Status : 0 = succ; -1 = fail, -2 = timeout
	if (status == -1) {
		return false;
	}
	wpa_ctrl_close(event_ctrl);
	wpa_ctrl_close(cmd_ctrl);
	emit(closed());
	return true;
}
bool CWpa_Supplicant::connected() {
	if (wps_cmd_PING() == "PONG") {
		return true;
	}
	else {
		return false;
	}
}

//Public slots


void setLog(bool enabled) {
	log_enabled = enabled;
}
//Function to respond to ctrl requests from wpa_supplicant
void CWpa_Supplicant::wps_response(wps_req request, QString msg) {
	switch (request.type) {
		case (WR_IDENTITY):
			wps_cmd_CTRL_RSP("IDENTITY",request.id,msg);
			break;
		case (WR_NEW_PASSWORD):
			wps_cmd_CTRL_RSP("NEW_PASSWORD",request.id,msg);
			break;
		case (WR_PIN):
			wps_cmd_CTRL_RSP("PIN",request.id,msg);
			break;
		case (WR_OTP):
			wps_cmd_CTRL_RSP("OTP",request.id,msg);
			break;
		case (WR_PASSPHRASE):
			wps_cmd_CTRL_RSP("PASSPHRASE",request.id,msg);
			break;
		default:
			break;
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
	wps_cmd_SCAN();
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
	if ("FAIL" == reply) {
		return -1;
	}
	else {
		return reply.toInt();
	}
}
//TODO:Check is id is in range
void CWpa_Supplicant::setBssid(int id, nut::MacAddress bssid) {
	wps_cmd_BSSID(id,bssid.toString());
}
//Plain setVaraiable functions
void CWpa_Supplicant::setVariable(QString var, QString val) {
	wps_cmd_SET(var,val);
}
void CWpa_Supplicant::setNetworkVariable(int id, QString var, QString val) {
	wps_cmd_SET_NETWORK(id,var,val);
}
QString CWpa_Supplicant::getNetworkVariable(int id, QString val) {
	return wps_cmd_GET_NETWORK(id,val);
}

//Future Functions:
/*

QList<wps_network> CWpa_Supplicant::listNetworks() {
	return QList<wps_network>();
}


*/
}
