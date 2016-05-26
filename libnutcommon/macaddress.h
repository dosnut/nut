#ifndef NUT_COMMON_MACADDRESS_H
#define NUT_COMMON_MACADDRESS_H

#pragma once

#include <QString>
#include <QDBusArgument>
#include <QHash>
#include <cstdio>

extern "C" {
struct ether_addr;
}

namespace libnutcommon {
	class MacAddress {
	public:
		static MacAddress const Zero;

		explicit MacAddress() { clear(); }
		explicit MacAddress(QString const& str);
		explicit MacAddress(quint8 const* d);
		explicit MacAddress(ether_addr const* eth);

		bool operator==(MacAddress const& b) const;
		bool operator!=(MacAddress const& b) const;
		bool operator<(MacAddress const& b) const;
		QString toString() const;

		bool zero() const;
		bool valid() const;
		void clear();

		struct {
			quint8 bytes[6];
		} data;
	};

	uint qHash(libnutcommon::MacAddress const& key);

	QDBusArgument& operator<<(QDBusArgument& argument, MacAddress const& data);
	QDBusArgument const& operator>>(QDBusArgument const& argument, MacAddress& data);
}

Q_DECLARE_METATYPE(libnutcommon::MacAddress)

#endif
