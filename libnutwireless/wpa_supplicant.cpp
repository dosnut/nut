#ifndef LIBNUT_NO_WIRELESS
#include "wpa_supplicant.h"

namespace libnutwireless {

void CWpaSupplicant::setApScanDefault() {
	if (-1 != m_apScanDefault) {
		return;
	}
	QList<ShortNetworkInfo> netlist = listNetworks();
	//Assume ap_scan=1 as default:
	m_apScanDefault = 1;
	printMessage("Auto-setting ap_scan=1");
	m_lastWasAdHoc = false;
	//Check if active network is ad-hoc network:
	foreach(ShortNetworkInfo i, netlist) {
		if (NF_CURRENT == i.flags) {
			if (i.adhoc) {
				m_lastWasAdHoc = true;
				printMessage("auto-setting last was adhoc to true");
			}
			break;
		}
	}
}

//Function to respond to ctrl requests from wpa_supplicant
void CWpaSupplicant::response(Request request, QString msg) {
	QString cmd = toString(request.type);
	if (!cmd.isEmpty()) {
		wpaCtrlCmd_CTRL_RSP(cmd,request.id,msg);
	}
}

bool CWpaSupplicant::selectNetwork(int id) {
	//TODO: Check if we need to set ap_scan first

	QOOL hasMode = toQOOL(getNetworkVariable(id,"mode"));
	//Let's hope wpa_supplicant returns fail only when network id is invalid
	if (QOOL_UNDEFINED == hasMode) {
		return false;
	}
	else if (QOOL_TRUE == hasMode) {
		ap_scan(2);
		printMessage(tr("auto-setting ap_scan=2."));
	}
	else {
		//Check if we have defaults, and if the last network was an ap-network
		if (m_lastWasAdHoc) {
			m_lastWasAdHoc = false;
			if (-1 != m_apScanDefault) {
				ap_scan(m_apScanDefault);
				printMessage(tr("Using your last ap_scan settings for auto-setting: %1").arg(QString::number(m_apScanDefault)));
			}
			else {
				printMessage(tr("You must set ap_scan to your needs!"));

			}
		}
	}
	if ("OK\n" == wpaCtrlCmd_SELECT_NETWORK(id)) {
		emit networkListUpdated();
		return true;
	}
	else {
		//Reset ap_scan:
		if (m_lastWasAdHoc) {
			ap_scan(m_apScanDefault);
		}
		return false;
	}
}
bool CWpaSupplicant::enableNetwork(int id) {
	if ("OK\n" == wpaCtrlCmd_ENABLE_NETWORK(id)) {
		emit networkListUpdated();
		return true;
	}
	else {
		return false;
	}
}
bool CWpaSupplicant::disableNetwork(int id) {
	if ("OK\n" == wpaCtrlCmd_DISABLE_NETWORK(id)) {
		emit networkListUpdated();
		return true;
	}
	else {
		return false;
	}
}

bool CWpaSupplicant::ap_scan(int type) {
	if ( (0 <= type and 2 >= type) ) {
		if ("FAIL\n" == wpaCtrlCmd_AP_SCAN(type)) {
			return false;
		}
		else {
			printMessage(QString("Setting ap_scan=%1").arg(type));
			//ap_scan variables accordingly
			if (2 == type) {
				m_lastWasAdHoc = true;
			}
			else {
				m_lastWasAdHoc = false;
				m_apScanDefault = type;
			}
			return true;
		}
	}
	return false;
}
bool CWpaSupplicant::save_config() {
	return !("FAIL\n" == wpaCtrlCmd_SAVE_CONFIG());
}
void CWpaSupplicant::disconnect_device() {
	wpaCtrlCmd_DISCONNECT();
}
void CWpaSupplicant::logon() {
	wpaCtrlCmd_LOGON();
}
void CWpaSupplicant::logoff() {
	wpaCtrlCmd_LOGOFF();
}
void CWpaSupplicant::reassociate() {
	wpaCtrlCmd_REASSOCIATE();
}
void CWpaSupplicant::debug_level(int level) {
	wpaCtrlCmd_LEVEL(level);
}
void CWpaSupplicant::reconfigure() {
	wpaCtrlCmd_RECONFIGURE();
}
void CWpaSupplicant::terminate() {
	wpaCtrlCmd_TERMINATE();
}
void CWpaSupplicant::preauth(libnutcommon::MacAddress bssid) {
	wpaCtrlCmd_PREAUTH(bssid.toString());
}
int CWpaSupplicant::addNetwork() {
	QString reply = wpaCtrlCmd_ADD_NETWORK();
	if ("FAIL\n" == reply) {
		return -1;
	}
	else {
		return reply.toInt();
	}
}

NetconfigStatus CWpaSupplicant::addNetwork(CNetworkConfig config) {
	NetconfigStatus status;
	status.failures = NCF_NONE;
	status.eap_failures = ENCF_NONE;
	int netid = addNetwork();
	if (-1 == netid) {
		status.failures = NCF_ALL;
		status.eap_failures = ENCF_ALL;
		status.id = -1;
		return status;
	}
	else {
		status = editNetwork(netid,config);
		if ( (status.eap_failures != ENCF_NONE) || (NCF_NONE != status.failures) ) {
			removeNetwork(netid);
			status.id = -1;
		}
		else {
			emit networkListUpdated();
		}
		return status;
	}
}

void CWpaSupplicant::removeNetwork(int id) {
	wpaCtrlCmd_REMOVE_NETWORK(id);
	emit networkListUpdated();
}

bool CWpaSupplicant::setBssid(int id, libnutcommon::MacAddress bssid) {
	if (0 == wpaCtrlCmd_BSSID(id,bssid.toString()).indexOf("OK")) {
		return true;
	}
	else {
		return false;
	}
}
//Plain setVaraiable functions
void CWpaSupplicant::setVariable(QString var, QString val) {
	wpaCtrlCmd_SET(var,val);
}
bool CWpaSupplicant::setNetworkVariable(int id, QString var, QString val) {
	QString ret = wpaCtrlCmd_SET_NETWORK(id,var,val);
	if (ret.contains("OK")) {
		return true;
	}
	else {
		return false;
	}
}
QString CWpaSupplicant::getNetworkVariable(int id, QString val) {
	return wpaCtrlCmd_GET_NETWORK(id,val);
}

//Functions with a lot more functionality  (in the parser functions :)
QList<ShortNetworkInfo> CWpaSupplicant::listNetworks() {
	QString reply = wpaCtrlCmd_LIST_NETWORKS();
	if (!reply.isEmpty()) {
		QList<ShortNetworkInfo> info = parseListNetwork(sliceMessage(reply));
		for (int i = 0; i < info.size(); i++)
			info[i].adhoc = toBool(getNetworkVariable(info[i].id,"mode"));
		return info;
	}
	else {
		return QList<ShortNetworkInfo>();
	}
}

Status CWpaSupplicant::status() {
	QString reply = wpaCtrlCmd_STATUS(true);
	if (!reply.isEmpty()) {
		return parseStatus(sliceMessage(reply));
	}
	else {
		Status dummy;
		return dummy;
	}
}
MIBVariables CWpaSupplicant::getMIBVariables() {
	QString reply = wpaCtrlCmd_MIB();
	if (!reply.isEmpty()) {
		return parseMIB(sliceMessage(reply));
	}
	else {
		return (MIBVariables) QList<MIBVariable>();
	}
}

Capabilities CWpaSupplicant::getCapabilities() {
	Capabilities caps;
	caps.eap = EAPM_UNDEFINED;
	caps.pairwise = PCI_UNDEFINED;
	caps.group = GCI_UNDEFINED;
	caps.keyManagement = KM_UNDEFINED;
	caps.proto = PROTO_UNDEFINED;
	caps.auth_alg = AUTHALG_UNDEFINED;
	QString response;
	response = wpaCtrlCmd_GET_CAPABILITY("eap",false);
	if ("FAIL\n" != response) {
		caps.eap = toEapMethod(response);
	}
	response = wpaCtrlCmd_GET_CAPABILITY("pairwise",false);
	if ("FAIL\n" != response) {
		caps.pairwise = toPairwiseCiphers(response);
	}
	response = wpaCtrlCmd_GET_CAPABILITY("group",false);
	if ("FAIL\n" != response) {
		caps.group = toGroupCiphers(response);
	}
	response = wpaCtrlCmd_GET_CAPABILITY("key_mgmt",false);
	if ("FAIL\n" != response) {
		caps.keyManagement = toKeyMgmt(response);
	}
	response = wpaCtrlCmd_GET_CAPABILITY("proto",false);
	if ("FAIL\n" != response) {
		caps.proto = toProtocols(response);
	}
	response = wpaCtrlCmd_GET_CAPABILITY("auth_alg",false);
	if ("FAIL\n" != response) {
		caps.auth_alg = toAuthAlg(response);
	}
	return caps;
}
}
#endif
