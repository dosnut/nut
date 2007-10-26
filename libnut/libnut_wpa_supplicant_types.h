#ifndef LIBNUT_LIBNUT_WPA_SUPPLICANT_TYPES_H
#define LIBNUT_LIBNUT_WPA_SUPPLICANT_TYPES_H

#include <QString>
#include <QByteArray>
#include <QList>
#include <common/macaddress.h>
#include <QHostAddress>

namespace libnut {

	typedef enum {WNF_NONE=0, WNF_CURRENT=1} wps_network_flags;
	typedef enum {CI_UNDEFINED=0, CI_NONE=1, CI_CCMP=2, CI_TKIP=4, CI_WEP104=8, CI_WEP40=16, CI_WEP=32} CIPHERS;
	typedef enum {KEYMGMT_PLAIN=0, KEYMGMT_WPA_PSK=2, KEYMGMT_WPA2_PSK=4, KEYMGMT_WPA_EAP=8, KEYMGMT_WPA2_EAP=16, KEYMGMT_IEEE8021X=32, KEYMGMT_UNDEFINED=64} KEYMGMT;
	typedef enum {WKI_UNDEFINED=0, WKI_WPA=1, WKI_RSN=2} wps_protocols;
	typedef enum {WI_MSG, WI_REQ,WI_EVENT} wps_interact_type;
	typedef enum {WR_FAIL, WR_PASSWORD, WR_IDENTITY, WR_NEW_PASSWORD, WR_PIN, WR_OTP, WR_PASSPHRASE} wps_req_type;
	typedef enum {WE_OTHER, WE_DISCONNECTED, WE_CONNECTED, WE_TERMINATING, WE_PASSWORD_CHANGED, WE_EAP_NOTIFICATION, WE_EAP_STARTED, WE_EAP_METHOD, WE_EAP_SUCCESS, WE_EAP_FAILURE } wps_event_type;
	typedef enum {WAA_UNDEFINED=0, WAA_OPEN=1, WAA_SHARED=2, WAA_LEAP=4} wps_auth_algs;
	typedef enum {EAP_NONE=0,EAP_DYN_UNICAST_WEP=1, EAP_BCAST_WEP=2} EAPOL_FLAGS;
	typedef enum {EAP_ALL=127, EAPM_MD5=1,EAPM_MSCHAPV2=2,EAPM_OTP=4,EAPM_GTC=8,EAPM_TLS=16,EAPM_PEAP=32,EAPM_TTLS=64} EAP_METHOD;

	struct wps_scan {
		nut::MacAddress bssid;
		QString ssid;
		int freq;
		int level;
		CIPHERS ciphers;
		KEYMGMT key_mgmt;
	};
	struct wps_variable;
	typedef QList<wps_variable> wps_MIB;
	//enums are NOT complete, but maybe we schould change this to QString
	struct wps_status {
// 		typedef enum {COMPLETED} WPA_STATE;
// 		typedef enum {AUTHENTICATED} PAE_STATE;
// 		typedef enum {AUTHORIZED} PORT_STATUS;
// 		typedef enum {AUTO} PORT_CONTROL;
// 		typedef enum {IDLE} BACKEND_STATE;
// 		typedef enum {SUCCESS} EAP_STATE;
// 		typedef enum {NOSTATE} METHOD_STATE;
// 		typedef enum {COND_SUCC} DECISION;
		//These typedefs may change in the future to the ones above (more complete)
		typedef QString WPA_STATE;
		typedef QString PAE_STATE;
		typedef QString PORT_STATUS;
		typedef QString PORT_CONTROL;
		typedef QString BACKEND_STATE;
		typedef QString EAP_STATE;
		typedef QString METHOD_STATE;
		typedef QString DECISION;
		nut::MacAddress bssid;
		QString ssid;
		int id;
		CIPHERS pairwise_cipher;
		CIPHERS group_cipher;
		KEYMGMT key_mgmt;
		WPA_STATE wpa_state;
		QHostAddress ip_address;
		PAE_STATE pae_state;
		PORT_STATUS PortStatus;
		int heldPeriod;
		int authPeriod;
		int startPeriod;
		int maxStart;
		PORT_CONTROL portControl;
		BACKEND_STATE backend_state;
		EAP_STATE eap_state;
		int reqMethod;
		METHOD_STATE methodState;
		DECISION decision;
		int ClientTimeout;
	};

	struct wps_variable {
		typedef enum {PLAIN=1,STRING=2,NUMBER=4,LOGIC=8} wps_variable_type;
		wps_variable_type type;
		QString name;
		union {
			qint32 * num;
			QString * str;
			bool * logic;
		} value;
	};
	struct wps_net_var {
		typedef enum {PLAIN=1,STRING=2,NUMBER=4,LOGIC=8} Type;
		QString name;
		union {
			int * num;
			QString * str;
			bool * logic;
		} value;
	};
	struct wps_req {
		wps_req_type type;
		int id;
	};

	QString toString(wps_protocols proto);
	QString toString(CIPHERS cip);
	QString toString(KEYMGMT keym);
	QString toString(wps_req_type reqt);
	QString toString(wps_auth_algs algs);
	QString toString(EAPOL_FLAGS flags);
	QString toString(EAP_METHOD method);
	
	struct wps_network {
		int id;
		QString ssid;
		nut::MacAddress bssid;
		wps_network_flags flags;
	};
	class wps_network_config { //All without linebreak
		public:
			wps_network_config();
			~wps_network_config();
			QString ssid;
			nut::MacAddress bssid;
			bool disabled;
			QString id_str; // Network identifier string for external scripts
			bool scan_ssid; // (do not) scan with SSID-specific Probe Request frames
			int priority;
			bool mode; //0 = infrastructure (Managed) mode, i.e., associate with an AP (default) 1 = IBSS (ad-hoc, peer-to-peer)
			int frequency;
			wps_protocols proto; //list of accepted protocols TODO: implement
			KEYMGMT key_mgmt; // list of accepted authenticated key management protocols
			wps_auth_algs auth_alg; //list of allowed IEEE 802.11 authentication algorithms TODO:implement
			CIPHERS pairwise; //list of accepted pairwise (unicast) ciphers for WPA (CCMP,TKIP,NONE)
			CIPHERS group; //list of accepted group (broadcast/multicast) ciphers for WPA (CCMP;TKIP;WEP104/40)
			QString psk; //WPA preshared key; 256-bit pre-shared key
			EAPOL_FLAGS eapol_flags; // IEEE 802.1X/EAPOL options (bit field) TODO:implement
			bool mixed_cell; //This option can be used to configure whether so called mixed
			bool proactive_key_caching; //Enable/disable opportunistic PMKSA caching for WPA2.
			QString wep_key0; //Static WEP key (ASCII in double quotation, hex without)
			QString wep_key1;
			QString wep_key2;
			QString wep_key3;
			uchar wep_tx_keyidx; //Default WEP key index (TX) (0..3) TODO: implement
			bool peerkey; //Whether PeerKey negotiation for direct links (IEEE 802.11e DLS) is allowed.
	};
	class wps_eap_network_config: wps_network_config {
		public:
			//Following fields are only used with internal EAP implementation.
			EAP_METHOD eap; //space-separated list of accepted EAP methods TODO: implement
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
			int nai; //user NAI
			int pac_file; //File path for the PAC entries.
	};


}
#endif
