#ifndef LIBNUT_LIBNUT_WPA_SUPPLICANT_TYPES_H
#define LIBNUT_LIBNUT_WPA_SUPPLICANT_TYPES_H

#include <QString>
#include <QList>
#include <QHostAddress>
#include <common/macaddress.h>
#include <iwlib.h>
extern "C" {
// #include <linux/wireless.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
}
namespace libnutws {

	typedef enum {NF_NONE, NF_CURRENT, NF_DISABLED} NetworkFlags;

	typedef enum {CI_UNDEFINED=0, CI_NONE=1, CI_CCMP=2, CI_TKIP=4, CI_WEP104=8, CI_WEP40=16, CI_WEP=24} ScanCiphers;
	typedef enum {AUTH_UNDEFINED=0, AUTH_PLAIN=1,AUTH_WPA_PSK=2,AUTH_WPA2_PSK=4, AUTH_WPA_EAP=8, AUTH_WPA2_EAP=16, AUTH_IEEE8021X=32, AUTH_DEFAULT=62} ScanAuthentication;
	//AUTH_WPA_PSK = AUTHALG_OPEN && WKI_WPA && KEYMGMT_WPA_PSK && (GCI_CCMP || GCI_TKIP) && (PCI_CCMP || PCI_TKIP)
	//AUTH_WPA2_PSK = AUTHALG_OPEN && WKI_RSN && KEYMGMT_WPA_PSK && (GCI_CCMP || GCI_TKIP) && (PCI_CCMP || PCI_TKIP)
	//AUTH_WPA_EAP = (AUTHALG_OPEN || AUTHALG_LEAP) && KEYMGMT_WPA_EAP && WKI_WPA && (GCI_CCMP || GCI_TKIP) && (PCI_CCMP || PCI_TKIP)
	//AUTH_WPA2_EAP = (AUTHALG_OPEN || AUTHALG_LEAP) && KEYMGMT_WPA_EAP && WKI_RSN && (GCI_CCMP || GCI_TKIP) && (PCI_CCMP || PCI_TKIP)
	//AUTH_IEEE8021X = (AUTHALG_OPEN || LEAP) && KEYMGMT_IEEE8021X && (WKI_RSN || WKI_WPA) // This is not very clear yet


	typedef enum {PROTO_UNDEFINED=0, PROTO_WPA=1, PROTO_RSN=2, PROTO_DEFAULT=3} wps_protocols; //RSN=WPA2
	typedef enum {GCI_UNDEFINED=0, GCI_CCMP=2, GCI_TKIP=4, GCI_WEP104=8, GCI_WEP40=16, GCI_DEF=30} wps_group_ciphers;
	typedef enum {PCI_UNDEFINED=0, PCI_NONE=1, PCI_CCMP=2, PCI_TKIP=4, PCI_DEF=6} wps_pairwise_ciphers;
	typedef enum {KM_UNDEFINED=0, KM_NONE=1, KM_WPA_PSK=2, KM_WPA_EAP=4, KM_IEEE8021X=8, KM_DEF=6} wps_key_management;
	typedef enum {AUTHALG_UNDEFINED=0, AUTHALG_OPEN=1, AUTHALG_SHARED=2, AUTHALG_LEAP=4} wps_auth_algs; //Default: automatic selection

	typedef enum {INTERACT_MSG, INTERACT_REQ,INTERACT_EVENT} wps_interact_type;
	typedef enum {REQ_FAIL, REQ_PASSWORD, REQ_IDENTITY, REQ_NEW_PASSWORD, REQ_PIN, REQ_OTP, REQ_PASSPHRASE} wps_req_type;
	typedef enum {EVENT_OTHER, EVENT_DISCONNECTED, EVENT_CONNECTED, EVENT_TERMINATING, EVENT_PASSWORD_CHANGED, EVENT_EAP_NOTIFICATION, EVENT_EAP_STARTED, EVENT_EAP_METHOD, EVENT_EAP_SUCCESS, EVENT_EAP_FAILURE } wps_event_type;

	typedef enum {EAPF_UNDEFINED=-1, EAPF_WIRED=0,EAPF_DYN_UNICAST_WEP=1, EAPF_BCAST_WEP=2,EAPF_DEFAULT=3} wps_eapol_flags;

	//Default: all build in
	typedef enum {EAPM_UNDEFINED=0, EAPM_MD5=1,EAPM_MSCHAPV2=2,EAPM_OTP=4,EAPM_GTC=8,EAPM_TLS=16,EAPM_PEAP=32,EAPM_TTLS=64,EAPM_ALL=127, EAPM_AKA=128, EAPM_FAST=256, EAPM_LEAP=512,EAPM_PSK=1024,EAPM_PAX=2048,EAPM_SAKE=4096,EAPM_GPSK=8192} wps_eap_method;
	//0001 = 1
	//0010 = 2
	//0100 = 4
	//0100 = 8

	typedef enum {
	NCF_NONE=0x0000000000, NCF_SSID=0x0000000001,NCF_BSSID=0x0000000002,NCF_DISABLED=0x0000000004,
	NCF_ID_STR=0x0000000008, NCF_SCAN_SSID=0x0000000010, NCF_PRIORITY=0x0000000020,
	NCF_MODE=0x0000000040, NCF_FREQ=0x0000000080, NCF_PROTO=0x00000000100, NCF_KEYMGMT=0x00000000200,
	NCF_AUTH_ALG=0x00000000400, NCF_PAIRWISE=0x00000000800, NCF_GROUP=0x0000001000,
	NCF_PSK=0x0000002000, NCF_EAPOL_FLAGS=0x0000004000, NCF_MIXED_CELL=0x0000008000,
	NCF_PROA_KEY_CACHING=0x00000010000, NCF_WEP_KEY0=0x0000020000, NCF_WEP_KEY1=0x0000040000,
	NCF_WEP_KEY2=0x0000080000, NCF_WEP_KEY3=0x0000100000, NCF_WEP_KEY_IDX=0x0000200000,
	NCF_PEERKEY=0x0000400000, NCF_ALL=0x00007FFFFF
	}wps_netconfig_failures;

	typedef enum {
	ENCF_NONE=0x0000000000, ENCF_EAP=0x0000000001,ENCF_IDENTITY=0x0000000002,ENCF_ANON_IDENTITY=0x0000000004, ENCF_PASSWD=0x0000000008,
	ENCF_CA_CERT=0x0000000010, ENCF_CA_PATH=0x0000000020, ENCF_CLIENT_CERT=0x0000000040, ENCF_PRIVATE_KEY=0x0000000080,
	ENCF_PRIVATE_KEY_PASSWD=0x0000000100, ENCF_DH_FILE=0x0000000200, ENCF_SUBJECT_MATCH=0x0000000400, ENCF_ALTSUBJECT_MATCH=0x0000000800,
	ENCF_PHASE1=0x0000001000, ENCF_PHASE2=0x0000002000, ENCF_CA_CERT2=0x0000004000, ENCF_CA_PATH2=0x0000008000,
	ENCF_CLIENT_CERT2=0x00000010000, ENCF_PRIVATE_KEY2=0x0000020000,
	ENCF_PRIVATE_KEY2_PASSWD=0x0000040000, ENCF_DH_FILE2=0x0000080000, ENCF_SUBJECT_MATCH2=0x0000100000,
	ENCF_ALTSUBJECT_MATCH2=0x0000200000, ENCF_FRAGMENT_SIZE=0x0000400000, ENCF_EAPPSK=0x0000800000,
	ENCF_NAI=0x00001000000, ENCF_PAC_FILE=0x00002000000, ENCF_ALL=0x00003FFFFFF
	} wps_eap_netconfig_failures;

	typedef enum {
		BOOL_UNDEFINED=-1, BOOL_FALSE=0,BOOL_TRUE=1
	} wps_bool;

	struct wps_netconfig_status {
		wps_netconfig_failures failures;
		wps_eap_netconfig_failures eap_failures;
		int id;
	};
	struct wps_capabilities {
		wps_eap_method eap;
		wps_pairwise_ciphers pairwise;
		wps_group_ciphers group;
		wps_key_management keyManagement;
		wps_protocols proto;
		wps_auth_algs auth_alg;
	};

	struct	wps_raw_signal_quality {
		quint8 qual;	/* link quality (%retries, SNR, %missed beacons or better...) */
		quint8 level;		/* signal level (dBm) */
		quint8 noise;		/* noise level (dBm) */
		quint8 updated;	/* Flags to know if updated */
	};

	struct wps_wext_raw_scan {
		nut::MacAddress bssid;
		wps_raw_signal_quality quality;
		wps_raw_signal_quality maxquality;
		wps_raw_signal_quality avgquality;
		int hasRange;
		int we_version_compiled;
	};

// 	typedef enum {
// 		WSIG_QUALITY_ALLABS=0, WSIG_LEVEL_REL=1, WSIG_NOISE_REL=2, WSIG_UNKNOWN=4
// 	} wps_signal_quality_encoding; 

// 	struct wps_wext_scan_readable : public wps_wext_scan {
// 		wps_signal_quality_encoding encoding;
// 	};
	
	typedef enum {
		WSR_UNKNOWN=0, WSR_RCPI=1, WSR_ABSOLUTE=2, WSR_RELATIVE=3
	} wps_wext_signal_readable_type; 

	struct wps_wext_signal_readable {
		wps_wext_signal_readable_type type;
		struct {
			quint8 value;
			quint8 maximum;
		} quality;
		union {
			qreal rcpi;
			struct {
				qint16 value;
				quint8 maximum;
			} nonrcpi;
		} noise;
		union {
			qreal rcpi;
			struct {
				qint16 value;
				quint8 maximum;
			} nonrcpi;
		} level;
	};

	struct wps_wext_scan_readable {
		nut::MacAddress bssid;
		wps_wext_signal_readable signal;
	};
	
	struct wps_scan {
		nut::MacAddress bssid;
		QString ssid;
		int freq;
		wps_wext_signal_readable signal;
		ScanCiphers ciphers;
		wps_key_management keyManagement;
		wps_protocols protocols;
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
		wps_pairwise_ciphers pairwise_cipher;
		wps_group_ciphers group_cipher;
		wps_key_management key_mgmt;
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


	//Conversion functions
	wps_group_ciphers toGroupCiphers(ScanCiphers cip);
	wps_pairwise_ciphers toPairwiseCiphers(ScanCiphers cip);
	wps_key_management toKeyManagment(ScanAuthentication auth);
	wps_auth_algs toAuthAlgs(ScanAuthentication auth);
	wps_protocols toProtocols(ScanAuthentication auth);


	QString toString(wps_group_ciphers cip);
	QString toString(wps_pairwise_ciphers cip);
	QString toString(wps_key_management keym);
	QString toString(wps_auth_algs algs);
	QString toString(wps_protocols proto);


	QString toString(ScanCiphers cip);
	QString toString(ScanAuthentication auth);
	QString toString(wps_req_type reqt);
	
	QString toString(wps_eapol_flags flags);
	QString toString(wps_eap_method method);

	//Function converts the encoded scan values to real values
	wps_wext_signal_readable convertValues(wps_wext_raw_scan scan);
	QString signalQualityToString(wps_wext_raw_scan scan);
	QStringList signalQualityToStringList(wps_wext_raw_scan scan);
	

	QString toNumberString(wps_bool b);
	bool toBool(wps_bool b);
	wps_bool toWpsBool(bool b);

	inline int toNumber(bool b) {
		return ((b)? 1 : 0);
	}
	inline int toNumber(wps_bool b) {
		return ( (b == BOOL_UNDEFINED) ? -1 : ( (b == BOOL_TRUE) ? 1 : 0)); 
	}
	inline bool toBool(QString str) {
		return ( ("0" == str) ? true : false);
	}
	inline wps_bool toWpsBool(QString str) {
		if ("0" == str) {
			return BOOL_FALSE;
		}
		else if ("1" == str) {
			return BOOL_TRUE;
		}
		return BOOL_UNDEFINED;
	}
	
	struct wps_network {
		int id;
		QString ssid;
		nut::MacAddress bssid;
		NetworkFlags flags;
	};

	class wps_eap_network_config {
		public:
			wps_eap_network_config();
			~wps_eap_network_config();
			//Following fields are only used with internal EAP implementation.
			wps_eap_method eap; //space-separated list of accepted EAP methods TODO: implement
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
	};

	class wps_network_config { //All without linebreak
		public:
			wps_network_config();
			~wps_network_config();
			QString ssid;
			nut::MacAddress bssid;
			wps_bool disabled;
			QString id_str; // Network identifier string for external scripts
			wps_bool scan_ssid; // (do not) scan with SSID-specific Probe Request frames
			int priority;
			wps_bool mode; //0 = infrastructure (Managed) mode, i.e., associate with an AP (default) 1 = IBSS (ad-hoc, peer-to-peer)
			int frequency;
			wps_protocols protocols; //list of accepted protocols TODO: implement
			wps_key_management keyManagement; // list of accepted authenticated key management protocols
			wps_auth_algs auth_alg; //list of allowed IEEE 802.11 authentication algorithms TODO:implement
			wps_pairwise_ciphers pairwise; //list of accepted pairwise (unicast) ciphers for WPA (CCMP,TKIP,NONE)
			wps_group_ciphers group; //list of accepted group (broadcast/multicast) ciphers for WPA (CCMP;TKIP;WEP104/40)
			QString psk; //WPA preshared key; 256-bit pre-shared key
			wps_eapol_flags eapol_flags; // IEEE 802.1X/EAPOL options (bit field) TODO:implement
			wps_bool mixed_cell; //This option can be used to configure whether so called mixed
			wps_bool proactive_key_caching; //Enable/disable opportunistic PMKSA caching for WPA2.
			QString wep_key0; //Static WEP key (ASCII in double quotation, hex without)
			QString wep_key1;
			QString wep_key2;
			QString wep_key3;
			char wep_tx_keyidx; //Default WEP key index (TX) (0..3) TODO: implement
			wps_bool peerkey; //Whether PeerKey negotiation for direct links (IEEE 802.11e DLS) is allowed.
			wps_eap_network_config eap_config;
	};
}

#endif
