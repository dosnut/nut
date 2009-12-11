#include "macaddress.h"

extern "C" {
#include <net/ethernet.h>
}

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
	
	static inline int read_hexdigit(char c) {
		if ('0' <= c && c <= '9')
			return c - '0';
		if ('a' <= c && c <= 'f')
			return c - 'a' + 10;
		if ('A' <= c && c <= 'F')
			return c - 'A' + 10;
		return -1;
	}
	
	static inline char* hex2quint8(char* msg, quint8 &val) {
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
	
	MacAddress::MacAddress(const QString &str) {
		data.ui64 = 0;
		if (str == QLatin1String("any") || str.isEmpty()) {
			return;
		}
		QByteArray buf = str.toUtf8();
		char *s = buf.data();
//		qDebug(s);
		quint8 val;
		char *s2;
		for (int i = 0; i < 6; i++) {
			if (!*s) {
//				qDebug() << QString("Unexpected end of mac");
				return;
			}
			if ((s2 = hex2quint8(s, val)) == s) {
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
			data.ui64 = 0;
			for (int i = 0; i < 6; i++)
				data.bytes[i] = d[i];
		}
	}
	MacAddress::MacAddress(const ether_addr * eth) {
		data.ui64 = 0;
		data.bytes[0] = (quint8) eth->ether_addr_octet[0];
		data.bytes[1] = (quint8) eth->ether_addr_octet[1];
		data.bytes[2] = (quint8) eth->ether_addr_octet[2];
		data.bytes[3] = (quint8) eth->ether_addr_octet[3];
		data.bytes[4] = (quint8) eth->ether_addr_octet[4];
		data.bytes[5] = (quint8) eth->ether_addr_octet[5];
	}
}
