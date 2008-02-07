#ifndef LIBNUTWIRELESS_TYPES_H
#define LIBNUTWIRELESS_TYPES_H

#include <QString>
#include <QList>
#include <QHostAddress>
#include <libnutcommon/macaddress.h>
#include <iwlib.h>
extern "C" {
#include <linux/wireless.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
}
namespace libnutwireless {

	/** Enum of possible NetworkFlags */
	typedef enum {NF_NONE, NF_CURRENT, NF_DISABLED} NetworkFlags;
	
	
	typedef enum {CI_UNDEFINED=0, CI_NONE=1, CI_CCMP=2, CI_TKIP=4, CI_WEP104=8, CI_WEP40=16, CI_WEP=24} ScanCiphers;
	typedef enum {AUTH_UNDEFINED=0, AUTH_PLAIN=1,AUTH_WPA_PSK=2,AUTH_WPA2_PSK=4, AUTH_WPA_EAP=8, AUTH_WPA2_EAP=16, AUTH_IEEE8021X=32, AUTH_WPA_NONE=64, AUTH_WPA2_NONE=128, AUTH_DEFAULT=62} ScanAuthentication;
	//AUTH_WPA_PSK = AUTHALG_OPEN && WKI_WPA && KEYMGMT_WPA_PSK && (GCI_CCMP || GCI_TKIP) && (PCI_CCMP || PCI_TKIP)
	//AUTH_WPA2_PSK = AUTHALG_OPEN && WKI_RSN && KEYMGMT_WPA_PSK && (GCI_CCMP || GCI_TKIP) && (PCI_CCMP || PCI_TKIP)
	//AUTH_WPA_EAP = (AUTHALG_OPEN || AUTHALG_LEAP) && KEYMGMT_WPA_EAP && WKI_WPA && (GCI_CCMP || GCI_TKIP) && (PCI_CCMP || PCI_TKIP)
	//AUTH_WPA2_EAP = (AUTHALG_OPEN || AUTHALG_LEAP) && KEYMGMT_WPA_EAP && WKI_RSN && (GCI_CCMP || GCI_TKIP) && (PCI_CCMP || PCI_TKIP)
	//AUTH_IEEE8021X = (AUTHALG_OPEN || LEAP) && KEYMGMT_IEEE8021X && (WKI_RSN || WKI_WPA) // This is not very clear yet


	/** Enum of possible protocols. RSN=WPA2 */
	typedef enum {PROTO_UNDEFINED=0, PROTO_WPA=1, PROTO_RSN=2, PROTO_DEFAULT=3} Protocols; //RSN=WPA2
	/** Enum of possible group ciphers. */
	typedef enum {GCI_UNDEFINED=0,  GCI_NONE=2, GCI_WEP40=4, GCI_WEP104=8, GCI_TKIP=16, GCI_CCMP=32, GCI_WRAP=64, GCI_DEF=60} GroupCiphers;
	/** Enum of possible pairwise ciphers */
	typedef enum {PCI_UNDEFINED=0, PCI_NONE=1, PCI_TKIP=2, PCI_CCMP=4, PCI_DEF=6} PairwiseCiphers;
	/** Enum of possible  key management */
	typedef enum {KM_UNDEFINED=0, KM_OFF=2, KM_NONE=4, KM_WPA_NONE=8, KM_WPA_PSK=16, KM_WPA_EAP=32, KM_IEEE8021X=64, KM_DEF=48} KeyManagement; //TODO:change parsers due to KM_OFF
	/** Enum of possible authentication  algorithms */
	typedef enum {AUTHALG_UNDEFINED=0, AUTHALG_OPEN=1, AUTHALG_SHARED=2, AUTHALG_LEAP=4} AuthenticationAlgs; //Default: automatic selection

	
	typedef enum {INTERACT_MSG, INTERACT_REQ,INTERACT_EVENT} InteractiveType;
	/** RequestType contains all possible requests from wpa_supplicant. */
	typedef enum {REQ_FAIL, REQ_PASSWORD, REQ_IDENTITY, REQ_NEW_PASSWORD, REQ_PIN, REQ_OTP, REQ_PASSPHRASE} RequestType;
	/** Possible events from wpa_supplicant, not complete */
	typedef enum {EVENT_OTHER, EVENT_DISCONNECTED, EVENT_CONNECTED, EVENT_TERMINATING, EVENT_PASSWORD_CHANGED, EVENT_EAP_NOTIFICATION, EVENT_EAP_STARTED, EVENT_EAP_METHOD, EVENT_EAP_SUCCESS, EVENT_EAP_FAILURE } EventType;
	/** Eapol flags */
	typedef enum {EAPF_UNDEFINED=-1, EAPF_WIRED=0,EAPF_DYN_UNICAST_WEP=1, EAPF_BCAST_WEP=2,EAPF_DEFAULT=3} EapolFlags;

	/** List of Eap methods for wpa_supplicant */
	typedef enum {EAPM_UNDEFINED=0, EAPM_MD5=1,EAPM_MSCHAPV2=2,EAPM_OTP=4,EAPM_GTC=8,EAPM_TLS=16,EAPM_PEAP=32,EAPM_TTLS=64,EAPM_ALL=127, EAPM_AKA=128, EAPM_FAST=256, EAPM_LEAP=512,EAPM_PSK=1024,EAPM_PAX=2048,EAPM_SAKE=4096,EAPM_GPSK=8192} EapMethod;
	//0001 = 1
	//0010 = 2
	//0100 = 4
	//0100 = 8

	/** Enum of possible config failures.  */
	typedef enum {
	NCF_NONE=0x0000000000, NCF_SSID=0x0000000001,NCF_BSSID=0x0000000002,NCF_DISABLED=0x0000000004,
	NCF_ID_STR=0x0000000008, NCF_SCAN_SSID=0x0000000010, NCF_PRIORITY=0x0000000020,
	NCF_MODE=0x0000000040, NCF_FREQ=0x0000000080, NCF_PROTO=0x00000000100, NCF_KEYMGMT=0x00000000200,
	NCF_AUTH_ALG=0x00000000400, NCF_PAIRWISE=0x00000000800, NCF_GROUP=0x0000001000,
	NCF_PSK=0x0000002000, NCF_EAPOL_FLAGS=0x0000004000, NCF_MIXED_CELL=0x0000008000,
	NCF_PROA_KEY_CACHING=0x00000010000, NCF_WEP_KEY0=0x0000020000, NCF_WEP_KEY1=0x0000040000,
	NCF_WEP_KEY2=0x0000080000, NCF_WEP_KEY3=0x0000100000, NCF_WEP_KEY_IDX=0x0000200000,
	NCF_PEERKEY=0x0000400000, NCF_ALL=0x00007FFFFF
	}NetconfigFailures;

	/** Enum of possible eap config failures */
	typedef enum {
	ENCF_NONE=0x0000000000, ENCF_EAP=0x0000000001,ENCF_IDENTITY=0x0000000002,ENCF_ANON_IDENTITY=0x0000000004, ENCF_PASSWD=0x0000000008,
	ENCF_CA_CERT=0x0000000010, ENCF_CA_PATH=0x0000000020, ENCF_CLIENT_CERT=0x0000000040, ENCF_PRIVATE_KEY=0x0000000080,
	ENCF_PRIVATE_KEY_PASSWD=0x0000000100, ENCF_DH_FILE=0x0000000200, ENCF_SUBJECT_MATCH=0x0000000400, ENCF_ALTSUBJECT_MATCH=0x0000000800,
	ENCF_PHASE1=0x0000001000, ENCF_PHASE2=0x0000002000, ENCF_CA_CERT2=0x0000004000, ENCF_CA_PATH2=0x0000008000,
	ENCF_CLIENT_CERT2=0x00000010000, ENCF_PRIVATE_KEY2=0x0000020000,
	ENCF_PRIVATE_KEY2_PASSWD=0x0000040000, ENCF_DH_FILE2=0x0000080000, ENCF_SUBJECT_MATCH2=0x0000100000,
	ENCF_ALTSUBJECT_MATCH2=0x0000200000, ENCF_FRAGMENT_SIZE=0x0000400000, ENCF_EAPPSK=0x0000800000,
	ENCF_NAI=0x00001000000, ENCF_PAC_FILE=0x00002000000, ENCF_ALL=0x00003FFFFFF
	} EapNetconfigFailures;

	/** QOOL is a tri-state. Its name is derived from Bool and qubit. */
	typedef enum {
		QOOL_UNDEFINED=-1, QOOL_FALSE=0,QOOL_TRUE=1
	} QOOL; // Like a qubit :)

	/** Enum of operation modes of wireless networks (see WirelessExtension) */
	typedef enum {
		OPM_AUTO=0, OPM_ADHOC=1, OPM_MANAGED=2,OPM_MASTER=3,OPM_REPEATER=4,OPM_SECONDARY=5,OPM_MONITOR=6,OPM_UNKNOWN_BUG=7
	} OPMODE;

	/**
		NetconfigStatus contains error information when configuring a network
		@param failures Standard network failures
		@param eap_failures Eap network failures
		@param id on failures, id is -1, else it's the network id which was configured
	*/
	struct NetconfigStatus {
		NetconfigFailures failures;
		EapNetconfigFailures eap_failures;
		int id;
	};
	/** wpa_supplicant's capabilities */
	struct Capabilities {
		EapMethod eap;
		PairwiseCiphers pairwise;
		GroupCiphers group;
		KeyManagement keyManagement;
		Protocols proto;
		AuthenticationAlgs auth_alg;
	};

	/** Raw signal is contains the data from the kernel. 
		For human readable format, it has to be converted to WextSignal */
	struct	WextRawSignal {
		quint8 qual;	/* link quality (%retries, SNR, %missed beacons or better...) */
		quint8 level;		/* signal level (dBm) */
		quint8 noise;		/* noise level (dBm) */
		quint8 updated;	/* Flags to know if updated */
	};

	/**
		WextRawScan contains the data from the kernel.
		It's converted to WextScan.
	*/
	struct WextRawScan {
		QString ssid;
		libnutcommon::MacAddress bssid;
		WextRawSignal quality;
		WextRawSignal maxquality;
		WextRawSignal avgquality;
		int hasRange;
		int we_version_compiled;
		int freq;
		GroupCiphers group;
		PairwiseCiphers pairwise;
		KeyManagement keyManagement;
		Protocols protocols;
		OPMODE opmode;
		QList<qint32> bitrates;
		//Further information pending...
	};

	/** enum of possible signal encodig */
	typedef enum {
		WSR_UNKNOWN=0, WSR_RCPI=1, WSR_ABSOLUTE=2, WSR_RELATIVE=3
	} WextSignalType; 
	
	/** signal information in human readable format */
	struct WextSignal {
		int frequency;
		WextSignalType type;
		QList<qint32> bitrates; //Current bitrate
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

	/** Scan in human readable format (internal use) */
	struct WextScan {
		QString ssid;
		libnutcommon::MacAddress bssid;
		WextSignal signal;
		int hasRange;
		int we_version_compiled;
		int freq;
		GroupCiphers group;
		PairwiseCiphers pairwise;
		KeyManagement keyManagement;
		Protocols protocols;
		OPMODE opmode;
		QList<qint32> bitrates;
	};


	
	/** One scan result (network) in human readable format */
	struct ScanResult {
		libnutcommon::MacAddress bssid;
		QString ssid;
		int freq;
		WextSignal signal;
		GroupCiphers group;
		PairwiseCiphers pairwise;
		KeyManagement keyManagement;
		Protocols protocols;
		OPMODE opmode;
		QList<qint32> bitrates;
	};
	//Comparison functions
	/** Compare ScanResult by bssid */
	inline bool lessThanBSSID(libnutwireless::ScanResult a, libnutwireless::ScanResult b) {
		return (a.bssid < b.bssid);
	}
	/** Compare ScanResult by ssid */
	inline bool lessThanSSID(libnutwireless::ScanResult a, libnutwireless::ScanResult b) {
		return (a.ssid < b.ssid);
	}
	/** Compare ScanResult by frequency */
	inline bool lessThanFreq(libnutwireless::ScanResult a, libnutwireless::ScanResult b) {
		return (a.freq < b.freq);
	}
	/** Compare ScanResult by signal quality */
	inline bool lessThanSignalQuality(libnutwireless::ScanResult a, libnutwireless::ScanResult b) {
		return (a.signal.quality.value < b.signal.quality.value);
	}
	/** Compare ScanResult by signal level */
	inline bool lessThanSignalLevel(libnutwireless::ScanResult a, libnutwireless::ScanResult b) {
		return (a.signal.level.rcpi < b.signal.level.rcpi);
	}
	/** Compare ScanResult by signal noise */
	inline bool lessThanSignalNoise(libnutwireless::ScanResult a, libnutwireless::ScanResult b) {
		return (a.signal.noise.rcpi < b.signal.noise.rcpi);
	}

	/** Compare ScanResult by keymanagement rotocol (sort order in enum) */
	inline bool lessThanKeyManagement(libnutwireless::ScanResult a, libnutwireless::ScanResult b) {
		return ((int) a.keyManagement < (int) b.keyManagement);
	}
	/** Compare ScanResult by group (sort order in enum)  */
	inline bool lessThanGroup(libnutwireless::ScanResult a, libnutwireless::ScanResult b) {
		return ((int) a.group < (int) b.group);
	}
	/** Compare ScanResult by pairwise (sort order in enum) */
	inline bool lessThanPairwise(libnutwireless::ScanResult a, libnutwireless::ScanResult b) {
		return ((int) a.pairwise < (int) b.pairwise);
	}
	/** Compare ScanResult by protocol (sort order in enum) */
	inline bool lessThanProtocols(libnutwireless::ScanResult a, libnutwireless::ScanResult b) {
		return ((int) a.protocols < (int) b.protocols);
	}
	/** Compare ScanResult by operation mode. (Adhoc is bigger) */
	inline bool lessThanOpmode(libnutwireless::ScanResult a, libnutwireless::ScanResult b) {
		return (a.opmode == false && b.opmode == true);
	}
	/** Compare ScanResult by highest available bitrate */
	bool lessThanBitrates(libnutwireless::ScanResult a, libnutwireless::ScanResult b) {
		//Find the maximum of the a and b bitrates:
		qint32 maxa = 0;
		//Find maximum of a;
		for(QList<qint32>::iterator i = a.bitrates.begin(); i != a.bitrates.end(); ++i) {
			if (*i > maxa) {
				maxa = *i;
			}
		}
		//Check if b has higher bitrate
		for(QList<qint32>::iterator i = b.bitrates.begin(); i != b.bitrates.end(); ++i) {
			if (*i > maxa) {
				return true;
			}
		}
		//b has no bitrate that is higher
		return false;
	}


	struct MIBVariable;
	typedef QList<MIBVariable> MIBVariables;
	//enums are NOT complete, but maybe we schould change this to QString

	/** Status of wpa_supplicant */
	struct Status {
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
		libnutcommon::MacAddress bssid;
		QString ssid;
		int id;
		PairwiseCiphers pairwise_cipher;
		GroupCiphers group_cipher;
		KeyManagement key_mgmt;
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

	/** one wpa_Supplicant MIB variable */
	struct MIBVariable {
		typedef enum {PLAIN=1,STRING=2,NUMBER=4,LOGIC=8} MIBVariable_type;
		MIBVariable_type type;
		QString name;
		union {
			qint32 * num;
			QString * str;
			bool * logic;
		} value;
	};
	
	/** One wpa_suplicant network variable. Reserved for future purpose. */
	struct NetworkVariable {
		typedef enum {PLAIN=1,STRING=2,NUMBER=4,LOGIC=8} Type;
		QString name;
		union {
			int * num;
			QString * str;
			bool * logic;
		} value;
	};

	/** Request made by wpa_supplicant */
	struct Request {
		RequestType type;
		int id;
	};


	//Conversion functions
	/** Convert ScanCiphers to GroupCiphers */
	GroupCiphers toGroupCiphers(ScanCiphers cip);
	/// Convert ScanCiphers to PairwiseCiphers
	PairwiseCiphers toPairwiseCiphers(ScanCiphers cip);
	/// Convert ScanAuthentication to KeyManagement
	KeyManagement toKeyManagment(ScanAuthentication auth);
	///Convert ScanAuthentication to AuthenticationAlgs
	AuthenticationAlgs toAuthAlgs(ScanAuthentication auth);
	///Convert ScanAuthentication to Protocols
	Protocols toProtocols(ScanAuthentication auth);

	///Convert GroupCiphers to QString
	QString toString(GroupCiphers cip);
	///Convert PairwiseCiphers to QString
	QString toString(PairwiseCiphers cip);
	///Convert KeyManagement to QString
	QString toString(KeyManagement keym);
	///Convert AuthenticationAlgs to QString
	QString toString(AuthenticationAlgs algs);
	///Convert Protocols to QString
	QString toString(Protocols proto);

	///Convert ScanCiphers to QString
	QString toString(ScanCiphers cip);
	///Convert ScanAuthentication to QString
	QString toString(ScanAuthentication auth);
	///Convert RequestType to QString
	QString toString(RequestType reqt);
	
	///Convert EapolFlags to QString
	QString toString(EapolFlags flags);
	///Convert EapMethod to QString
	QString toString(EapMethod method);

	/// Function converts the encoded scan values to real values
	WextSignal convertValues(WextRawScan &scan);
	/// Function to convert the signal quality of a WextRawScan to a human readable string
	QString signalQualityToString(WextRawScan scan);
	
	/** Convert frequency to channel: 2,4GHz => 1-14; 5GHz => 36-167
		@return -1 on invalid values */
	int frequencyToChannel(int freq);
	/** Convert channel to frequency: 1-14 => 2,4GHz; 36-167 => 5GHz
		@return -1 on invalid values */
	int channelToFrequency(int channel);

	///Convert QOOL to QString (QOOL_UNDEFINED="-1", QOOL_FALSE="0", QOOL_TRUE="1")
	QString toNumberString(QOOL b);
	///Convert QOOL to bool (QOOL_UNDEFINED=false, QOOL_FALSE=false, QOOL_TRUE=true)
	bool toBool(QOOL b);
	///Convert bool to QOOL (false=QOOL_FALSE, true=QOOL_TRUE)
	QOOL toQOOL(bool b);

	/// Convert bool to number (false=0; true=1)
	inline int toNumber(bool b) {
		return ((b)? 1 : 0);
	}
	/// Convert QOOL to number (QOOL_UNDEFINED=-1, QOOL_FALSE=0, QOOL_TRUE=1)
	inline int toNumber(QOOL b) {
		return ( (b == QOOL_UNDEFINED) ? -1 : ( (b == QOOL_TRUE) ? 1 : 0)); 
	}
	/// Convert QString to bool ("1" = true; otherwise false)
	inline bool toBool(QString str) {
		return ( ("1" == str) ? true : false);
	}
	/// Convert QString to QOOL ("-1"=QOOL_UNDEFINED, "0"=QOOL_FALSE, "1"=QOOL_TRUE)
	inline QOOL toQOOL(QString str) {
		if ("0" == str) {
			return QOOL_FALSE;
		}
		else if ("1" == str) {
			return QOOL_TRUE;
		}
		return QOOL_UNDEFINED;
	}

	/** Information about a configured network (see listNetworks) */
	struct ShortNetworkInfo {
		int id;
		QString ssid;
		libnutcommon::MacAddress bssid;
		NetworkFlags flags;
		bool adhoc;
	};
	//Comparison functions
	/** Compare ShortNetworkinfo by Id*/
	inline bool lessThanId(ShortNetworkInfo a, ShortNetworkInfo b) {
		return (a.id < b.id);
	}
	/** Compare ShortNetworkinfo by Ssid*/
	inline bool lessThanSSID(ShortNetworkInfo a, ShortNetworkInfo b) {
		return (a.ssid < b.ssid);
	}
	/** Compare ShortNetworkinfo by Bssid*/
	inline bool lessThanBSSID(ShortNetworkInfo a, ShortNetworkInfo b) {
		return (a.bssid < b.bssid);
	}
	/** Compare ShortNetworkinfo by NetworkFlags*/
	inline bool lessThanFlags(ShortNetworkInfo a, ShortNetworkInfo b) {
		return ((int) a.flags < (int) b.flags);
	}
	/** Compare ShortNetworkinfo by Adhoc (adhoc is bigger)*/
	inline bool lessThanAdhoc(ShortNetworkInfo a, ShortNetworkInfo b) {
		return (a == false && b == true);
	}

	/**
		The eap network config class contains all information for configuring
		the eap part of a network. On instantiation all values will be set to undefined.
	*/
	class EapNetworkConfig {
		public:
			EapNetworkConfig();
			~EapNetworkConfig();
			//Following fields are only used with internal EAP implementation.
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
	};

	/**
		The network config class contains all information for configuring a network.
		On instantiation all values will be set to undefined.
	*/
	class NetworkConfig { //All without linebreak
		public:
			NetworkConfig();
			NetworkConfig(ScanResult scan);
			~NetworkConfig();
			QString ssid;
			libnutcommon::MacAddress bssid;
			QOOL disabled;
			QString id_str; // Network identifier string for external scripts
			QOOL scan_ssid; // (do not) scan with SSID-specific Probe Request frames
			int priority;
			QOOL mode; //0 = infrastructure (Managed) mode, i.e., associate with an AP (default) 1 = IBSS (ad-hoc, peer-to-peer)
			int frequency;
			Protocols protocols; //list of accepted protocols 
			KeyManagement keyManagement; // list of accepted authenticated key management protocols
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
			EapNetworkConfig eap_config;
	};
}

#endif
