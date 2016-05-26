#include "macaddress.h"

extern "C" {
#include <net/ethernet.h>
}

#include <cstring>

#include <QtEndian>

namespace libnutcommon {
	QDBusArgument& operator<<(QDBusArgument& argument, MacAddress const& data) {
		argument << data.toString();
		return argument;
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, MacAddress& data) {
		QString addr;
		argument >> addr;
		data = MacAddress(addr);
		return argument;
	}

	static int read_hexdigit(char c) {
		if ('0' <= c && c <= '9')
			return c - '0';
		if ('a' <= c && c <= 'f')
			return c - 'a' + 10;
		if ('A' <= c && c <= 'F')
			return c - 'A' + 10;
		return -1;
	}

	static char* hex2quint8(char* msg, quint8& val) {
		int i;
		val = 0;
		if (!msg || !*msg) return msg;
		if ((i = read_hexdigit(*msg)) == -1)
			return msg;
		msg++;
		if (!*msg) return msg;
		val = ((quint8) i) << 4;
		if ((i = read_hexdigit(*msg)) == -1)
			return msg;
		msg++;
		val |= i;
		return msg;
	}

	MacAddress::MacAddress(QString const& str) {
		if (str == QLatin1String("any") || str.isEmpty()) {
			return;
		}
		auto buf = str.toUtf8();
		auto s = buf.data();
		MacAddressData addr;
//		qDebug(s);
		for (int i = 0; i < 6; i++) {
			if (!*s) {
//				qDebug() << QString("Unexpected end of mac");
				return;
			}
			quint8 val;
			auto s2 = hex2quint8(s, val);
			if (s2 == s) {
//				qDebug() << QString("No hexdata found at pos %1, '%2'").arg((int) (s - buf.data())).arg(*s);
				return;
			}
			s = s2;
			if (*s && *s++ != ':') {
//				qDebug() << QString("Unexpected char in mac: '%1'").arg(*s);
				return;
			}
			addr.octet[i] = val;
		}
		data = addr;
//		qDebug() << QString("-> %1").arg(toString());
	}

	MacAddress::MacAddress(quint8 const (&d)[6]) {
		memcpy(&data, d, sizeof(data));
	}
	MacAddress::MacAddress(ether_addr const* eth) {
		memcpy(data.octet, eth->ether_addr_octet, sizeof(data));
	}

	bool MacAddress::operator==(MacAddress const& b) const {
		return 0 == std::memcmp(&data, &b.data, sizeof(data));
	}

	bool MacAddress::operator!=(MacAddress const& b) const {
		return 0 != std::memcmp(&data, &b.data, sizeof(data));
	}

	bool MacAddress::operator<(MacAddress const& b) const {
		return 0 < std::memcmp(&data, &b.data, sizeof(data));
	}
	QString MacAddress::toString() const {
		char buf[sizeof("00:00:00:00:00:00")];
		sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
			data.octet[0],
			data.octet[1],
			data.octet[2],
			data.octet[3],
			data.octet[4],
			data.octet[5]);
		return QString(buf);
	}

	bool MacAddress::zero() const {
		return MacAddress() == *this;
	}

	bool MacAddress::valid() const {
		return MacAddress() != *this;
	}

	void MacAddress::clear() {
		*this = MacAddress();
	}

	uint qHash(MacAddress const& key) {
		auto n = qFromBigEndian<quint64>(key.data.octet);
		return ::qHash(n);
	}
}
