//
// C++ Interface: macaddress
//
// Description: Container for MacAddress
//
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef NUT_COMMON_MACADDRESS_H
#define NUT_COMMON_MACADDRESS_H

#include <QString>
#include <QDBusArgument>

namespace nut {
	class MacAddress;

	QDBusArgument &operator<< (QDBusArgument &argument, const MacAddress &data);
	const QDBusArgument &operator>> (const QDBusArgument &argument, MacAddress &data);
}

namespace nut {
	/**
		@author Stefan Bühler <stbuehler@web.de>
	*/
	class MacAddress {
		public:
			MacAddress() { clear(); }
			MacAddress(const QString &str);
			MacAddress(const quint8 *d);
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

Q_DECLARE_METATYPE(nut::MacAddress);

#endif
