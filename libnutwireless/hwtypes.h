#ifndef LIBNUTWIRELESS_CHWTYPES_H
#define LIBNUTWIRELESS_CHWTYPES_H

#include "types.h"

namespace libnutwireless {

	/** enum of possible signal encodig */
	typedef enum {
		WSR_UNKNOWN=0, WSR_RCPI=1, WSR_ABSOLUTE=2, WSR_RELATIVE=3
	} SignalQualityType; 
	
	/** signal information in human readable format */
	struct SignalQuality {
		SignalQuality() : frequency(-1), type(WSR_UNKNOWN) {
			quality.value = 0;
			quality.maximum = -1;
			noise.rcpi = 0.0;
			level.rcpi = 0.0;
		}
		int frequency;
		SignalQualityType type;
		QList<qint32> bitrates; //Current bitrate
		QString ssid;
		libnutcommon::MacAddress bssid;
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
	
	/** One scan result (network) in human readable format */
	struct ScanResult {
		ScanResult() {
			freq = -1;
			group = GCI_UNDEFINED;
			pairwise = PCI_UNDEFINED;
			keyManagement = KM_UNDEFINED;
			protocols = PROTO_UNDEFINED;
			opmode = OPM_AUTO;
		}
		libnutcommon::MacAddress bssid;
		QString ssid;
		int freq;
		SignalQuality signal;
		GroupCiphers group;
		PairwiseCiphers pairwise;
		KeyManagement keyManagement;
		Protocols protocols;
		OPMODE opmode;
		QList<qint32> bitrates;
	};
	//Comparison functions
	/** Compare ScanResult by BSSID */
	inline bool lessThanBSSID(libnutwireless::ScanResult a, libnutwireless::ScanResult b) {
		return (a.bssid < b.bssid);
	}
	/** Compare ScanResult by SSID */
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

	/** Compare ScanResult by keymanagement protocol (sort order in enum) */
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
		return (!a.opmode && b.opmode);
	}
	/** Compare ScanResult by highest available bitrate */
	inline bool lessThanBitrates(libnutwireless::ScanResult a, libnutwireless::ScanResult b) {
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
}
#endif