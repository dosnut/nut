#ifndef NUT_COMMON_MACADDRESS_H
#define NUT_COMMON_MACADDRESS_H

#include <QString>
#include <QDBusArgument>
#include <QHash>
#include <cstdio>

struct ether_addr;

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
			union {
				quint64 ui64;
				quint8 bytes[6];
				quint16 words[3];
			} data;

			inline bool operator==(const MacAddress &ma) const {
				return data.ui64 == ma.data.ui64;
			}
			inline bool operator!=(const MacAddress &ma) const {
				return !(*this == ma);
			}
			inline bool operator<(const MacAddress &b) {
				return data.ui64 < b.data.ui64;
			}
			inline QString toString() const {
				char buf[sizeof("00:00:00:00:00:00")];
				sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
					data.bytes[0],data.bytes[1],data.bytes[2],data.bytes[3],data.bytes[4],data.bytes[5]);
				return QString(buf);
			}

			inline bool zero() const {
				return data.ui64 == 0;
			}

			inline bool valid() const {
				return !zero();
			}

			inline void clear() {
				data.ui64 = 0;
			}
	};
}

static inline uint qHash(const libnutcommon::MacAddress &key) {
	return qHash(key.data.ui64);
}

Q_DECLARE_METATYPE(libnutcommon::MacAddress)

#endif
