#include "macaddress.h"

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
		if (str == QLatin1String("any")) {
			memset(data, 0, sizeof(data));
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
			data[i] = val;
		}
//		qDebug() << QString("-> %1").arg(toString());
	}
	
	MacAddress::MacAddress(const quint8 *d) {
		if (d == 0) {
			MacAddress();
		} else {
			for (int i = 0; i < 6; i++)
				data[i] = d[i];
		}
	}
	MacAddress::MacAddress(const ether_addr * eth) {
		data[0] = (quint8) eth->ether_addr_octet[0];
		data[1] = (quint8) eth->ether_addr_octet[1];
		data[2] = (quint8) eth->ether_addr_octet[2];
		data[3] = (quint8) eth->ether_addr_octet[3];
		data[4] = (quint8) eth->ether_addr_octet[4];
		data[5] = (quint8) eth->ether_addr_octet[5];
	}
}
