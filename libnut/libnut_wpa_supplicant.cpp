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
	size_t * reply_len;
	size_t command_len;
	char * command;
	char * reply;
	
	command = new char[5];
	command = "PING";
	command_len = sizeof(command);
	int status = wpa_ctrl_request(cmd_ctrl, command, command_len, reply, reply_len,NULL);
	if ( (status != 0) or (QString(reply) != "PONG") ) {
		//TODO: Maybe we need to detach and close wpa_supplicant:
		wps_close();
		return QString();
	}
	if (cmd != "PING") {
		delete [] reply;
		delete reply_len;
		command = cmd.toAscii().data();
		command_len = cmd.toAscii().size();
		
		status = wpa_ctrl_request(cmd_ctrl, command, command_len, reply, reply_len,NULL);
		if (0 == status) {
			if (reply_len > 0) {
				//TODO:Check if reply is \0 terminated
				return QString(reply);
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
		return QString(reply);
	}
}

//parser Functions:
void CWpa_Supplicant::parseMessage(QString msg) {
	emit(message(msg));
}

//Private slots:
//Reads messages from wpa_supplicant
void CWpa_Supplicant::wps_read(int socket) {
	if (socket == wps_fd) {
		//status: 1 = msg available; 0 = no messages, -1 = error
		int status = wpa_ctrl_pending(event_ctrl);
		if (1 == status) {
			//Receive Messages
			char * reply;
			size_t * reply_len;
			wpa_ctrl_recv(event_ctrl, reply,reply_len);
			//TODO:Write parser functions needed here:
			//TODO:Check if reply is \0 terminated
			parseMessage(QString(reply));
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

//Future Functions:
/*

QList<wps_network> CWpa_Supplicant::listNetworks() {
	return QList<wps_network>();
}


*/
}
