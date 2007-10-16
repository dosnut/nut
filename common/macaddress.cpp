//
// C++ Implementation: macaddress
//
// Description: 
//
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "macaddress.h"

namespace nut {
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
		QByteArray buf = str.toUtf8();
		char *s = buf.data();
		quint8 val;
		char *s2;
		for (int i = 0; i < 6; i++) {
			if (!*s) return;
			if ((s2 = hex2quint8(s, val)) == s) return;
			s = s2;
			if (*s && *s != ':') return;
			data[i] = val;
		}
	}
	
	MacAddress::MacAddress(const quint8 *d) {
		if (d == 0) {
			MacAddress();
		} else {
			for (int i = 0; i < 6; i++)
				data[i] = d[i];
		}
	}
}
