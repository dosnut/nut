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
#include "log.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>

#include <linux/if_arp.h>
#include <linux/if_ether.h>

#include <QDataStream>

#include "random.h"

namespace nuts {
	/* Copied from udhcp */
	quint16 checksum(char *addr, int count)
	{
    /* Compute Internet Checksum for "count" bytes
		*         beginning at location "addr".
	*/
		register qint32 sum = 0;
		quint16 *source = (quint16*) addr;

		while (count > 1)  {
			/*  This is the inner loop */
			sum += *source++;
			count -= 2;
		}

		/*  Add left-over byte, if any */
		if (count > 0) {
        /* Make sure that the left-over byte is added correctly both
		* with little and big endian hosts */
			quint16 tmp = 0;
			*(quint8 *) (&tmp) = * (quint8 *) source;
			sum += tmp;
		}
		/*  Fold 32-bit sum to 16 bits */
		while (sum >> 16)
			sum = (sum & 0xffff) + (sum >> 16);

		return (quint16) ~sum;
	}
	
	DHCPPacket::DHCPPacket(bool client)
	: sendUnicast(false) {
		memset(&msg, 0, sizeof(msg));
		memset(&headers, 0, sizeof(headers));
		headers.ip.protocol = IPPROTO_UDP;
		headers.ip.daddr = (quint32) -1;
		if (client) {
			headers.udp.source = htons(68);
			headers.udp.dest = htons(67);
			msg.op    = BOOT_REQUEST;
			msg.htype = ARPHRD_ETHER;
			msg.hlen  = ETH_ALEN;
			msg.cookie = htonl(DHCP_MAGIC);
		} else {
			headers.udp.source = htons(67);
			headers.udp.dest = htons(68);
			msg.op    = BOOT_REPLY;
			msg.htype = ARPHRD_ETHER;
			msg.hlen  = ETH_ALEN;
			msg.cookie = htonl(DHCP_MAGIC);
		}
	}
	
	DHCPPacket::DHCPPacket(bool client, const QHostAddress &unicast_addr)
	: sendUnicast(true), unicast_addr(unicast_addr) {
		memset(&msg, 0, sizeof(msg));
		memset(&headers, 0, sizeof(headers));
		headers.ip.protocol = IPPROTO_UDP;
		headers.ip.daddr = (quint32) -1;
		if (client) {
			headers.udp.source = htons(68);
			headers.udp.dest = htons(67);
			msg.op    = BOOT_REQUEST;
			msg.htype = ARPHRD_ETHER;
			msg.hlen  = ETH_ALEN;
			msg.cookie = htonl(DHCP_MAGIC);
		} else {
			headers.udp.source = htons(67);
			headers.udp.dest = htons(68);
			msg.op    = BOOT_REPLY;
			msg.htype = ARPHRD_ETHER;
			msg.hlen  = ETH_ALEN;
			msg.cookie = htonl(DHCP_MAGIC);
		}
	}
	
	DHCPPacket::DHCPPacket(QDataStream &in, quint32 from_ip)
	: sendUnicast(false) {
		memset(&headers, 0, sizeof(headers));
		headers.ip.saddr = from_ip;
		memset(&msg, 0, sizeof(msg));
		if (in.readRawData((char*) &msg, sizeof(msg)) != sizeof(msg))
			return;
		while (!in.atEnd()) {
			quint8 k, size;
			in >> k;
			// return on end option (0xff)
			if (k == DHCP_END)
				return;
			// continue on pad (0x00)
			if (k == DHCP_PADDING)
				continue;
			if (in.atEnd()) return;
			in >> size;
			QVector<quint8> buf(size);
			// unexpected end of stream
			if (in.readRawData((char*) buf.data(), size) != size)
				return;
			options.insert(k, buf);
		}
	}
	
	DHCPPacket::~DHCPPacket() {
	}
		
	DHCPPacket* DHCPPacket::parseRaw(QByteArray &buf) {
		// Packet big enough?
		if (buf.size() < (int) (sizeof(struct dhcp_msg) + sizeof(struct udp_dhcp_packet)))
			return 0;
		struct udp_dhcp_packet *h = (struct udp_dhcp_packet*) buf.data();
		struct dhcp_msg *msg = (struct dhcp_msg*) (sizeof(struct udp_dhcp_packet) + (char*) h);
		// check ip version
		if (h->ip.ihl != (sizeof(h->ip) >> 2) || h->ip.version != IPVERSION)
			return 0;
		// udp?
		if (h->ip.protocol != IPPROTO_UDP)
			return 0;
		// correct source/dest port?
		if (h->udp.source != htons(67) || h->udp.dest != htons(68))
			return 0;
		// check "magic" header values
		if (msg->op != BOOT_REPLY || msg->htype != ARPHRD_ETHER
		   || msg->hlen != ETH_ALEN || msg->cookie != htonl(DHCP_MAGIC))
			return 0;
		// check size headers
		if (h->ip.tot_len != htons(buf.size()) || h->udp.len != htons(buf.size() - sizeof(h->ip)))
			return 0;
		// ignore checksum!
		QDataStream in(buf);
		in.skipRawData(sizeof(*h));
		return new DHCPPacket(in, h->ip.saddr);
	}
	
	DHCPPacket* DHCPPacket::parseData(QByteArray &buf, struct sockaddr_in &from) {
		// Packet big enough?
		if (buf.size() < (int) sizeof(struct dhcp_msg))
			return 0;
		struct dhcp_msg *msg = (struct dhcp_msg*)  buf.data();
		// check "magic" header values
		if (msg->op != BOOT_REPLY || msg->htype != ARPHRD_ETHER
		   || msg->hlen != ETH_ALEN || msg->cookie != htonl(DHCP_MAGIC))
			return 0;
		QDataStream in(buf);
		return new DHCPPacket(in, from.sin_addr.s_addr);
	}
	
	bool DHCPPacket::check() {
		msgdata.clear();
//		msgdata.fill(0, sizeof(headers) + sizeof(msg) + 308);
		QDataStream s(&msgdata, QIODevice::WriteOnly);
		QHashIterator< quint8, QVector<quint8> > i(options);
		s.writeRawData((const char*) &headers, sizeof(headers));
		s.writeRawData((const char*) &msg, sizeof(msg));
		while (i.hasNext()) {
			i.next();
			quint8 k = i.key();
			// Drop end/pad options (0xff/0x00)
			if (k != DHCP_END && k != DHCP_PADDING) {
				s << k;
				// clip size to 255 max.
				quint8 size = (quint8) qMin(255, i.value().size());
				s << size;
				s.writeRawData((const char*) i.value().constData(), size);
			}
		}
		s << (quint8) 255;
		s.device()->close();
		s.unsetDevice();
//		int padlen = (sizeof(headers) + sizeof(msg) + 308) - msgdata.size();
		struct udp_dhcp_packet *h;
		h = (struct udp_dhcp_packet*) msgdata.data();
		h->udp.len = htons(msgdata.size() - sizeof(h->ip));
		h->ip.tot_len = h->udp.len;
		h->udp.check = checksum((char*) h, msgdata.size());
		h->ip.tot_len = htons(msgdata.size());
		h->ip.ihl = sizeof(headers.ip) >> 2;
		h->ip.version = IPVERSION;
		h->ip.ttl = IPDEFTTL;
		h->ip.check = checksum((char*) &h->ip, sizeof(h->ip));
		return true;
	}
	
	libnutcommon::MacAddress DHCPPacket::getClientMac() {
		return libnutcommon::MacAddress(msg.chaddr);
	}
	
	void DHCPPacket::setClientMac(const libnutcommon::MacAddress &chaddr) {
		memcpy(msg.chaddr, chaddr.data, 6);
		quint8 clid[7];
		memcpy(&clid[1], chaddr.data, 6);
		clid[0] = ARPHRD_ETHER;
		setOption(DHCP_CLIENT_ID, clid, sizeof(clid));
	}
	
	void DHCPPacket::setClientAddress(const QHostAddress &addr) {
		msg.ciaddr = htonl(addr.toIPv4Address());
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
	
	DHCPClientPacket::DHCPClientPacket(Interface_IPv4 *iface)
	: DHCPPacket(true), iface(iface) {
		setClientMac(iface->m_env->m_device->getMacAddress());
	}
	
	DHCPClientPacket::DHCPClientPacket(Interface_IPv4 *iface, const QHostAddress &unicast_addr)
	: DHCPPacket(true, unicast_addr), iface(iface) {
		setClientMac(iface->m_env->m_device->getMacAddress());
	}
	
	void DHCPClientPacket::setVendor() {
		setOptionString(DHCP_VENDOR, "nuts-0.1");
	}
	
	void DHCPClientPacket::setParamRequest() {
		quint8 parameter_request[] = {
			0x01, 0x03, 0x06, 0x0c, 0x0f, 0x11, 0x1c, 0x28, 0x29, 0x2a
		};
		setOption(DHCP_PARAM_REQ, parameter_request, sizeof(parameter_request));
	}
	
	void DHCPClientPacket::doDHCPDiscover() {
		quint32 xid = getRandomUInt32();
		while (!iface->registerXID(xid)) xid++;
		setXID(xid);
		setMessageType(DHCP_DISCOVER);
		setParamRequest();
		setVendor();
	}
	
	void DHCPClientPacket::doDHCPRequest(const DHCPPacket &reply) {
		setXID(reply.msg.xid);
		setMessageType(DHCP_REQUEST);
		setOption(DHCP_REQUESTED_IP, (quint8*) &reply.msg.yiaddr, 4);
		// requestIP(QHostAddress(ntohl()));
		setOptionFrom(DHCP_SERVER_ID, reply);
		setParamRequest();
		setVendor();
	}
	
	void DHCPClientPacket::doDHCPRenew(const QHostAddress &ip) {
		// should be unicast to server
		quint32 xid = getRandomUInt32();
		while (!iface->registerUnicastXID(xid)) xid++;
		setXID(xid);
		setMessageType(DHCP_REQUEST);
		setClientAddress(ip);
		setVendor();
	}
	
	void DHCPClientPacket::doDHCPRebind(const QHostAddress &ip) {
		quint32 xid = getRandomUInt32();
		while (!iface->registerXID(xid)) xid++;
		setXID(xid);
		setMessageType(DHCP_REQUEST);
		setClientAddress(ip);
		setVendor();
	}
	
	void DHCPClientPacket::doDHCPRelease(const QHostAddress &ip, const QVector<quint8> server_id) {
		// should be unicast to server
		quint32 xid = getRandomUInt32();
		if (!xid) xid++;
		setXID(xid);
		setMessageType(DHCP_RELEASE);
		setClientAddress(ip);
		setOption(DHCP_SERVER_ID, server_id);
	}
	
	void DHCPClientPacket::requestIP(const QHostAddress &ip) {
		quint32 addr = htonl(ip.toIPv4Address());
		quint8 *a = (quint8*) &addr;
		setOption(DHCP_REQUESTED_IP, a, 4);
	}

	void DHCPClientPacket::send() {
		DHCPPacket::send(iface);
	}
}
