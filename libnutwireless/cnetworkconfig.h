#ifndef LIBNUTWIRELESS_CNETWORKCONFIG_H
#define LIBNUTWIRELESS_CNETWORKCONFIG_H

#pragma once

#include "hwtypes.h"
#include <QTextStream>

namespace libnutwireless {
	/**
		The network config class contains all information for configuring a network.
		On instantiation all values will be set to undefined.
	*/
	class CNetworkConfig final {
	public:
		struct NetworkId {
			qint64 pid{-1};
			qint32 id{-1};
		};
		static NetworkId toNetworkId(QString str);

	public:
		explicit CNetworkConfig();
		explicit CNetworkConfig(ScanResult const& scan);

		void writeTo(QTextStream &stream);

		void setEqualsToUndefinded(CNetworkConfig const& other);

		//Access functions
		libnutcommon::SSID const& get_ssid() const { return ssid;}
		libnutcommon::MacAddress const& get_bssid() const { return bssid; }
		QOOL get_disabled() const { return disabled; }
		QString const& get_id_str() const { return id_str; }
		QOOL get_scan_ssid() const { return scan_ssid; }
		int get_priority() const { return priority; }
		QOOL get_mode() const { return mode; }
		int get_frequency() const { return frequency; }
		Protocols get_protocols() const { return protocols; }
		KeyManagement get_key_mgmt() const { return key_mgmt; }
		AuthenticationAlgs get_auth_alg() const { return auth_alg; }
		PairwiseCiphers get_pairwise() const { return pairwise; }
		GroupCiphers get_group() const { return group; }
		QString const& get_psk() const { return psk; }
		EapolFlags get_eapol_flags() const { return eapol_flags; }
		QOOL get_mixed_cell() const { return mixed_cell; }
		QOOL get_proactive_key_caching() const { return proactive_key_caching; }
		QString const& get_wep_key0() const { return wep_key0; }
		QString const& get_wep_key1() const { return wep_key1; }
		QString const& get_wep_key2() const { return wep_key2; }
		QString const& get_wep_key3() const { return wep_key3; }
		char get_wep_tx_keyidx() const { return wep_tx_keyidx; }
		QOOL get_peerkey() const { return peerkey; }
		EapMethod get_eap() const { return eap; }
		QString const& get_identity() const { return identity; }
		QString const& get_anonymous_identity() const { return anonymous_identity; }
		QString const& get_password() const { return password; }
		QString const& get_ca_cert() const { return ca_cert; }
		QString const& get_ca_path() const { return ca_path; }
		QString const& get_client_cert() const { return client_cert; }
		QString const& get_private_key() const { return private_key; }
		QString const& get_private_key_passwd() const { return private_key_passwd; }
		QString const& get_dh_file() const { return dh_file; }
		QString const& get_subject_match() const { return subject_match; }
		QString const& get_altsubject_match() const { return altsubject_match; }
		QString const& get_phase1() const { return phase1; }
		QString const& get_phase2() const { return phase2; }
		QString const& get_ca_cert2() const { return ca_cert2; }
		QString const& get_ca_path2() const { return ca_path2; }
		QString const& get_client_cert2() const { return client_cert2; }
		QString const& get_private_key2() const { return private_key2; }
		QString const& get_private_key2_passwd() const { return private_key2_passwd; }
		QString const& get_dh_file2() const { return dh_file2; }
		QString const& get_subject_match2() const { return subject_match2; }
		QString const& get_altsubject_match2() const { return altsubject_match2; }
		int get_fragment_size() const { return fragment_size; }
		QString const& get_pac_file() const { return pac_file; }

		//non config related
		NetworkId getNetworkId() const { return netId; }
		bool hasValidNetworkId() const { return (netId.id != -1 && netId.pid != -1); }

		//Set functions:
		void set_proto(Protocols proto) { protocols = proto; }
		void set_key_mgmt(KeyManagement k) { key_mgmt = k; }
		void set_auth_alg(AuthenticationAlgs algs) { auth_alg = algs; }
		void set_pairwise(PairwiseCiphers p) { pairwise = p; }
		void set_group(GroupCiphers g) { group = g; }
		void set_eap(EapMethod e) { eap = e; }

		void setNetworkId(NetworkId id) { netId = id; id_str = QString("\"nut:%1:%2\"").arg(id.pid).arg(id.id); }

		//Set Parse functions:
		bool set_ssid(libnutcommon::SSID const& value);
		bool set_bssid(libnutcommon::MacAddress const& value);
		bool set_disabled(bool value);
		bool set_id_str(QString const& value);
		bool set_scan_ssid(bool value);
		bool set_priority(int value);
		bool set_mode(bool mode);
		bool set_frequency(int value);
		bool set_proto(QString const& value);
		bool set_key_mgmt(QString const& value);
		bool set_auth_alg(QString const& value);
		bool set_pairwise(QString const& value);
		bool set_group(QString const& value);
		bool set_psk(QString const& value, bool addQuotes = false);
		bool set_eapol_flags(int value);
		bool set_eapol_flags(QString const& value);
		bool set_mixed_cell(bool value);
		bool set_proactive_key_caching(bool value);
		bool set_wep_key0(QString const& value, bool addQuotes = false);
		bool set_wep_key1(QString const& value, bool addQuotes = false);
		bool set_wep_key2(QString const& value, bool addQuotes = false);
		bool set_wep_key3(QString const& value, bool addQuotes = false);
		bool set_wep_tx_keyidx(int value);
		bool set_peerkey(bool value);
		bool set_eap(QString const& value);
		bool set_identity(QString const& value, bool addQuotes = false);
		bool set_anonymous_identity(QString const& value, bool addQuotes = false);
		bool set_password(QString const& value, bool addQuotes = false);
		bool set_ca_cert(QString const& value, bool addQuotes = false);
		bool set_ca_path(QString const& value, bool addQuotes = false);
		bool set_client_cert(QString const& value, bool addQuotes = false);
		bool set_private_key(QString const& value, bool addQuotes = false);
		bool set_private_key_passwd(QString const& value, bool addQuotes = false);
		bool set_dh_file(QString const& value, bool addQuotes = false);
		bool set_subject_match(QString const& value, bool addQuotes = false);
		bool set_altsubject_match(QString const& value, bool addQuotes = false);
		bool set_phase1(QString const& value, bool addQuotes = false);
		bool set_phase2(QString const& value, bool addQuotes = false);
		bool set_ca_cert2(QString const& value, bool addQuotes = false);
		bool set_ca_path2(QString const& value, bool addQuotes = false);
		bool set_client_cert2(QString const& value, bool addQuotes = false);
		bool set_private_key2(QString const& value, bool addQuotes = false);
		bool set_private_key2_passwd(QString const& value, bool addQuotes = false);
		bool set_dh_file2(QString const& value, bool addQuotes = false);
		bool set_subject_match2(QString const& value, bool addQuotes = false);
		bool set_altsubject_match2(QString const& value, bool addQuotes = false);
		bool set_fragment_size(int value);
		bool set_pac_file(QString const& value, bool addQuotes = false);

	private:
		libnutcommon::SSID ssid;
		libnutcommon::MacAddress bssid;
		QOOL disabled{QOOL_UNDEFINED};
		// Network identifier string for external scripts
		QString id_str;
		// (do not) scan with SSID-specific Probe Request frames
		QOOL scan_ssid{QOOL_UNDEFINED};
		int priority{-1};
		// 0 = infrastructure (Managed) mode, i.e., associate with an AP (default) 1 = IBSS (ad-hoc, peer-to-peer)
		QOOL mode{QOOL_UNDEFINED};
		int frequency{-1};
		// list of accepted protocols
		Protocols protocols{PROTO_UNDEFINED};
		// list of accepted authenticated key management protocols
		KeyManagement key_mgmt{KM_UNDEFINED};
		// list of allowed IEEE 802.11 authentication algorithms
		AuthenticationAlgs auth_alg{AUTHALG_UNDEFINED};
		// list of accepted pairwise (unicast) ciphers for WPA (CCMP,TKIP,NONE)
		PairwiseCiphers pairwise{PCI_UNDEFINED};
		// list of accepted group (broadcast/multicast) ciphers for WPA (CCMP;TKIP;WEP104/40)
		GroupCiphers group{GCI_UNDEFINED};
		// WPA preshared key; 256-bit pre-shared key
		QString psk;
		// IEEE 802.1X/EAPOL options (bit field)
		EapolFlags eapol_flags{EAPF_UNDEFINED};
		// this option can be used to configure whether so called mixed
		QOOL mixed_cell{QOOL_UNDEFINED};
		// enable/disable opportunistic PMKSA caching for WPA2.
		QOOL proactive_key_caching{QOOL_UNDEFINED};
		// static WEP keys (ASCII in double quotation, hex without)
		QString wep_key0;
		QString wep_key1;
		QString wep_key2;
		QString wep_key3;
		// default WEP key index (TX) (0..3)
		char wep_tx_keyidx{-1};
		// whether PeerKey negotiation for direct links (IEEE 802.11e DLS) is allowed.
		QOOL peerkey{QOOL_UNDEFINED};

		//eap config part:
		EapMethod eap{EAPM_UNDEFINED}; //space-separated list of accepted EAP methods
		QString identity; //Identity string for EAP
		QString anonymous_identity; //Anonymous identity string for EAP;
		QString password; //Password string for EAP.
		QString ca_cert; //File path to CA certificate file (PEM/DER).
		QString ca_path; //Directory path for CA certificate files (PEM).
		QString client_cert; //File path to client certificate file (PEM/DER)
		QString private_key; //File path to client private key file (PEM/DER/PFX)
		QString private_key_passwd; //Password for private key file
		QString dh_file; //File path to DH/DSA parameters file (in PEM format)
		QString subject_match; //Substring to be matched against the subject of the authentication server certificate.
		QString altsubject_match; //Semicolon separated string of entries to be matched against the alternative subject name of the authentication server certificate.
		QString phase1; //Phase1 (outer authentication, i.e., TLS tunnel) parameters (string with field-value pairs, e.g., "peapver=0" or	"peapver=1 peaplabel=1")
		QString phase2; //Phase2 (inner authentication with TLS tunnel) parameters
		QString ca_cert2; //File path to CA certificate file.
		QString ca_path2; //Directory path for CA certificate files (PEM)
		QString client_cert2; //File path to client certificate file
		QString private_key2; //File path to client private key file
		QString private_key2_passwd; //Password for private key file
		QString dh_file2; //File path to DH/DSA parameters file (in PEM format)
		QString subject_match2; //Substring to be matched against the subject of the authentication server certificate.
		QString altsubject_match2; //Substring to be matched against the alternative subject name of the authentication server certificate.
		int fragment_size{-1}; //Maximum EAP fragment size in bytes (default 1398);
		QString pac_file; //File path for the PAC entries.

		NetworkId netId;
	};

	inline bool operator==(libnutwireless::CNetworkConfig::NetworkId const& a, libnutwireless::CNetworkConfig::NetworkId const& b) {
		return ((a.id == b.id) && (a.pid == b.pid));
	}

	inline uint qHash(libnutwireless::CNetworkConfig::NetworkId const& key) {
		return qHash(QPair<qint64,qint32>(key.pid,key.id));
	}
}
#endif /* LIBNUTWIRELESS_CNETWORKCONFIG_H */
