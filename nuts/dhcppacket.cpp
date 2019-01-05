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
	/* Compute "Internet Checksum" (RFC 768)
	 *
	 * Calculate the checksum over data while the checksum field is set to 0;
	 * then set it to the value computed with this function.
	 *
	 * To verify calculate the checksum over the data containing the checksum;
	 * if it returns 0 the checksum was valid.
	 *
	 * The "Internet Checksum" is the one's complement of the sum using one's
	 * complement arithmetic on 16-bit values (endianess doesn't matter) after
	 * padding with zeores at the end to a multiple of 16-bits.
	 *
	 * This function never returns 0xffff.
	 *
	 *
	 * Math background:
	 *
	 * One's complement arithmetic means that (-n) is (~n), i.e. negation simply
	 * flips all bits. This implies there are two representations for 0; "-0" has
	 * bits set.
	 *
	 * An integer in one's complement arithmetic can also be read as unsigned
	 * representation in Z/(2^b-1)Z, i.e. mod (2^b - 1). This again implies
	 * that 0 and (2^b-1) are both representing 0.
	 *
	 * This "unsigned" representation allows us to actually work with the numbers;
	 * if we perform unsigned addition, we can handle overflows in mod (2^b-1):
	 * if (a + b) >= 2^b, we simply subtract (2^b-1); this can be done by simple
	 * bit masking and shifting.
	 *
	 * The operation ("fold"):
	 *    x := (x & (2^b-1)) + (x >> b)
	 * changes the representation (reducing the width by at least b-1 bits, but
	 * not below b bits), but not the semantic value in Z/(2^b-1)Z.
	 *
	 * The fold operation above also never reduces the output to 0 if the input
	 * wasn't already zero.
	 *
	 * Due to the way overflows get folded endianess doesn't have an impact on the
	 * result.
	 *
	 * RFC 768 says the sum needs to be 0xffff for the data to be valid; as this
	 * function returns the complement of the sum one needs to compare with 0
	 * instead.
	 */
	quint16 checksum(char const* addr, size_t count)
	{
		/* 0xffff and 0x0 both represent zero; using 0xffff makes
		 * sure sum never is 0 at the end, even if all data was zero
		 */
		quint32 sum = 0xffff;

		/* sum must not overflow, so we fold after each step.
		 * theoretically this only needs to be done every ~2^16 steps,
		 * but performance is not that important for a few packets
		 */

		while (count > 1)  {
			quint16 s;
			memcpy(&s, addr, sizeof(s));

			sum += s;
			/* fold */
			sum = (sum & 0xffff) + (sum >> 16);

			addr += 2;
			count -= 2;
		}

		/* add left-over byte, if any */
		if (count > 0) {
			/* Make sure that the left-over byte is aligned correctly both
			 * with little and big endian hosts */
			quint16 s = 0;
			memcpy(&s, addr, 1);
			sum += s;
			/* fold */
			sum = (sum & 0xffff) + (sum >> 16);
		}

		/* sum is never 0, so ~sum is never 0xffff */

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
			msg.op    = bootp_op::REQUEST;
			msg.htype = ARPHRD_ETHER;
			msg.hlen  = ETH_ALEN;
			msg.cookie = htonl(DHCP_MAGIC);
		} else {
			headers.udp.source = htons(67);
			headers.udp.dest = htons(68);
			msg.op    = bootp_op::REPLY;
			msg.htype = ARPHRD_ETHER;
			msg.hlen  = ETH_ALEN;
			msg.cookie = htonl(DHCP_MAGIC);
		}
	}

	DHCPPacket::DHCPPacket(bool client, QHostAddress const& unicast_addr)
	: sendUnicast(true), unicast_addr(unicast_addr) {
		memset(&msg, 0, sizeof(msg));
		memset(&headers, 0, sizeof(headers));
		headers.ip.protocol = IPPROTO_UDP;
		headers.ip.daddr = (quint32) -1;
		if (client) {
			headers.udp.source = htons(68);
			headers.udp.dest = htons(67);
			msg.op    = bootp_op::REQUEST;
			msg.htype = ARPHRD_ETHER;
			msg.hlen  = ETH_ALEN;
			msg.cookie = htonl(DHCP_MAGIC);
		} else {
			headers.udp.source = htons(67);
			headers.udp.dest = htons(68);
			msg.op    = bootp_op::REPLY;
			msg.htype = ARPHRD_ETHER;
			msg.hlen  = ETH_ALEN;
			msg.cookie = htonl(DHCP_MAGIC);
		}
	}

	DHCPPacket::DHCPPacket(QDataStream& in, quint32 from_ip)
	: sendUnicast(false) {
		memset(&headers, 0, sizeof(headers));
		headers.ip.saddr = from_ip;
		memset(&msg, 0, sizeof(msg));
		if (in.readRawData((char*) &msg, sizeof(msg)) != sizeof(msg))
			return;
		while (!in.atEnd()) {
			quint8 k, size;
			in >> k;
			dhcp_option kopt = static_cast<dhcp_option>(k);
			// return on end option (0xff)
			if (kopt == dhcp_option::END)
				return;
			// continue on pad (0x00)
			if (kopt == dhcp_option::PADDING)
				continue;
			if (in.atEnd()) return;
			in >> size;
			QVector<quint8> buf(size);
			// unexpected end of stream
			if (in.readRawData((char*) buf.data(), size) != size)
				return;
			options.emplace(kopt, buf);
		}
	}

	DHCPPacket* DHCPPacket::parseRaw(QByteArray const& buf) {
		// Packet big enough?
		if (buf.size() < (int) (sizeof(struct dhcp_msg) + sizeof(struct udp_dhcp_packet)))
			return nullptr;
		struct udp_dhcp_packet const* h = reinterpret_cast<struct udp_dhcp_packet const*>(buf.constData());
		struct dhcp_msg const* msg = reinterpret_cast<struct dhcp_msg const*>(sizeof(struct udp_dhcp_packet) + buf.constData());
		// check ip version
		if (h->ip.ihl != (sizeof(h->ip) >> 2) || h->ip.version != IPVERSION)
			return nullptr;
		// udp?
		if (h->ip.protocol != IPPROTO_UDP)
			return nullptr;
		// correct source/dest port?
		if (h->udp.source != htons(67) || h->udp.dest != htons(68))
			return nullptr;
		// check "magic" header values
		if (msg->op != bootp_op::REPLY || msg->htype != ARPHRD_ETHER
		   || msg->hlen != ETH_ALEN || msg->cookie != htonl(DHCP_MAGIC))
			return nullptr;
		// check size headers:
		// ip total len must not exceed buffer
		if (ntohs(h->ip.tot_len) > buf.size())
			return nullptr;
		// udp len + ip header must not exceed ip total len
		if (ntohs(h->udp.len) + (h->ip.ihl << 2) > h->ip.tot_len)
			return nullptr;
		// ignore checksum and trailing data!
		QDataStream in(buf.mid(h->ip.ihl << 2, ntohs(h->udp.len)));
		return new DHCPPacket(in, h->ip.saddr);
	}

	DHCPPacket* DHCPPacket::parseData(QByteArray const& buf, struct sockaddr_in const& from) {
		// Packet big enough?
		if (buf.size() < (int) sizeof(struct dhcp_msg))
			return 0;
		struct dhcp_msg const* msg = reinterpret_cast<struct dhcp_msg const*>(buf.constData());
		// check "magic" header values
		if (msg->op != bootp_op::REPLY || msg->htype != ARPHRD_ETHER
		   || msg->hlen != ETH_ALEN || msg->cookie != htonl(DHCP_MAGIC))
			return 0;
		QDataStream in(buf);
		return new DHCPPacket(in, from.sin_addr.s_addr);
	}

	bool DHCPPacket::check() {
		if (creationFailed) return false;
		msgdata.clear();
//		msgdata.fill(0, sizeof(headers) + sizeof(msg) + 308);
		QDataStream s(&msgdata, QIODevice::WriteOnly);
		s.writeRawData((const char*) &headers, sizeof(headers));
		s.writeRawData((const char*) &msg, sizeof(msg));
		for (auto i: options) {
			dhcp_option k = i.first;
			// Drop end/pad options (0xff/0x00)
			if (k != dhcp_option::END && k != dhcp_option::PADDING) {
				s << static_cast<quint8>(k);
				// clip size to 255 max.
				quint8 size = (quint8) qMin(255, i.second.size());
				s << size;
				s.writeRawData((const char*) i.second.constData(), size);
			}
		}
		s << (quint8) 255;
		s.device()->close();
		s.unsetDevice();
//		int padlen = (sizeof(headers) + sizeof(msg) + 308) - msgdata.size();
		struct udp_dhcp_packet* h = reinterpret_cast<struct udp_dhcp_packet*>(msgdata.data());
		h->udp.len = htons(msgdata.size() - sizeof(h->ip));
		h->ip.tot_len = h->udp.len;
		h->udp.check = checksum((char*) h, msgdata.size());
		h->ip.tot_len = htons(msgdata.size());
		h->ip.ihl = sizeof(headers.ip) >> 2;
		h->ip.version = IPVERSION;
		h->ip.ttl = IPDEFTTL;
		h->ip.check = checksum(reinterpret_cast<char const*>(&h->ip), sizeof(h->ip));
		return true;
	}

	libnutcommon::MacAddress DHCPPacket::getClientMac() {
		return libnutcommon::MacAddress::fromBuffer(msg.chaddr);
	}

	void DHCPPacket::setClientMac(libnutcommon::MacAddress const& chaddr) {
		memcpy(msg.chaddr, &chaddr.data, 6);
		quint8 clid[7];
		memcpy(&clid[1], &chaddr.data, 6);
		clid[0] = ARPHRD_ETHER;
		setOption(dhcp_option::CLIENT_ID, clid, sizeof(clid));
	}

	void DHCPPacket::setClientAddress(QHostAddress const& addr) {
		msg.ciaddr = htonl(addr.toIPv4Address());
	}

	void DHCPPacket::setXID(quint32 xid) {
		msg.xid = xid;
	}

	void DHCPPacket::setOption(dhcp_option op, QVector<quint8> const& data) {
		options.emplace(op, data);
	}

	void DHCPPacket::setOption(dhcp_option op, quint8 const* data, int size) {
		QVector<quint8> buf(size);
		memcpy(buf.data(), data, size);
		options.emplace(op, buf);
	}

	void DHCPPacket::setOptionString(dhcp_option op, QString const& s) {
		QByteArray buf = s.toUtf8();
		int size = buf.size();
		if (size > 255) {
			buf.resize(255);
			size = 255;
		}
		QVector<quint8> buf2(size);
		memcpy(buf2.data(), buf.data(), size);
		options.emplace(op, buf2);
	}

	void DHCPPacket::setOptionFrom(dhcp_option op, DHCPPacket const& p) {
		options.emplace(op, p.getOption(op));
	}

	DHCPClientPacket::DHCPClientPacket(Interface_IPv4* iface)
	: DHCPPacket(true), iface(iface) {
		setClientMac(iface->m_env->m_device->getMacAddress());
	}

	DHCPClientPacket::DHCPClientPacket(Interface_IPv4* iface, QHostAddress const& unicast_addr)
	: DHCPPacket(true, unicast_addr), iface(iface) {
		setClientMac(iface->m_env->m_device->getMacAddress());
	}

	void DHCPClientPacket::setVendor() {
		setOptionString(dhcp_option::VENDOR, "nuts-0.1");
	}

	void DHCPClientPacket::setParamRequest() {
		quint8 parameter_request[] = {
			0x01, 0x03, 0x06, 0x0c, 0x0f, 0x11, 0x1c, 0x28, 0x29, 0x2a
		};
		setOption(dhcp_option::PARAM_REQ, parameter_request, sizeof(parameter_request));
	}

	void DHCPClientPacket::doDHCPDiscover() {
		quint32 xid = getRandomUInt32();
		while (!iface->registerXID(xid)) xid++;
		setXID(xid);
		setMessageType(dhcp_message_type::DISCOVER);
		setParamRequest();
		setVendor();
	}

	void DHCPClientPacket::doDHCPRequest(DHCPPacket const& reply) {
		setXID(reply.msg.xid);
		setMessageType(dhcp_message_type::REQUEST);
		setOption(dhcp_option::REQUESTED_IP, (quint8*) &reply.msg.yiaddr, 4);
		// requestIP(QHostAddress(ntohl()));
		setOptionFrom(dhcp_option::SERVER_ID, reply);
		setParamRequest();
		setVendor();
	}

	void DHCPClientPacket::doDHCPRenew(QHostAddress const& ip) {
		// should be unicast to server
		quint32 xid = 0;
		if (!iface->registerUnicastXID(xid)) {
			creationFailed = true;
		}
		setXID(xid);
		setMessageType(dhcp_message_type::REQUEST);
		setClientAddress(ip);
		setVendor();
	}

	void DHCPClientPacket::doDHCPRebind(QHostAddress const& ip) {
		quint32 xid = getRandomUInt32();
		while (!iface->registerXID(xid)) xid++;
		setXID(xid);
		setMessageType(dhcp_message_type::REQUEST);
		setClientAddress(ip);
		setVendor();
	}

	void DHCPClientPacket::doDHCPRelease(QHostAddress const& ip, QVector<quint8> server_id) {
		// should be unicast to server
		quint32 xid = getRandomUInt32();
		if (!xid) xid++;
		setXID(xid);
		setMessageType(dhcp_message_type::RELEASE);
		setClientAddress(ip);
		setOption(dhcp_option::SERVER_ID, server_id);
	}

	void DHCPClientPacket::requestIP(QHostAddress const& ip) {
		quint32 addr = htonl(ip.toIPv4Address());
		quint8 const* a = reinterpret_cast<quint8 const*>(&addr);
		setOption(dhcp_option::REQUESTED_IP, a, 4);
	}

	void DHCPClientPacket::send() {
		DHCPPacket::send(iface);
	}
}
