#ifndef NUT_COMMON_MACADDRESS_H
#define NUT_COMMON_MACADDRESS_H

#include <QString>
#include <QDBusArgument>
#include <QHash>
extern "C" {
#include <net/ethernet.h>
}
namespace libnutcommon {
	class MacAddress;

	QDBusArgument &operator<< (QDBusArgument &argument, const MacAddress &data);
	const QDBusArgument &operator>> (const QDBusArgument &argument, MacAddress &data);
}

namespace libnutcommon {
	/**
		@author Stefan Bühler <stbuehler@web.de>
	*/
	class MacAddress {
		public:
			MacAddress() { clear(); }
			MacAddress(const QString &str);
			MacAddress(const quint8 *d);
			MacAddress(const ether_addr * eth);
			quint8 data[6];
			
			inline bool operator==(const MacAddress &ma) const {
				for (int i = 0; i < 6; i++)
					if (data[i] != ma.data[i])
						return false;
				return true;
			}
			inline bool operator!=(const MacAddress &ma) const {
				return !(*this == ma);
			}
			inline bool operator<(const MacAddress &a, const MacAddress &b) {
				if ( *((quint32*)a.data) < *((quint32*)b.data) ) {
					return true;
				}
				else if ( *((quint32*)a.data) == *((quint32*)b.data) ) { //First is equal
					return ( (*((quint16*)((a.data)+4))) < *((quint16*)((b.data)+4)) );
				}
				else {
					return false;
				}
			}
			inline QString toString() const {
				char buf[sizeof("00:00:00:00:00:00")];
				sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
					data[0],data[1],data[2],data[3],data[4],data[5]);
				return QString(buf);
			}
			
			inline bool zero() const {
				return *((quint32*)data) == 0 && *((quint16*)(data+4)) == 0;
			}
			
			inline bool valid() const {
				return !zero();
			}
			
			inline void clear() {
				*((quint32*)data) = 0;
				*((quint16*)(data+4)) = 0;
			}
	};
}

static inline uint qHash(const libnutcommon::MacAddress &key) {
	quint8 data[8] = { key.data[0], key.data[2], key.data[3], key.data[4], key.data[5], key.data[6], 0, 0};
	return qHash(*((quint64*)data));
}

Q_DECLARE_METATYPE(libnutcommon::MacAddress)

#endif
