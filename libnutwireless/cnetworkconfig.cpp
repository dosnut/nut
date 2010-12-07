#include "cnetworkconfig.h"
#include <iostream>
namespace libnutwireless {


CNetworkConfig::CNetworkConfig() :
	//Set default values
	disabled(QOOL_UNDEFINED),
	scan_ssid(QOOL_UNDEFINED),
	priority(-1),
	mode(QOOL_UNDEFINED),
	frequency(-1),
	protocols(PROTO_UNDEFINED),
	key_mgmt(KM_UNDEFINED),
	auth_alg(AUTHALG_UNDEFINED),
	pairwise(PCI_UNDEFINED),
	group(GCI_UNDEFINED),
	eapol_flags(EAPF_UNDEFINED),
	mixed_cell(QOOL_UNDEFINED),
	proactive_key_caching(QOOL_UNDEFINED),
	wep_tx_keyidx(-1),
	peerkey(QOOL_UNDEFINED),
	eap(EAPM_UNDEFINED),
	fragment_size(-1)
{
	netId.id = -1;
	netId.pid = -1;
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
	pac_file(c.pac_file),
	netId(c.netId)
{}

CNetworkConfig::CNetworkConfig(ScanResult scan) :
	ssid(scan.ssid),
	bssid(scan.bssid),
	disabled(QOOL_UNDEFINED),
	scan_ssid(QOOL_UNDEFINED),
	priority(-1),
	mode(scan.opmode == OPM_ADHOC ? QOOL_TRUE : QOOL_FALSE),
	frequency(scan.opmode == OPM_ADHOC ? scan.freq : -1),
	protocols(scan.protocols),
	key_mgmt(scan.keyManagement),
	auth_alg(AUTHALG_UNDEFINED),
	pairwise(scan.pairwise),
	group(scan.group),
	eapol_flags(EAPF_UNDEFINED),
	mixed_cell(QOOL_UNDEFINED),
	proactive_key_caching(QOOL_UNDEFINED),
	wep_tx_keyidx(-1),
	peerkey(QOOL_UNDEFINED),
	eap(EAPM_UNDEFINED),
	fragment_size(-1)
{
	netId.id = -1;
	netId.pid = -1;
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

void CNetworkConfig::writeTo(QTextStream &stream) {
	stream << QString("network {\n");
	if (!ssid.isEmpty())
		stream << QString("ssid=%1\n").arg(ssid);
	if (!bssid.valid() && !bssid.zero())
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
		stream << QString("proto=%1\n").arg( toString(protocols)); 
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
	if (-1 != wep_tx_keyidx) {
		bool ok=false;
		if (0 == wep_tx_keyidx && !wep_key0.isEmpty()) ok=true;
		else if (1 == wep_tx_keyidx && !wep_key1.isEmpty()) ok=true;
		else if (2 == wep_tx_keyidx && !wep_key2.isEmpty()) ok=true;
		else if (3 == wep_tx_keyidx && !wep_key3.isEmpty()) ok=true;
		if (ok) stream << QString("wep_tx_keyidx=%1\n").arg( (int)wep_tx_keyidx);
	}
	if (QOOL_UNDEFINED != peerkey)
		stream << QString("peerkey=%1\n").arg(toString(peerkey));

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

#define SET_EQUAL_TO(a, b, c) a = (a == b) ? c : a

bool CNetworkConfig::setEqualsToUndefinded(CNetworkConfig & other) {
	SET_EQUAL_TO(ssid, other.ssid, QString());
	SET_EQUAL_TO(bssid, other.bssid, libnutcommon::MacAddress());
	SET_EQUAL_TO(disabled, other.disabled, QOOL_UNDEFINED);
	SET_EQUAL_TO(id_str, other.id_str, QString());
	SET_EQUAL_TO(scan_ssid, other.scan_ssid, QOOL_UNDEFINED);
	SET_EQUAL_TO(priority, other.priority, -1);
	SET_EQUAL_TO(mode, other.mode, QOOL_UNDEFINED);
	SET_EQUAL_TO(frequency, other.frequency, -1);
	SET_EQUAL_TO(protocols, other.protocols, PROTO_UNDEFINED);
	SET_EQUAL_TO(key_mgmt, other.key_mgmt, KM_UNDEFINED);
	SET_EQUAL_TO(auth_alg, other.auth_alg, AUTHALG_UNDEFINED);
	SET_EQUAL_TO(pairwise, other.pairwise, PCI_UNDEFINED);
	SET_EQUAL_TO(group, other.group, GCI_UNDEFINED);
	SET_EQUAL_TO(psk, other.psk, QString());
	SET_EQUAL_TO(eapol_flags, other.eapol_flags, EAPF_UNDEFINED);
	SET_EQUAL_TO(mixed_cell, other.mixed_cell, QOOL_UNDEFINED);
	SET_EQUAL_TO(proactive_key_caching, other.proactive_key_caching, QOOL_UNDEFINED);
	
	SET_EQUAL_TO(wep_key0, other.wep_key0, QString());
	SET_EQUAL_TO(wep_key1, other.wep_key1, QString());
	SET_EQUAL_TO(wep_key2, other.wep_key2, QString());
	SET_EQUAL_TO(wep_key3, other.wep_key3, QString());
	SET_EQUAL_TO(wep_tx_keyidx, other.wep_tx_keyidx, -1);
	
	SET_EQUAL_TO(peerkey, other.peerkey, QOOL_UNDEFINED);
	
	SET_EQUAL_TO(eap, other.eap, EAPM_UNDEFINED);
	SET_EQUAL_TO(identity, other.identity, QString());
	SET_EQUAL_TO(anonymous_identity, other.anonymous_identity, QString());
	SET_EQUAL_TO(password, other.password, QString());
	
	SET_EQUAL_TO(ca_cert, other.ca_cert, QString());
	SET_EQUAL_TO(ca_path, other.ca_path, QString());
	SET_EQUAL_TO(client_cert, other.client_cert, QString());
	SET_EQUAL_TO(private_key, other.private_key, QString());
	SET_EQUAL_TO(private_key_passwd, other.private_key_passwd, QString());
	SET_EQUAL_TO(dh_file, other.dh_file, QString());
	SET_EQUAL_TO(subject_match, other.subject_match, QString());
	SET_EQUAL_TO(altsubject_match, other.altsubject_match, QString());
	
	SET_EQUAL_TO(ca_cert2, other.ca_cert2, QString());
	SET_EQUAL_TO(ca_path2, other.ca_path2, QString());
	SET_EQUAL_TO(client_cert2, other.client_cert2, QString());
	SET_EQUAL_TO(private_key2, other.private_key2, QString());
	SET_EQUAL_TO(private_key2_passwd, other.private_key2_passwd, QString());
	SET_EQUAL_TO(dh_file2, other.dh_file2, QString());
	SET_EQUAL_TO(subject_match2, other.subject_match2, QString());
	SET_EQUAL_TO(altsubject_match2, other.altsubject_match2, QString());
	
	SET_EQUAL_TO(phase1, other.phase1, QString());
	SET_EQUAL_TO(phase2, other.phase2, QString());
	
	SET_EQUAL_TO(fragment_size, other.fragment_size, -1);
	SET_EQUAL_TO(eappsk, other.eappsk, QString());
	SET_EQUAL_TO(nai, other.nai, QString());
	SET_EQUAL_TO(pac_file, other.pac_file, QString());
}

#define QUOTED(a) '\"' + a + '\"'
#define DEP_QUOTED(a, b) (b ? QUOTED(a) : a) 

//parser stuff
bool CNetworkConfig::set_ssid(QString value, bool addQuotes) {
	ssid = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_bssid(libnutcommon::MacAddress value) {
	bssid = value;
	return true;
}
bool CNetworkConfig::set_disabled(bool value) {
	disabled = toQOOL(value);
	return true;
}
bool CNetworkConfig::set_id_str(QString value) {
	id_str = value;
	netId = toNetworkId(value);
	return true;
}
bool CNetworkConfig::set_scan_ssid(bool value) {
	scan_ssid = toQOOL(value);
	return true;
}
bool CNetworkConfig::set_priority(int value) {
	if (priority >= 0) {
		priority = value;
		return true;
	}
	return false;
}
bool CNetworkConfig::set_mode(bool value) {
	mode = toQOOL(value);
	return true;
}
bool CNetworkConfig::set_frequency(int value) {
	if (value > 0) {
		frequency = value;
		return true;
	}
	else return false;
}
bool CNetworkConfig::set_proto(QString value) {
	protocols = toProtocols(value);
	return true;
}
bool CNetworkConfig::set_key_mgmt(QString value) {
	key_mgmt = toKeyMgmt(value);
	return true;
}
bool CNetworkConfig::set_auth_alg(QString value) {
	auth_alg = toAuthAlg(value);
	return true;
}
bool CNetworkConfig::set_pairwise(QString value) {
	pairwise = toPairwiseCiphers(value);
	return true;
}
bool CNetworkConfig::set_group(QString value) {
	group = toGroupCiphers(value);
	return true;
}
bool CNetworkConfig::set_psk(QString value, bool addQuotes) {
	if (value != "*") {
		psk = DEP_QUOTED(value, addQuotes);
		return true;
	}
	return false;
}
bool CNetworkConfig::set_eapol_flags(QString value) {
	eapol_flags = toEapolFlags(value);
	return true;
}
bool CNetworkConfig::set_eapol_flags(int value) {
	if (value >= 0 && value <= 3) {
		eapol_flags = (EapolFlags)value;
		return true;
	}
	return false;
}
bool CNetworkConfig::set_mixed_cell(bool value) {
	mixed_cell = toQOOL(value);
	return true;
}
bool CNetworkConfig::set_proactive_key_caching(bool value) {
	proactive_key_caching = toQOOL(value);
	return true;
}
bool CNetworkConfig::set_wep_key0(QString value, bool addQuotes) {
	if (value != "*") {
		wep_key0 = DEP_QUOTED(value, addQuotes);
		return true;
	}
	return false;
}
bool CNetworkConfig::set_wep_key1(QString value, bool addQuotes) {
	if (value != "*") {
		wep_key1 = DEP_QUOTED(value, addQuotes);
		return true;
	}
	return false;
}
bool CNetworkConfig::set_wep_key2(QString value, bool addQuotes) {
	if (value != "*") {
		wep_key2 = DEP_QUOTED(value, addQuotes);
		return true;
	}
	return false;
}
bool CNetworkConfig::set_wep_key3(QString value, bool addQuotes) {
	if (value != "*") {
		wep_key3 = DEP_QUOTED(value, addQuotes);;
		return true;
	}
	return false;
}
bool CNetworkConfig::set_wep_tx_keyidx(int value) {
	if (value >= 0 && value <= 3) {
		wep_tx_keyidx = value;
		return true;
	}
	else return false;
}
bool CNetworkConfig::set_peerkey(bool value) {
	peerkey = toQOOL(value);
	return true;
}
bool CNetworkConfig::set_eap(QString value) {
	eap = toEapMethod(value);
	return true;
}
bool CNetworkConfig::set_identity(QString value, bool addQuotes) {
	identity = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_anonymous_identity(QString value, bool addQuotes) {
	anonymous_identity = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_password(QString value, bool addQuotes) {
	if (value != "*") {
		password = DEP_QUOTED(value, addQuotes);
		return true;
	}
	return false;
}
bool CNetworkConfig::set_ca_cert(QString value, bool addQuotes) {
	ca_cert = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_ca_path(QString value, bool addQuotes) {
	ca_path = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_client_cert(QString value, bool addQuotes) {
	client_cert = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_private_key(QString value, bool addQuotes) {
	private_key = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_private_key_passwd(QString value, bool addQuotes) {
	if (value != "*") {
		private_key_passwd = DEP_QUOTED(value, addQuotes);
		return true;
	}
	return false;
}
bool CNetworkConfig::set_dh_file(QString value, bool addQuotes) {
	dh_file = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_subject_match(QString value, bool addQuotes) {
	subject_match = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_altsubject_match(QString value, bool addQuotes) {
	altsubject_match = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_phase1(QString value, bool addQuotes) {
	phase1 = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_phase2(QString value, bool addQuotes) {
	phase2 = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_ca_cert2(QString value, bool addQuotes) {
	ca_cert2 = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_ca_path2(QString value, bool addQuotes) {
	ca_path2 = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_client_cert2(QString value, bool addQuotes) {
	client_cert2 = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_private_key2(QString value, bool addQuotes) {
	private_key2 = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_private_key2_passwd(QString value, bool addQuotes) {
	if (value != "*") {
		private_key2_passwd = DEP_QUOTED(value, addQuotes);
		return true;
	}
	return false;
}
bool CNetworkConfig::set_dh_file2(QString value, bool addQuotes) {
	dh_file2 = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_subject_match2(QString value, bool addQuotes) {
	subject_match2 = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_altsubject_match2(QString value, bool addQuotes) {
	altsubject_match2 = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_fragment_size(int value) {
	fragment_size = value;
	return true;
}
bool CNetworkConfig::set_eappsk(QString value) {
	if (value != "*") {
		eappsk = value;
		return true;
	}
	return false;
}
bool CNetworkConfig::set_nai(QString value, bool addQuotes) {
	nai = DEP_QUOTED(value, addQuotes);
	return true;
}
bool CNetworkConfig::set_pac_file(QString value, bool addQuotes) {
	pac_file = DEP_QUOTED(value, addQuotes);
	return true;
}

}
