//
// C++ Implementation: arp
//
// Description: 
//
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "arp.h"

#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>

#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>

#include "log.h"
#include "device.h"

typedef struct tv {
	time_t sec;
	suseconds_t usec;
	tv(time_t sec = 0, suseconds_t usec = 0)
	: sec(sec), usec(usec) { }
} tv;

class Time {
	private:
		tv m_tv;
	
	public:
		Time(time_t sec = 0, suseconds_t usec = 0)
		: m_tv(sec, usec) { }
		
		static Time current() {
			struct timeval tv = { 0, 0 };
			gettimeofday(&tv, 0);
			return Time(tv.tv_sec, tv.tv_usec);
		}
		
		static Time random(int min, int max) {
			
		}
		
		Time operator +(const Time &a) const {
			return Time(m_tv.sec + a.m_tv.sec, m_tv.usec + a.m_tv.usec);
		}
		Time operator -(const Time &a) const {
			return Time(m_tv.sec + a.m_tv.sec, m_tv.usec + a.m_tv.usec);
		}
		Time& operator += (const Time &a) {
			m_tv.sec += a.m_tv.sec; m_tv.usec += a.m_tv.usec;
			return *this;
		}
		Time& operator -= (const Time &a) {
			m_tv.sec -= a.m_tv.sec; m_tv.usec -= a.m_tv.usec;
			return *this;
		}
		
		bool operator <=(const Time &a) const {
			return (m_tv.sec < a.m_tv.sec) || (m_tv.sec == a.m_tv.sec && m_tv.usec <= a.m_tv.usec);
		}
		bool operator >=(const Time &a) const {
			return (a <= *this);
		}
		bool operator <(const Time &a) const {
			return !(a <= *this);
		}
		bool operator >(const Time &a) const {
			return !(*this >= a);
		}
};

// Constant values from other headers:
#if 0
#define ARPOP_REQUEST   1
#define ARPOP_REPLY 2

#define ARPHRD_ETHER    1
#define ETH_ALEN    6
#define ETH_P_IP    0x0800
#endif

#define ARP_BASE_STRUCT \
	quint16 hwtype, ptype; \
	quint8 hlen, plen; \
	quint16 operation;

typedef struct arp_packet_base arp_packet_base;
struct arp_packet_base {
	ARP_BASE_STRUCT
};

typedef struct arp_packet_ipv4 arp_packet_ipv4;
struct arp_packet_ipv4 {
	ARP_BASE_STRUCT
	quint8 sender_hw_addr[ETH_ALEN];
	quint32 sender_p_addr;
	quint8 target_hw_addr[ETH_ALEN];
	quint32 target_p_addr;
};

static bool arp_parse(const QByteArray &data, quint8 &proto_len, nuts::ArpOperation &operation) {
	if (data.size() < (int) sizeof(arp_packet_base))
		return false;
	arp_packet_base *packet = (arp_packet_base*) data.data();
	if (   (packet->hwtype != htons(ARPHRD_ETHER))
		|| (packet->ptype != htons(ETH_P_IP))
		|| (packet->hlen != ETH_ALEN))
		return false;
	proto_len = packet->plen;
	operation = (nuts::ArpOperation) ntohs(packet->operation);
	return true;
}

template<typename Packet>
static void arp_base_request(Packet &packet, quint8 proto_len, nuts::ArpOperation operation) {
	packet.hwtype = htons(ARPHRD_ETHER);
	packet.ptype = htons(ETH_P_IP);
	packet.hlen = ETH_ALEN;
	packet.plen = proto_len;
	packet.operation = htons((quint16) operation);
}

static void arp_ipv4_request(arp_packet_ipv4 &packet, const nut::MacAddress &sender_mac, const QHostAddress &sender_ip, const QHostAddress &target_ip) {
	arp_base_request(packet, 4, nuts::ARP_REQUEST);
	memcpy(packet.sender_hw_addr, sender_mac.data, ETH_ALEN);
	packet.sender_p_addr = sender_ip.toIPv4Address();
	packet.target_p_addr = target_ip.toIPv4Address();
}

static bool arp_ipv4_parse(const QByteArray &data, nut::MacAddress &sender_mac, QHostAddress &sender_ip, nut::MacAddress &target_mac, QHostAddress &target_ip) {
	if (data.size() != sizeof(arp_packet_ipv4))
		return false;
	arp_packet_ipv4 *packet = (arp_packet_ipv4*) data.data();
	if (   (packet->hwtype != htons(ARPHRD_ETHER))
		|| (packet->ptype != htons(ETH_P_IP))
		|| (packet->hlen != ETH_ALEN)
		|| (packet->plen != 4))
		return false;
	sender_mac = nut::MacAddress(packet->sender_hw_addr);
	sender_ip = QHostAddress(packet->sender_p_addr);
	target_mac = nut::MacAddress(packet->target_hw_addr);
	target_ip = QHostAddress(packet->target_p_addr);
	return true;
}

static void writeARPPacket(int socket, int if_index, const QByteArray &data) {
	struct sockaddr_ll sock;
	memset(&sock, 0, sizeof(sock));
	sock.sll_family = AF_PACKET;
	sock.sll_protocol = htons(ETH_P_ARP);
	sock.sll_ifindex = if_index;
	sock.sll_halen = ETH_ALEN;
	memset(sock.sll_addr, 0xff, ETH_ALEN);
	sendto(socket, data.data(), data.size(), 0, (struct sockaddr *) &sock, sizeof(sock));
}

static QByteArray readARPPacket(int socket) {
	struct sockaddr_ll sock;
	socklen_t slen = sizeof(sock);
	int size;
	if (ioctl(socket, FIONREAD, &size) < 0) {
		nuts::err << QString("readARPPacket: ioctl(FIONREAD) failed: %1").arg(strerror(errno)) << endl;
		return QByteArray();
	}
	if (size <= 0) size = 512;
	QByteArray buf;
	buf.resize(size);
	size = recvfrom(socket, buf.data(), buf.size(), 0, (struct sockaddr *) &sock, &slen);
	if (size < 0) {
		nuts::err << QString("readARPPacket: recvfrom(...) failed: %1").arg(strerror(errno)) << endl;
		return QByteArray();
	}
	buf.resize(size);
	return buf;
}

namespace nuts {
	ARP::ARP(Device* device)
	: QObject(device), m_device(device) {
	}

	ARP::~ARP() {
		stop();
	}
	
	bool ARP::start() {
		int if_index = m_device->interfaceIndex;
		if (if_index < 0) {
			err << "Interface index invalid" << endl;
			return false;
		}
		if ((m_arp_socket = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_ARP))) < 0) {
			err << "Couldn't open PF_PACKET for arp" << endl;
			m_arp_socket = -1;
			return false;
		}
//		const char MAC_BCAST_ADDR[] = "\xff\xff\xff\xff\xff\xff";
		struct sockaddr_ll sock;
		memset(&sock, 0, sizeof(sock));
		sock.sll_family = AF_PACKET;
		sock.sll_protocol = htons(ETH_P_ARP);
		sock.sll_ifindex = if_index;
//		sock.sll_halen = 6;
//		memcpy(sock.sll_addr, MAC_BCAST_ADDR, 6);
		if (bind(m_arp_socket, (struct sockaddr *) &sock, sizeof(sock)) < 0) {
			log << "Couldn't bind socket for arp" << endl;
			close(m_arp_socket);
			m_arp_socket = -1;
			return false;
		}
		
		m_arp_read_nf = new QSocketNotifier(m_arp_socket, QSocketNotifier::Read);
		m_arp_write_nf = new QSocketNotifier(m_arp_socket, QSocketNotifier::Write);
 		m_arp_write_nf->setEnabled(!m_arp_write_buf.empty());
 		connect(m_arp_read_nf, SIGNAL(activated(int)), SLOT(arpReadNF()));
 		connect(m_arp_write_nf, SIGNAL(activated(int)), SLOT(arpWriteNF()));
		return true;
	}
	
	void ARP::stop() {
		if (m_arp_read_nf) {
			m_arp_read_nf->deleteLater();
			m_arp_read_nf = 0;
		}
		if (m_arp_write_nf) {
			m_arp_write_nf->deleteLater();
			m_arp_write_nf = 0;
		}
		if (m_arp_socket != -1) {
			close(m_arp_socket);
			m_arp_socket = -1;
		}
	}
	
	bool ARP::probeIPv4(QHostAddress &addr) {
		arp_packet_ipv4 packet;
		arp_ipv4_request(packet, m_device->getMacAddress(), QHostAddress((quint32) 0), addr);
		arpWrite(QByteArray::fromRawData((const char*) &packet, sizeof(packet)));
		return true;
	}

	void ARP::arpReadNF() {
		QByteArray data = readARPPacket(m_arp_socket);
		quint8 proto_len;
		ArpOperation op;
		if (!arp_parse(data, proto_len, op))
			return;
		switch (proto_len) {
			case 4: { // IPv4:
				nut::MacAddress sender_mac, target_mac;
				QHostAddress sender_ip, target_ip;
				if (!arp_ipv4_parse(data, sender_mac, sender_ip, target_mac, target_ip))
					return;
				switch (op) {
					case ARP_REQUEST:
						emit gotRequestIPv4(sender_mac, sender_ip, target_ip);
						break;
					case ARP_REPLY:
						emit gotReplyIPv4(sender_mac, sender_ip, target_mac, target_ip);
						break;
				}
				} break;
		}
	}
	
	void ARP::arpWriteNF() {
//		log << "writeARPSocket" << endl;
		if (!m_arp_write_buf.empty()) {
			QByteArray msgdata = m_arp_write_buf.takeFirst();
			writeARPPacket(m_arp_socket, m_device->interfaceIndex, msgdata);
		}
		m_arp_write_nf->setEnabled(!m_arp_write_buf.empty());
	}
	
	void ARP::arpWrite(const QByteArray &buf) {
		if (m_arp_write_buf.empty() && m_arp_write_nf)
			m_arp_write_nf->setEnabled(true);
		m_arp_write_buf.append(buf);
	}
}
