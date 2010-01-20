#ifndef LIBNUTWIRELESS_CNETWORKCONFIG_H
#define LIBNUTWIRELESS_CNETWORKCONFIG_H

#include "hwtypes.h"

namespace libnutwireless {

	/**
		The network config class contains all information for configuring a network.
		On instantiation all values will be set to undefined.
	*/
	class CNetworkConfig { //All without linebreak
		private:
			QString ssid;
			libnutcommon::MacAddress bssid;
			QOOL disabled;
			QString id_str; // Network identifier string for external scripts
			QOOL scan_ssid; // (do not) scan with SSID-specific Probe Request frames
			int priority;
			QOOL mode; //0 = infrastructure (Managed) mode, i.e., associate with an AP (default) 1 = IBSS (ad-hoc, peer-to-peer)
			int frequency;
			Protocols protocols; //list of accepted protocols 
			KeyManagement key_mgmt; // list of accepted authenticated key management protocols
			AuthenticationAlgs auth_alg; //list of allowed IEEE 802.11 authentication algorithms
			PairwiseCiphers pairwise; //list of accepted pairwise (unicast) ciphers for WPA (CCMP,TKIP,NONE)
			GroupCiphers group; //list of accepted group (broadcast/multicast) ciphers for WPA (CCMP;TKIP;WEP104/40)
			QString psk; //WPA preshared key; 256-bit pre-shared key
			EapolFlags eapol_flags; // IEEE 802.1X/EAPOL options (bit field)
			QOOL mixed_cell; //This option can be used to configure whether so called mixed
			QOOL proactive_key_caching; //Enable/disable opportunistic PMKSA caching for WPA2.
			QString wep_key0; //Static WEP key (ASCII in double quotation, hex without)
			QString wep_key1;
			QString wep_key2;
			QString wep_key3;
			char wep_tx_keyidx; //Default WEP key index (TX) (0..3)
			QOOL peerkey; //Whether PeerKey negotiation for direct links (IEEE 802.11e DLS) is allowed.

			//eap config part:
			EapMethod eap; //space-separated list of accepted EAP methods
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
			int fragment_size; //Maximum EAP fragment size in bytes (default 1398);
			QString eappsk; //16-byte (128-bit, 32 hex digits) pre-shared key in hex format
			QString nai; //user NAI
			QString pac_file; //File path for the PAC entries.

		public:
			CNetworkConfig();
			CNetworkConfig(ScanResult scan);
			~CNetworkConfig();
			void readFrom(QDataStream &stream);
			void writeTo(QDataStream &stream);

			//Access functions
			inline QString get_ssid() { return ssid;}
			inline libnutcommon::MacAddress get_bssid() { return bssid; }
			inline QOOL get_disabled() { return disabled; }
			inline QString get_id_str() { return id_str; }
			inline QOOL get_scan_ssid() { return scan_ssid; }
			inline int get_priority() { return priority; }
			inline QOOL get_mode() { return mode; }
			inline int get_frequency() { return frequency; }
			inline Protocols get_protocols() { return protocols; }
			inline KeyManagement get_key_mgmt() { return key_mgmt; }
			inline AuthenticationAlgs get_auth_alg() { return auth_alg; }
			inline PairwiseCiphers get_pairwise() { return pairwise; }
			inline GroupCiphers get_group() { return group; } 
			inline QString get_psk() { return psk; } 
			inline EapolFlags get_eapol_flags() { return eapol_flags; } 
			inline QOOL get_mixed_cell() { return mixed_cell; } 
			inline QOOL get_proactive_key_caching() { return proactive_key_caching; } 
			inline QString get_wep_key0() { return wep_key0; } 
			inline QString get_wep_key1() { return wep_key1; }
			inline QString get_wep_key2() { return wep_key2; }
			inline QString get_wep_key3() { return wep_key3; }
			inline char get_wep_tx_keyidx() { return wep_tx_keyidx; } 
			inline QOOL get_peerkey() { return peerkey; }
			inline EapMethod get_eap() { return eap; } 
			inline QString get_identity() { return identity; } 
			inline QString get_anonymous_identity() { return anonymous_identity; } 
			inline QString get_password() { return password; } 
			inline QString get_ca_cert() { return ca_cert; } 
			inline QString get_ca_path() { return ca_path; } 
			inline QString get_client_cert() { return client_cert; } 
			inline QString get_private_key() { return private_key; } 
			inline QString get_private_key_passwd() { return private_key_passwd; } 
			inline QString get_dh_file() { return dh_file; } 
			inline QString get_subject_match() { return subject_match; } 
			inline QString get_altsubject_match() { return altsubject_match; } 
			inline QString get_phase1() { return phase1; } 
			inline QString get_phase2() { return phase2; } 
			inline QString get_ca_cert2() { return ca_cert2; } 
			inline QString get_ca_path2() { return ca_path2; } 
			inline QString get_client_cert2() { return client_cert2; } 
			inline QString get_private_key2() { return private_key2; } 
			inline QString get_private_key2_passwd() { return private_key2_passwd; } 
			inline QString get_dh_file2() { return dh_file2; } 
			inline QString get_subject_match2() { return subject_match2; } 
			inline QString get_altsubject_match2() { return altsubject_match2; } 
			inline int get_fragment_size() { return fragment_size; } 
			inline QString get_eappsk() { return eappsk; } 
			inline QString get_nai() { return nai; } 
			inline QString get_pac_file() { return pac_file; } 
			
			//Set functions:
			inline void set_proto(Protocols proto) { protocols = proto; };
			inline void set_key_mgmt(KeyManagement k) { key_mgmt = k; }
			inline void set_auth_alg(AuthenticationAlgs algs) { auth_alg = algs; };
			inline void set_pairwise(PairwiseCiphers p) { pairwise = p; };
			inline void set_group(GroupCiphers g) { group = g; };
			inline void set_eap(EapMethod e) { eap = e; }
			
			//Set Parse functions:
			bool set_ssid(QString str);
			bool set_bssid(libnutcommon::MacAddress addrr);
			bool set_disabled(bool disabled);
			bool set_id_str(QString str);
			bool set_scan_ssid(bool enabled);
			bool set_priority(int priority);
			bool set_mode(bool mode);
			bool set_frequency(int freq);
			bool set_proto(QString proto);
			bool set_key_mgmt(QString key_mgmt);
			bool set_auth_alg(QString auth_alg);
			bool set_pairwise(QString pairwise);
			bool set_group(QString group);
			bool set_psk(QString psk);
			bool set_eapol_flags(QString eapol_flags);
			bool set_mixed_cell(bool enabled);
			bool set_proactive_key_caching(bool enabled);
			bool set_wep_key0(QString key);
			bool set_wep_key1(QString key);
			bool set_wep_key2(QString key);
			bool set_wep_key3(QString key);
			bool set_wep_tx_keyidx(int idx);
			bool set_peerkey(bool enabled);
			bool set_eap(QString str);
			bool set_identity(QString str);
			bool set_anonymous_identity(QString str);
			bool set_password(QString str);
			bool set_ca_cert(QString str);
			bool set_ca_path(QString str);
			bool set_client_cert(QString str);
			bool set_private_key(QString str);
			bool set_private_key_passwd(QString str);
			bool set_dh_file(QString str); 
			bool set_subject_match(QString str); 
			bool set_altsubject_match(QString str); 
			bool set_phase1(QString str); 
			bool set_phase2(QString str); 
			bool set_ca_cert2(QString str); 
			bool set_ca_path2(QString str); 
			bool set_client_cert2(QString str); 
			bool set_private_key2(QString str); 
			bool set_private_key2_passwd(QString str);
			bool set_dh_file2(QString str); 
			bool set_subject_match2(QString str); 
			bool set_altsubject_match2(QString str); 
			bool set_fragment_size(int size);
			bool set_eappsk(QString str); 
			bool set_nai(QString str); 
			bool set_pac_file(QString str); 
	};
}
#endif