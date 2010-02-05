#include "cnetworkconfig.h"
namespace libnutwireless {


CNetworkConfig::CNetworkConfig() {
	//Set default values
	ssid = QString();
	bssid = libnutcommon::MacAddress();
	disabled = QOOL_UNDEFINED;
	id_str = QString();
	scan_ssid = QOOL_UNDEFINED; 
	priority = -1;
	mode = QOOL_UNDEFINED; 
	frequency = -1; 
	protocols = PROTO_UNDEFINED; 
	key_mgmt = KM_UNDEFINED; 
	auth_alg = AUTHALG_UNDEFINED; 
	pairwise = PCI_UNDEFINED; 
	group = GCI_UNDEFINED; 
	QString psk = QString(); 
	eapol_flags = EAPF_UNDEFINED;
	mixed_cell = QOOL_UNDEFINED; 
	proactive_key_caching = QOOL_UNDEFINED; 
	wep_key0 = QString(); 
	wep_key1 = QString();
	wep_key2 = QString();
	wep_key3 = QString();
	wep_tx_keyidx = -1; 
	peerkey = QOOL_UNDEFINED; 
	eap = EAPM_UNDEFINED; 
	fragment_size = -1; 
	nai = QString(); 
}

CNetworkConfig::CNetworkConfig(const CNetworkConfig &c) :
	ssid(c.ssid),
	bssid(c.bssid),
	disabled(c.disabled),
	id_str(c.id_str), 
	scan_ssid(c.scan_ssid), 
	priority(c.priority),
	mode(c.mode), 
	frequency(c.frequency),
	protocols(c.protocols), 
	key_mgmt(c.key_mgmt), 
	auth_alg(c.auth_alg), 
	pairwise(c.pairwise), 
	group(c.group), 
	psk(c.psk), 
	eapol_flags(c.eapol_flags), 
	mixed_cell(c.mixed_cell), 
	proactive_key_caching(c.proactive_key_caching), 
	wep_key0(c.wep_key0), 
	wep_key1(c.wep_key1),
	wep_key2(c.wep_key2),
	wep_key3(c.wep_key3),
	wep_tx_keyidx(c.wep_tx_keyidx), 
	peerkey(c.peerkey), 
	eap(c.eap), 
	identity(c.identity), 
	anonymous_identity(c.anonymous_identity), 
	password(c.password), 
	ca_cert(c.ca_cert), 
	ca_path(c.ca_path), 
	client_cert(c.client_cert), 
	private_key(c.private_key), 
	private_key_passwd(c.private_key_passwd), 
	dh_file(c.dh_file), 
	subject_match(c.subject_match), 
	altsubject_match(c.altsubject_match), 
	phase1(c.phase1), 
	phase2(c.phase2), 
	ca_cert2(c.ca_cert2), 
	ca_path2(c.ca_path2), 
	client_cert2(c.client_cert2), 
	private_key2(c.private_key2), 
	private_key2_passwd(c.private_key2_passwd), 
	dh_file2(c.dh_file2), 
	subject_match2(c.subject_match2), 
	altsubject_match2(c.altsubject_match2), 
	fragment_size(c.fragment_size), 
	eappsk(c.eappsk), 
	nai(c.nai), 
	pac_file(c.pac_file)
{}


CNetworkConfig::CNetworkConfig(ScanResult scan) {
	CNetworkConfig();
	bssid = scan.bssid;
	ssid = scan.ssid;
	group = scan.group;
	pairwise = scan.pairwise;
	key_mgmt = scan.keyManagement;
	protocols =  scan.protocols;
	if (scan.opmode == OPM_ADHOC) {
		frequency = scan.freq;
		mode = QOOL_TRUE;
	}
	else {
		mode = QOOL_FALSE;
	}
}

CNetworkConfig::~CNetworkConfig() {
}


CNetworkConfig::NetworkId CNetworkConfig::toNetworkId(QString str) {
	CNetworkConfig::NetworkId netid;
	netid.id = -1;
	netid.pid = -1;
	if (0 == str.indexOf("\"")) { //String is in "", remove them
		str.remove(0,1);
		str.chop(1);
	}
	if (0 != str.indexOf("nut:"))
		return netid;
	else {
		QStringList list = str.split(":");
		if (list.size() != 3)
			return netid;
		else {
			bool ok;
			netid.id = list[2].toInt(&ok);
			if (!ok) {
				netid.id = -1;
				return netid;
			}
			netid.pid = list[1].toInt(&ok);
			if (!ok) {
				netid.pid = -1;
				return netid;
			}
			return netid;
		}
	}
}

//TODO:check if \" is need, guess not, since wpa_supplicant needs them

void CNetworkConfig::writeTo(QDataStream &stream) {
	stream << QString("network {\n");
	if (!ssid.isEmpty())
		stream << QString("ssid=%1\n").arg(ssid);
	if (!bssid.valid())
		stream << QString("bssid=%1\n").arg( bssid.toString());
	if (QOOL_UNDEFINED != disabled)
		stream << QString("disabled=%1\n").arg( toString(disabled));
	if (!id_str.isEmpty())
		stream << QString("id_str=%1\n").arg( id_str);
	if (QOOL_UNDEFINED != scan_ssid)
		stream << QString("scan_ssid=%1\n").arg( toString(scan_ssid)); 
	if (priority >= 0)
		stream << QString("priority=%1\n").arg( QString::number(priority));
	if (QOOL_UNDEFINED != mode)
		stream << QString("mode=%1\n").arg( toString(mode)); 
	if (frequency != -1)
		stream << QString("frequency=%1\n").arg( frequency); 
	if (PROTO_UNDEFINED != protocols)
		stream << QString("protocols=%1\n").arg( toString(protocols)); 
	if (KM_UNDEFINED != key_mgmt)
		stream << QString("key_mgmt=%1\n").arg( toString(key_mgmt));
	if (AUTHALG_UNDEFINED != auth_alg)
		stream << QString("auth_alg=%1\n").arg( toString(auth_alg)); 
	if (PCI_UNDEFINED != pairwise)
		stream << QString("pairwise=%1\n").arg( toString(pairwise)); 
	if (GCI_UNDEFINED != group)
		stream << QString("group=%1\n").arg( toString(group)); 
	if (!psk.isEmpty())
		stream << QString("psk=%1\n").arg( psk); 
	if (EAPF_UNDEFINED != eapol_flags)
		stream << QString("eapol_flags=%1\n").arg( toString(eapol_flags));
	if (QOOL_UNDEFINED != mixed_cell)
		stream << QString("mixed_cell=%1\n").arg( toString(mixed_cell)); 
	if (QOOL_UNDEFINED != proactive_key_caching)
		stream << QString("proactive_key_caching=%1\n").arg( toString(proactive_key_caching)); 
	if (!wep_key0.isEmpty())
		stream << QString("wep_key0=%1\n").arg( wep_key0); 
	if (!wep_key1.isEmpty())
		stream << QString("wep_key1=%1\n").arg( wep_key1);
	if (!wep_key2.isEmpty())
		stream << QString("wep_key2=%1\n").arg( wep_key2);
	if (!wep_key3.isEmpty())
		stream << QString("wep_key3=%1\n").arg( wep_key3);
	if (-1 != wep_tx_keyidx)
		stream << QString("wep_tx_keyidx=%1\n").arg( (int)wep_tx_keyidx); 
	if (QOOL_UNDEFINED != peerkey)
		stream << QString("peerkey=%1\n").arg((int)peerkey);

	if (EAPM_UNDEFINED != eap)
		stream << QString("eap=%1\n").arg( toString(eap)); 
	if (!identity.isEmpty())
		stream << QString("identity=%1\n").arg( identity); 
	if (!anonymous_identity.isEmpty())
		stream << QString("anonymous_identity=%1\n").arg( anonymous_identity); 
	if (!password.isEmpty())
		stream << QString("password=%1\n").arg( password); 
	if (!ca_cert.isEmpty())
		stream << QString("ca_cert=%1\n").arg(ca_cert); 
	if (!ca_path.isEmpty())
		stream << QString("ca_path=%1\n").arg( ca_path); 
	if (!client_cert.isEmpty())
		stream << QString("client_cert=%1\n").arg( client_cert); 
	if (!private_key.isEmpty())
		stream << QString("private_key=%1\n").arg( private_key); 
	if (!private_key_passwd.isEmpty())
		stream << QString("private_key_passwd=%1\n").arg( private_key_passwd); 
	if (!dh_file.isEmpty())
		stream << QString("dh_file=%1\n").arg( dh_file); 
	if (!subject_match.isEmpty())
		stream << QString("subject_match=%1\n").arg( subject_match); 
	if (!altsubject_match.isEmpty())
		stream << QString("altsubject_match=%1\n").arg( altsubject_match);
	if (!phase1.isEmpty())
		stream << QString("phase1=%1\n").arg( phase1); 
	if (!phase2.isEmpty())
		stream << QString("phase2=%1\n").arg( phase2); 
	if (!ca_cert2.isEmpty())
		stream << QString("ca_cert2=%1\n").arg( ca_cert2); 
	if (!ca_path2.isEmpty())
		stream << QString("ca_path2=%1\n").arg( ca_path2); 
	if (!client_cert2.isEmpty())
		stream << QString("client_cert2=%1\n").arg( client_cert2); 
	if (!private_key2.isEmpty())
		stream << QString("private_key2=%1\n").arg( private_key2); 
	if (!private_key2_passwd.isEmpty())
		stream << QString("private_key2_passwd=%1\n").arg( private_key2_passwd); 
	if (!dh_file2.isEmpty())
		stream << QString("dh_file2=%1\n").arg( dh_file2); 
	if (!subject_match2.isEmpty())
		stream << QString("subject_match2=%1\n").arg( subject_match2); 
	if (!altsubject_match2.isEmpty())
		stream << QString("altsubject_match2=%1\n").arg( altsubject_match2); 
	if (fragment_size != -1)
		stream << QString("fragment_size=%1\n").arg( fragment_size); 
	if (!eappsk.isEmpty())
		stream << QString("eappsk=%1\n").arg( eappsk); 
	if (!eappsk.isEmpty())
		stream << QString("nai=%1\n").arg( nai); 
	if (!pac_file.isEmpty())
		stream << QString("pac_file=%1\n").arg( pac_file); 
	stream << QString("}\n");
}

//parser stuff
bool CNetworkConfig::set_ssid(QString str) {
	ssid = str;
	return true;
}
bool CNetworkConfig::set_bssid(libnutcommon::MacAddress addrr) {
	bssid = addrr;
	return true;
}
bool CNetworkConfig::set_disabled(bool d) {
	disabled = toQOOL(d);
	return true;
}
bool CNetworkConfig::set_id_str(QString str) {
	id_str = str;
	netId = toNetworkId(str);
	return true;
}
bool CNetworkConfig::set_scan_ssid(bool enabled) {
	scan_ssid = toQOOL(enabled);
	return true;
}
bool CNetworkConfig::set_priority(int p) {
	if (priority >= 0) {
		priority = p;
		return true;
	}
	return false;
}
bool CNetworkConfig::set_mode(bool mode) {
	mode = toQOOL(mode);
	return true;
}
bool CNetworkConfig::set_frequency(int freq) {
	if (freq >= 0) {
		frequency = freq;
		return true;
	}
	else return false;
}
bool CNetworkConfig::set_proto(QString p) {
	protocols = toProtocols(p);
	return true;
}
bool CNetworkConfig::set_key_mgmt(QString k) {
	key_mgmt = toKeyMgmt(k);
	return true;
}
bool CNetworkConfig::set_auth_alg(QString a) {
	auth_alg = toAuthAlg(a);
	return true;
}
bool CNetworkConfig::set_pairwise(QString p) {
	pairwise = toPairwiseCiphers(p);
	return true;
}
bool CNetworkConfig::set_group(QString g) {
	group = toGroupCiphers(g);
	return true;
}
bool CNetworkConfig::set_psk(QString p) {
	psk = p;
	return true;
}
bool CNetworkConfig::set_eapol_flags(QString e) {
	eapol_flags = toEapolFlags(e);
	return true;
}
bool CNetworkConfig::set_mixed_cell(bool enabled) {
	mixed_cell = toQOOL(enabled);
	return true;
}
bool CNetworkConfig::set_proactive_key_caching(bool enabled) {
	proactive_key_caching = toQOOL(enabled);
	return true;
}
bool CNetworkConfig::set_wep_key0(QString key) {
	wep_key0 = key;
	return true;
}
bool CNetworkConfig::set_wep_key1(QString key) {
	wep_key1 = key;
	return true;
}
bool CNetworkConfig::set_wep_key2(QString key) {
	wep_key2 = key;
	return true;
}
bool CNetworkConfig::set_wep_key3(QString key) {
	wep_key3 = key;
	return true;
}
bool CNetworkConfig::set_wep_tx_keyidx(int idx) {
	if (idx >= 0 && idx <= 3) {
		wep_tx_keyidx = idx;
		return true;
	}
	else return false;
}
bool CNetworkConfig::set_peerkey(bool enabled) {
	peerkey = toQOOL(enabled);
	return true;
}
bool CNetworkConfig::set_eap(QString str) {
	eap = toEapMethod(str);
	return true;
}
bool CNetworkConfig::set_identity(QString str) {
	identity = str;
	return true;
}
bool CNetworkConfig::set_anonymous_identity(QString str) {
	anonymous_identity = str;
	return true;
}
bool CNetworkConfig::set_password(QString str) {
	password = str;
	return true;
}
bool CNetworkConfig::set_ca_cert(QString str) {
	ca_cert = str;
	return true;
}
bool CNetworkConfig::set_ca_path(QString str) {
	ca_path = str;
	return true;
}
bool CNetworkConfig::set_client_cert(QString str) {
	client_cert = str;
	return true;
}
bool CNetworkConfig::set_private_key(QString str) {
	private_key = str;
	return true;
}
bool CNetworkConfig::set_private_key_passwd(QString str) {
	private_key_passwd = str;
	return true;
}
bool CNetworkConfig::set_dh_file(QString str) {
	dh_file = str;
	return true;
}
bool CNetworkConfig::set_subject_match(QString str) {
	subject_match = str;
	return true;
}
bool CNetworkConfig::set_altsubject_match(QString str) {
	altsubject_match = str;
	return true;
}
bool CNetworkConfig::set_phase1(QString str) {
	phase1 = str;
	return true;
}
bool CNetworkConfig::set_phase2(QString str) {
	phase2 = str;
	return true;
}
bool CNetworkConfig::set_ca_cert2(QString str) {
	ca_cert2 = str;
	return true;
}
bool CNetworkConfig::set_ca_path2(QString str) {
	ca_path2 = str;
	return true;
}
bool CNetworkConfig::set_client_cert2(QString str) {
	client_cert2 = str;
	return true;
}
bool CNetworkConfig::set_private_key2(QString str) {
	private_key2 = str;
	return true;
}
bool CNetworkConfig::set_private_key2_passwd(QString str) {
	private_key2_passwd = str;
	return true;
}
bool CNetworkConfig::set_dh_file2(QString str) {
	dh_file2 = str;
	return true;
}
bool CNetworkConfig::set_subject_match2(QString str) {
	subject_match2 = str;
	return true;
}
bool CNetworkConfig::set_altsubject_match2(QString str) {
	altsubject_match2 = str;
	return true;
}
bool CNetworkConfig::set_fragment_size(int size) {
	fragment_size = size;
	return true;
}
bool CNetworkConfig::set_eappsk(QString str) {
	eappsk = str;
	return true;
}
bool CNetworkConfig::set_nai(QString str) {
	nai = str;
	return true;
}
bool CNetworkConfig::set_pac_file(QString str) {
	pac_file = str;
	return true;
}

}