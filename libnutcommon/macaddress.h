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
	struct MacAddressData {
		quint8 octet[6];
	} __attribute__ ((__packed__));

	struct MacAddress {
		explicit constexpr MacAddress() = default;
		explicit MacAddress(QString const& str);
		explicit MacAddress(quint8 const (&d)[6]);
		explicit MacAddress(ether_addr const* eth);

		template<typename T, size_t N>
		static MacAddress fromBuffer(T const (&d)[N]) {
			static_assert(sizeof(d) >= 6, "buffer not large enough for MAC address");
			return MacAddress(reinterpret_cast<ether_addr const*>(d));
		}

		bool operator==(MacAddress const& b) const;
		bool operator!=(MacAddress const& b) const;
		bool operator<(MacAddress const& b) const;
		QString toString() const;

		bool zero() const;
		bool valid() const;
		void clear();

		MacAddressData data{{0,0,0,0,0,0}};
	};

	uint qHash(libnutcommon::MacAddress const& key);

	QDBusArgument& operator<<(QDBusArgument& argument, MacAddress const& data);
	QDBusArgument const& operator>>(QDBusArgument const& argument, MacAddress& data);
}

Q_DECLARE_METATYPE(libnutcommon::MacAddress)

#endif
