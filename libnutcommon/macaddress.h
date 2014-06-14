#ifndef NUT_COMMON_MACADDRESS_H
#define NUT_COMMON_MACADDRESS_H

#include <QString>
#include <QDBusArgument>
#include <QHash>
#include <cstdio>

extern "C" {
struct ether_addr;
}

namespace libnutcommon {
	/**
		@author Stefan Bühler <stbuehler@web.de>
	*/
	class MacAddress {
	public:
		static MacAddress const Zero;

		MacAddress() { clear(); }
		MacAddress(const QString &str);
		MacAddress(const quint8 *d);
		MacAddress(const ether_addr * eth);
		struct {
			quint8 bytes[6];
		} data;

		bool operator==(const MacAddress &b) const;
		bool operator!=(const MacAddress &b) const;
		bool operator<(const MacAddress &b) const;
		QString toString() const;

		bool zero() const;
		bool valid() const;
		void clear();
	};

	uint qHash(const libnutcommon::MacAddress &key);

	QDBusArgument &operator<< (QDBusArgument &argument, const MacAddress &data);
	const QDBusArgument &operator>> (const QDBusArgument &argument, MacAddress &data);
}

Q_DECLARE_METATYPE(libnutcommon::MacAddress)

#endif
