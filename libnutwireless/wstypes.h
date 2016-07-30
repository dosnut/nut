#ifndef LIBNUTWIRELESS_CWSTYPES_H
#define LIBNUTWIRELESS_CWSTYPES_H

#include <libnutcommon/ssid.h>

#include "types.h"

namespace libnutwireless {

	/** Enum of possible NetworkFlags */
	typedef enum {NF_NONE, NF_CURRENT, NF_DISABLED} NetworkFlags;

	/** Enum of possible interaction types **/
	typedef enum {INTERACT_MSG, INTERACT_REQ,INTERACT_EVENT} InteractiveType;

	/** RequestType contains all possible requests from wpa_supplicant. */
	typedef enum {REQ_FAIL, REQ_PASSWORD, REQ_IDENTITY, REQ_NEW_PASSWORD, REQ_PIN, REQ_OTP, REQ_PASSPHRASE} RequestType;

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

	/**
		NetconfigStatus contains error information when configuring a network
		@param failures Standard network failures
		@param eap_failures Eap network failures
		@param id on failures, id is -1, else it's the network id which was configured
	*/
	struct NetconfigStatus {
		NetconfigStatus(int id=-1) : failures(NCF_NONE), eap_failures(ENCF_NONE), id(id) {}
		NetconfigStatus(NetconfigFailures failures, EapNetconfigFailures eap_failures, int id=-1) : failures(failures), eap_failures(eap_failures), id(id) {}
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

//	/** One wpa_suplicant network variable. Reserved for future purpose. */
// 	struct NetworkVariable {
// 		typedef enum {PLAIN=1,STRING=2,NUMBER=4,LOGIC=8} Type;
// 		QString name;
// 		union {
// 			int * num;
// 			QString * str;
// 			bool * logic;
// 		} value;
// 	};

	/** Request made by wpa_supplicant */
	struct Request {
		RequestType type;
		int id;
	};

	/** Information about a configured network (see listNetworks) */
	struct ShortNetworkInfo {
		int id;
		libnutcommon::SSID ssid;
		libnutcommon::MacAddress bssid;
		NetworkFlags flags;
		bool adhoc;
	};

	//Comparison functions

	/** Compare ShortNetworkinfo by ID*/
	inline bool lessThanID(ShortNetworkInfo a, ShortNetworkInfo b) {
		return (a.id < b.id);
	}
	/** Compare ShortNetworkinfo by SSID*/
	inline bool lessThanSSID(ShortNetworkInfo a, ShortNetworkInfo b) {
		return QString::localeAwareCompare(a.ssid.quotedString(), b.ssid.quotedString()) < 0;
	}
	/** Compare ShortNetworkinfo by BSSID*/
	inline bool lessThanBSSID(ShortNetworkInfo a, ShortNetworkInfo b) {
		return (a.bssid < b.bssid);
	}
	/** Compare ShortNetworkinfo by NetworkFlags*/
	inline bool lessThanFlags(ShortNetworkInfo a, ShortNetworkInfo b) {
		return ((int) a.flags < (int) b.flags);
	}
	/** Compare ShortNetworkinfo by Adhoc (adhoc is bigger)*/
	inline bool lessThanAdhoc(ShortNetworkInfo a, ShortNetworkInfo b) {
		return (!a.adhoc && b.adhoc);
	}


	//Conversion functions:

	///Convert RequestType to QString
	QString toString(RequestType reqt);
}
#endif
