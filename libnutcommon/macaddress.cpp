#include "macaddress.h"

extern "C" {
#include <net/ethernet.h>
}

#include <cstring>

#include <QtEndian>

namespace libnutcommon {
	QDBusArgument &operator<< (QDBusArgument &argument, const MacAddress &data) {
		argument << data.toString();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, MacAddress &data) {
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

	static char* hex2quint8(char* msg, quint8 &val) {
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

	MacAddress const MacAddress::Zero { };

	MacAddress::MacAddress(const QString &str) {
		data = Zero.data;
		if (str == QLatin1String("any") || str.isEmpty()) {
			return;
		}
		auto buf = str.toUtf8();
		auto s = buf.data();
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
			data.bytes[i] = val;
		}
//		qDebug() << QString("-> %1").arg(toString());
	}

	MacAddress::MacAddress(const quint8 *d) {
		if (d == 0) {
			MacAddress();
		} else {
			memcpy(data.bytes, d, sizeof(data.bytes));
		}
	}
	MacAddress::MacAddress(const ether_addr* eth) {
		memcpy(data.bytes, eth->ether_addr_octet, sizeof(data));
	}

	bool MacAddress::operator==(const MacAddress &b) const {
		return 0 == std::memcmp(&data, &b.data, sizeof(data));
	}

	bool MacAddress::operator!=(const MacAddress &b) const {
		return 0 != std::memcmp(&data, &b.data, sizeof(data));
	}

	bool MacAddress::operator<(const MacAddress &b) const {
		return 0 < std::memcmp(&data, &b.data, sizeof(data));
	}
	QString MacAddress::toString() const {
		char buf[sizeof("00:00:00:00:00:00")];
		sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
			data.bytes[0],data.bytes[1],data.bytes[2],data.bytes[3],data.bytes[4],data.bytes[5]);
		return QString(buf);
	}

	bool MacAddress::zero() const {
		return Zero == *this;
	}

	bool MacAddress::valid() const {
		return Zero != *this;
	}

	void MacAddress::clear() {
		data = Zero.data;
	}

	uint qHash(const MacAddress& key) {
		auto n = qFromBigEndian<quint64>(key.data.bytes);
		return ::qHash(n);
	}
}
