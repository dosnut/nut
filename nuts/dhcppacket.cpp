//
// C++ Implementation: dhcppacket
//
// Description: 
//
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "dhcppacket.h"

#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <linux/if_arp.h>
#include <linux/if_ether.h>

#include <QDataStream>

namespace nuts {
	static quint32 get_rand() {
		static bool initialized = false;
		if (!initialized) {
			initialized = true;
			quint32 seed;
			int fd = open("/dev/urandom", O_RDONLY);
			if (fd >= 0 && read(fd, &seed, sizeof(seed)) == sizeof(seed)) {
				srand(seed);
			} else {
				srand(time(0));
			}
			if (fd >= 0)
				close(fd);
		}
		return rand();
	}
	
	DHCPPacket::DHCPPacket(bool client) {
		memset(&msg, 0, sizeof(msg));
		if (client) {
			msg.op    = BOOT_REQUEST;
			msg.htype = ARPHRD_ETHER;
			msg.hlen  = ETH_ALEN;
			msg.cookie = htonl(DHCP_MAGIC);
			msg.options[0] = DHCP_END;
		}
	}
	
	DHCPPacket::~DHCPPacket() {
	}
	
	bool DHCPPacket::check() {
		QByteArray buf;
		QDataStream s(&buf, QIODevice::WriteOnly);
		QHashIterator< quint8, QVector<quint8> > i(options);
		while (i.hasNext()) {
			i.next();
			s << i.key() << ((quint8) (quint32) i.value().size());
			s.writeRawData((const char*) i.value().constData(), i.value().size());
		}
		return false;
	}
	
	void DHCPPacket::setClientMac(const MacAddress &chaddr) {
		memcpy(msg.chaddr, chaddr.data, 6);
	}
	
	void DHCPPacket::setXID(quint32 xid) {
		msg.xid = xid;
	}
	
	void DHCPPacket::setOption(quint8 op, const QVector<quint8>& data) {
		options.insert(op, data);
	}
	
	void DHCPPacket::setOption(quint8 op, const quint8* data, int size) {
		QVector<quint8> buf(size);
		memcpy(buf.data(), data, size);
		options.insert(op, buf);
	}
	
	void DHCPPacket::setOptionString(quint8 op, const QString& s) {
		QByteArray buf = s.toUtf8();
		int size = buf.size();
		if (size > 255) {
			buf.resize(255);
			size = 255;
		}
		QVector<quint8> buf2(size);
		memcpy(buf2.data(), buf.data(), size);
		options.insert(op, buf2);
	}
	
	void DHCPPacket::setOptionFrom(quint8 op, const DHCPPacket &p) {
		options.insert(op, p.options.value(op));
	}
	
	DHCPClientPacket::DHCPClientPacket()
	: DHCPPacket(true) {
	}
	
	void DHCPClientPacket::doDHCPDiscover() {
		setXID(get_rand());
		setMessageType(DHCP_DISCOVER);
	}
	
	void DHCPClientPacket::doDHCPRequest(const DHCPPacket &reply) {
		setXID(reply.msg.xid);
		setMessageType(DHCP_REQUEST);
		setOption(DHCP_REQUESTED_IP, (quint8*) &reply.msg.yiaddr, 4);
		// requestIP(QHostAddress(ntohl()));
		setOptionFrom(DHCP_SERVER_ID, reply);
	}
	
	void DHCPClientPacket::requestIP(const QHostAddress &ip) {
		quint32 addr = htonl(ip.toIPv4Address());
		quint8 *a = (quint8*) &addr;
		setOption(DHCP_REQUESTED_IP, a, 4);
	}

}
