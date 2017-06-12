#ifndef LIBNUTWIRELESS_CWSTYPES_H
#define LIBNUTWIRELESS_CWSTYPES_H

#pragma once

#include <libnutcommon/ssid.h>
#include <libnutcommon/macaddress.h>

#include "types.h"

#include <QHostAddress>

namespace libnutwireless {
	/** Enum of possible NetworkFlags */
	enum NetworkFlags {
		NF_NONE,
		NF_CURRENT,
		NF_DISABLED,
	};

	/** Enum of possible interaction types **/
	enum InteractiveType {
		INTERACT_MSG,
		INTERACT_REQ,
		INTERACT_EVENT,
	};

	/** RequestType contains all possible requests from wpa_supplicant. */
	enum RequestType {
		REQ_FAIL,
		REQ_PASSWORD,
		REQ_IDENTITY,
		REQ_NEW_PASSWORD,
		REQ_PIN,
		REQ_OTP,
		REQ_PASSPHRASE,
	};

	/** Enum of possible config failures.  */
	enum NetconfigFailures {
		NCF_NONE =             0x00000000,
		NCF_SSID =             0x00000001,
		NCF_BSSID =            0x00000002,
		NCF_DISABLED =         0x00000004,
		NCF_ID_STR =           0x00000008,
		NCF_SCAN_SSID =        0x00000010,
		NCF_PRIORITY =         0x00000020,
		NCF_MODE =             0x00000040,
		NCF_FREQ =             0x00000080,
		NCF_PROTO =            0x00000100,
		NCF_KEYMGMT =          0x00000200,
		NCF_AUTH_ALG =         0x00000400,
		NCF_PAIRWISE =         0x00000800,
		NCF_GROUP =            0x00001000,
		NCF_PSK =              0x00002000,
		NCF_EAPOL_FLAGS =      0x00004000,
		NCF_MIXED_CELL =       0x00008000,
		NCF_PROA_KEY_CACHING = 0x00010000,
		NCF_WEP_KEY0 =         0x00020000,
		NCF_WEP_KEY1 =         0x00040000,
		NCF_WEP_KEY2 =         0x00080000,
		NCF_WEP_KEY3 =         0x00100000,
		NCF_WEP_KEY_IDX =      0x00200000,
		NCF_PEERKEY =          0x00400000,
		NCF_ALL =              0x007FFFFF,
	};

	/** Enum of possible eap config failures */
	enum EapNetconfigFailures {
		ENCF_NONE                = 0x00000000,
		ENCF_EAP                 = 0x00000001,
		ENCF_IDENTITY            = 0x00000002,
		ENCF_ANON_IDENTITY       = 0x00000004,
		ENCF_PASSWD              = 0x00000008,
		ENCF_CA_CERT             = 0x00000010,
		ENCF_CA_PATH             = 0x00000020,
		ENCF_CLIENT_CERT         = 0x00000040,
		ENCF_PRIVATE_KEY         = 0x00000080,
		ENCF_PRIVATE_KEY_PASSWD  = 0x00000100,
		ENCF_DH_FILE             = 0x00000200,
		ENCF_SUBJECT_MATCH       = 0x00000400,
		ENCF_ALTSUBJECT_MATCH    = 0x00000800,
		ENCF_PHASE1              = 0x00001000,
		ENCF_PHASE2              = 0x00002000,
		ENCF_CA_CERT2            = 0x00004000,
		ENCF_CA_PATH2            = 0x00008000,
		ENCF_CLIENT_CERT2        = 0x00010000,
		ENCF_PRIVATE_KEY2        = 0x00020000,
		ENCF_PRIVATE_KEY2_PASSWD = 0x00040000,
		ENCF_DH_FILE2            = 0x00080000,
		ENCF_SUBJECT_MATCH2      = 0x00100000,
		ENCF_ALTSUBJECT_MATCH2   = 0x00200000,
		ENCF_FRAGMENT_SIZE       = 0x00400000,
		ENCF_PAC_FILE            = 0x00800000,
		ENCF_ALL                 = 0x00FFFFFF,
	};

	/**
		NetconfigStatus contains error information when configuring a network
		@param failures Standard network failures
		@param eap_failures Eap network failures
		@param id on failures, id is -1, else it's the network id which was configured
	*/
	struct NetconfigStatus {
		explicit NetconfigStatus() { }
		explicit NetconfigStatus(int id) : id(id) {}
		explicit NetconfigStatus(NetconfigFailures failures, EapNetconfigFailures eap_failures, int id=-1)
		: failures(failures), eap_failures(eap_failures), id(id) {}

		NetconfigFailures failures{NCF_NONE};
		EapNetconfigFailures eap_failures{ENCF_NONE};
		int id{-1};
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
		enum MIBVariable_type {
			PLAIN                = 1,
			STRING               = 2,
			NUMBER               = 4,
			LOGIC                = 8,
		};

		MIBVariable_type type;
		QString name;
		union {
			qint32* num;
			QString* str;
			bool* logic;
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
