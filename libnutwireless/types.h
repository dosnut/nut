#ifndef LIBNUTWIRELESS_TYPES_H
#define LIBNUTWIRELESS_TYPES_H

#include <QString>
#include <QList>
#include <QHostAddress>
#include <QSettings>
#include <libnutcommon/macaddress.h>
#include <iwlib.h>
extern "C" {
#include <linux/wireless.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
}
namespace libnutwireless {

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

	/** Possible events from wpa_supplicant, not complete */
	typedef enum {EVENT_OTHER, EVENT_DISCONNECTED, EVENT_CONNECTED, EVENT_TERMINATING, EVENT_PASSWORD_CHANGED, EVENT_EAP_NOTIFICATION, EVENT_EAP_STARTED, EVENT_EAP_METHOD, EVENT_EAP_SUCCESS, EVENT_EAP_FAILURE } EventType;
	
	/** Eapol flags */
	typedef enum {EAPF_UNDEFINED=-1, EAPF_WIRED=0,EAPF_DYN_UNICAST_WEP=1, EAPF_BCAST_WEP=2,EAPF_DEFAULT=3} EapolFlags;

	/** List of Eap methods for wpa_supplicant */
	typedef enum {EAPM_UNDEFINED=0, EAPM_MD5=1,EAPM_MSCHAPV2=2,EAPM_OTP=4,EAPM_GTC=8,EAPM_TLS=16,EAPM_PEAP=32,EAPM_TTLS=64,EAPM_ALL=127, EAPM_AKA=128, EAPM_FAST=256, EAPM_LEAP=512,EAPM_PSK=1024,EAPM_PAX=2048,EAPM_SAKE=4096,EAPM_GPSK=8192} EapMethod;

	/** QOOL is a tri-state. Its name is derived from Bool and qubit. */
	typedef enum {
		QOOL_UNDEFINED=-1, QOOL_FALSE=0,QOOL_TRUE=1
	} QOOL; // Like a qubit :)

	/** Enum of operation modes of wireless networks (see WirelessExtension) */
	typedef enum {
		OPM_AUTO=0, OPM_ADHOC=1, OPM_MANAGED=2,OPM_MASTER=3,OPM_REPEATER=4,OPM_SECONDARY=5,OPM_MONITOR=6,OPM_UNKNOWN_BUG=7
	} OPMODE;

	//Conversion functions
	Protocols toProtocols(QString str);
	KeyManagement toKeyMgmt(QString str);
	AuthenticationAlgs toAuthAlg(QString str);
	PairwiseCiphers toPairwiseCiphers(QString str);
	GroupCiphers toGroupCiphers(QString str);
	EapolFlags toEapolFlags(QString str);
	EapMethod toEapMethod(QString str);

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

	///Convert EapolFlags to QString
	QString toString(EapolFlags flags);
	///Convert EapMethod to QString
	QString toString(EapMethod method);

	///Convert QOOL to QString (QOOL_UNDEFINED="-1", QOOL_FALSE="0", QOOL_TRUE="1")
	QString toNumberString(QOOL b);
	///Convert QOOL to QString (QOOL_UNDEFINED="undefined", QOOL_FALSE="false", QOOL_TRUE="true")
	QString toString(QOOL b);
	///Convert QOOL to bool (QOOL_UNDEFINED=false, QOOL_FALSE=false, QOOL_TRUE=true)
	bool toBool(QOOL b);
	///Convert bool to QOOL (false=QOOL_FALSE, true=QOOL_TRUE)
	QOOL toQOOL(bool b);
	///Convert int to QOOL
	QOOL toQOOL(int i);

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
}

#endif
