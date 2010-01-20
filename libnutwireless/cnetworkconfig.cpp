#include "cnetworkconfig.h"
namespace libnutwireless {

CNetworkConfig::CNetworkConfig() {
			//Set default values
			ssid = QString();
			bssid = libnutcommon::MacAddress();
			disabled = QOOL_UNDEFINED;
			id_str = QString();
			scan_ssid = QOOL_UNDEFINED; // (do not) scan with SSID-specific Probe Request frames
			priority = -1;
			mode = QOOL_UNDEFINED; //0 = infrastructure (Managed) mode, i.e., associate with an AP (default) 1 = IBSS (ad-hoc, peer-to-peer)
			frequency = -1; //no default, but -1 is not a working value
			protocols = PROTO_UNDEFINED; //list of accepted protocols
			key_mgmt = KM_UNDEFINED; // list of accepted authenticated key management protocols
			auth_alg = AUTHALG_UNDEFINED; //list of allowed IEEE 802.11 authentication algorithms
			pairwise = PCI_UNDEFINED; //list of accepted pairwise (unicast) ciphers for WPA (CCMP,TKIP,NONE)
			group = GCI_UNDEFINED; //list of accepted group (broadcast/multicast) ciphers for WPA (CCMP;TKIP;WEP104/40)
			QString psk = QString(); //WPA preshared key; 256-bit pre-shared key
			eapol_flags = EAPF_UNDEFINED;
			mixed_cell = QOOL_UNDEFINED; //This option can be used to configure whether so called mixed
			proactive_key_caching = QOOL_UNDEFINED; //Enable/disable opportunistic PMKSA caching for WPA2.
			wep_key0 = QString(); //Static WEP key (ASCII in double quotation, hex without)
			wep_key1 = QString();
			wep_key2 = QString();
			wep_key3 = QString();
			wep_tx_keyidx = -1; //Default WEP key index (TX) (0..3)
			peerkey = QOOL_UNDEFINED; //Whether PeerKey negotiation for direct links (IEEE 802.11e DLS) is allowed.
			eap = EAPM_UNDEFINED; //space-separated list of accepted EAP methods
			fragment_size = -1; //Maximum EAP fragment size in bytes (default 1398);
			nai = QString(); //user NAI
}

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

//TODO:check if \" is need, guess not, since wpa_supplicant needs them

void CNetworkConfig::writeTo(QDataStream &stream) {
	stream << QString("network {\n");
	if (!ssid.isEmpty())
		stream << QString("ssid=\"%1\"\n").arg(ssid);
	if (!bssid.valid())
		stream << QString("bssid=%1\n").arg( bssid.toString());
	if (QOOL_UNDEFINED != disabled)
		stream << QString("disabled=%1\n").arg( toString(disabled));
	if (!id_str.isEmpty())
		stream << QString("id_str=\"%1\"\n").arg( id_str);
	if (QOOL_UNDEFINED != scan_ssid)
		stream << QString("scan_ssid=%1\n").arg( toString(scan_ssid)); 
	if (priority >= 0)
		stream << QString("priority=%1\n").arg( QString::number(priority));
	if (QOOL_UNDEFINED != mode)
		stream << QString("mode=%1\n").arg( toString(mode)); 
	if (frequency != -1)
		stream << QString("frequency=%1\n").arg( frequency); 
	if (PROTO_UNDEFINED != protocols)
		stream << QString("protocols=\"%1\"\n").arg( toString(protocols)); 
	if (KM_UNDEFINED != key_mgmt)
		stream << QString("key_mgmt=%1\n").arg( toString(key_mgmt));
	if (AUTHALG_UNDEFINED != auth_alg)
		stream << QString("auth_alg=\"%1\"\n").arg( toString(auth_alg)); 
	if (PCI_UNDEFINED != pairwise)
		stream << QString("pairwise=\"%1\"\n").arg( toString(pairwise)); 
	if (GCI_UNDEFINED != group)
		stream << QString("group=\"%1\"\n").arg( toString(group)); 
	if (!psk.isEmpty())
		stream << QString("psk=\"%1\"\n").arg( psk); 
	if (EAPF_UNDEFINED != eapol_flags)
		stream << QString("eapol_flags=\"%1\"\n").arg( toString(eapol_flags));
	if (QOOL_UNDEFINED != mixed_cell)
		stream << QString("mixed_cell=\"%1\"\n").arg( toString(mixed_cell)); 
	if (QOOL_UNDEFINED != proactive_key_caching)
		stream << QString("proactive_key_caching=%1\n").arg( toString(proactive_key_caching)); 
	if (!wep_key0.isEmpty())
		stream << QString("wep_key0=\"%1\"\n").arg( wep_key0); 
	if (!wep_key1.isEmpty())
		stream << QString("wep_key1=\"%1\"\n").arg( wep_key1);
	if (!wep_key2.isEmpty())
		stream << QString("wep_key2=\"%1\"\n").arg( wep_key2);
	if (!wep_key3.isEmpty())
		stream << QString("wep_key3=\"%1\"\n").arg( wep_key3);
	if (-1 != wep_tx_keyidx)
		stream << QString("wep_tx_keyidx=%1\n").arg( (int)wep_tx_keyidx); 
	if (QOOL_UNDEFINED != peerkey)
		stream << QString("peerkey=%1\n").arg((int)peerkey);

	if (EAPM_UNDEFINED != eap)
		stream << QString("eap=\"%1\"\n").arg( toString(eap)); 
	if (!identity.isEmpty())
		stream << QString("identity=\"%1\"\n").arg( identity); 
	if (!anonymous_identity.isEmpty())
		stream << QString("anonymous_identity=\"%1\"\n").arg( anonymous_identity); 
	if (!password.isEmpty())
		stream << QString("password=\"%1\"\n").arg( password); 
	if (!ca_cert.isEmpty())
		stream << QString("ca_cert=\"%1\"\n").arg(ca_cert); 
	if (!ca_path.isEmpty())
		stream << QString("ca_path=\"%1\"\n").arg( ca_path); 
	if (!client_cert.isEmpty())
		stream << QString("client_cert=\"%1\"\n").arg( client_cert); 
	if (!private_key.isEmpty())
		stream << QString("private_key=\"%1\"\n").arg( private_key); 
	if (!private_key_passwd.isEmpty())
		stream << QString("private_key_passwd=\"%1\"\n").arg( private_key_passwd); 
	if (!dh_file.isEmpty())
		stream << QString("dh_file=\"%1\"\n").arg( dh_file); 
	if (!subject_match.isEmpty())
		stream << QString("subject_match=\"%1\"\n").arg( subject_match); 
	if (!altsubject_match.isEmpty())
		stream << QString("altsubject_match=\"%1\"\n").arg( altsubject_match);
	if (!phase1.isEmpty())
		stream << QString("phase1=\"%1\"\n").arg( phase1); 
	if (!phase2.isEmpty())
		stream << QString("phase2=\"%1\"\n").arg( phase2); 
	if (!ca_cert2.isEmpty())
		stream << QString("ca_cert2=\"%1\"\n").arg( ca_cert2); 
	if (!ca_path2.isEmpty())
		stream << QString("ca_path2=\"%1\"\n").arg( ca_path2); 
	if (!client_cert2.isEmpty())
		stream << QString("client_cert2=\"%1\"\n").arg( client_cert2); 
	if (!private_key2.isEmpty())
		stream << QString("private_key2=\"%1\"\n").arg( private_key2); 
	if (!private_key2_passwd.isEmpty())
		stream << QString("private_key2_passwd=\"%1\"\n").arg( private_key2_passwd); 
	if (!dh_file2.isEmpty())
		stream << QString("dh_file2=\"%1\"\n").arg( dh_file2); 
	if (!subject_match2.isEmpty())
		stream << QString("subject_match2=\"%1\"\n").arg( subject_match2); 
	if (!altsubject_match2.isEmpty())
		stream << QString("altsubject_match2=\"%1\"\n").arg( altsubject_match2); 
	if (fragment_size != -1)
		stream << QString("fragment_size=%1\n").arg( fragment_size); 
	if (!eappsk.isEmpty())
		stream << QString("eappsk=\"%1\"\n").arg( eappsk); 
	if (!eappsk.isEmpty())
		stream << QString("nai=\"%1\"\n").arg( nai); 
	if (!pac_file.isEmpty())
		stream << QString("pac_file=\"%1\"\n").arg( pac_file); 
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