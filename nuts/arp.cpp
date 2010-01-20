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
#include "random.h"

namespace nuts {
	Time Time::current() {
		struct timeval tv = { 0, 0 };
		gettimeofday(&tv, 0);
		return Time(tv.tv_sec, tv.tv_usec);
	}
	
	Time Time::random(int min, int max) {
		float range = (max - min);
		float r = min + range * ((float) getRandomUInt32()) / ((float) (quint32) -1);
		int sec = (int) r;
		r -= sec; r *= 1000000;
		int usec = (int) r;
		return Time(sec, usec);
	}

	Time Time::waitRandom(int min, int max) {
		return random(min, max) + current();
	}
	
	Time Time::wait(time_t sec, suseconds_t usec) {
		return current() + Time(sec, usec);
	}
}

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
} __attribute__ ((__packed__));

typedef struct arp_packet_ipv4 arp_packet_ipv4;
struct arp_packet_ipv4 {
	ARP_BASE_STRUCT
	quint8 sender_hw_addr[ETH_ALEN];
	quint32 sender_p_addr;
	quint8 target_hw_addr[ETH_ALEN];
	quint32 target_p_addr;
} __attribute__ ((__packed__));

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
	memset(&packet, 0, sizeof(packet));
	packet.hwtype = htons(ARPHRD_ETHER);
	packet.ptype = htons(ETH_P_IP);
	packet.hlen = ETH_ALEN;
	packet.plen = proto_len;
	packet.operation = htons((quint16) operation);
}

static void arp_ipv4_request(arp_packet_ipv4 &packet, const libnutcommon::MacAddress &sender_mac, const QHostAddress &sender_ip, const QHostAddress &target_ip) {
	arp_base_request(packet, 4, nuts::ARP_REQUEST);
	memcpy(packet.sender_hw_addr, sender_mac.data.bytes, ETH_ALEN);
	packet.sender_p_addr = htonl(sender_ip.toIPv4Address());
	packet.target_p_addr = htonl(target_ip.toIPv4Address());
}

static bool arp_ipv4_parse(const QByteArray &data, libnutcommon::MacAddress &sender_mac, QHostAddress &sender_ip, libnutcommon::MacAddress &target_mac, QHostAddress &target_ip) {
	if (data.size() < (int) sizeof(arp_packet_ipv4))
		return false;
	arp_packet_ipv4 *packet = (arp_packet_ipv4*) data.data();
	if (   (packet->hwtype != htons(ARPHRD_ETHER))
		|| (packet->ptype != htons(ETH_P_IP))
		|| (packet->hlen != ETH_ALEN)
		|| (packet->plen != 4))
		return false;
	sender_mac = libnutcommon::MacAddress(packet->sender_hw_addr);
	sender_ip = QHostAddress(ntohl(packet->sender_p_addr));
	target_mac = libnutcommon::MacAddress(packet->target_hw_addr);
	target_ip = QHostAddress(ntohl(packet->target_p_addr));
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
	ARPTimer::ARPTimer(ARP *arp)
	: QObject(arp), m_arp(arp), m_nextTimeout(Time::current()) { }
	
	ARPTimer::ARPTimer(ARP *arp, const Time &firstTimeout)
	: QObject(arp), m_arp(arp), m_nextTimeout(firstTimeout) { }
	
	ARPRequest::ARPRequest(ARP *arp, const QHostAddress &sourceip, const QHostAddress &targetip)
	: ARPTimer(arp, Time()), m_sourceip(sourceip), m_targetip(targetip), m_remaining_trys(ARPConst::PROBE_NUM), m_finished(false) {
		timeEvent();
	}
	
	ARPRequest::~ARPRequest() {
		if (!m_arp || m_finished) return;
		finish();
	}
	
	bool ARPRequest::timeEvent() {
		if (m_finished) return false;
		if (m_remaining_trys <= 0) {
			finish();
			emit timeout(m_targetip);
			return false;
		} else {
			--m_remaining_trys;
			m_nextTimeout = Time::waitRandom(ARPConst::PROBE_MIN, ARPConst::PROBE_MAX);
			
			arp_packet_ipv4 packet;
			arp_ipv4_request(packet, m_arp->m_device->getMacAddress(), m_sourceip, m_targetip);
			m_arp->arpWrite(QByteArray((const char*) &packet, sizeof(packet)));
			
			return true;
		}
	}
	
	void ARPRequest::gotPacket(const libnutcommon::MacAddress &mac) {
		if (mac.zero()) return;
		finish();
		emit foundMac(mac, m_targetip);
	}
	
	void ARPRequest::finish() {
		m_finished = true;
		m_arp->m_requests.remove(m_targetip);
		m_arp->arpTimerDelete(this);
		deleteLater();
	}
	
	ARPProbe::ARPProbe(ARP *arp, const QHostAddress &ip)
	: ARPTimer(arp, Time::waitRandom(0, ARPConst::PROBE_WAIT)), m_ip(ip), m_remaining_trys(ARPConst::PROBE_NUM),
	  m_finished(false), m_reserve(false), m_state(PROBING) {
	}
	
	ARPProbe::~ARPProbe() {
		if (!m_arp || m_finished) return;
		finish();
	}
	
	void ARPProbe::setReserve(bool reserve) {
		m_reserve = reserve;
	}
	
	bool ARPProbe::timeEvent() {
		if (m_state == CONFLICT) return false;
		if (m_remaining_trys <= 0 && !m_reserve) {
			m_state = RESERVING;
			finish();
			emit ready(m_ip);
			return false;
		} else {
			if (m_remaining_trys <= 0) {
				if (m_state != RESERVING) {
					m_state = RESERVING;
					emit ready(m_ip);
				}
			} else {
				--m_remaining_trys;
			}
			m_nextTimeout = Time::waitRandom(ARPConst::PROBE_MIN, ARPConst::PROBE_MAX);
			
			arp_packet_ipv4 packet;
			arp_ipv4_request(packet, m_arp->m_device->getMacAddress(), QHostAddress((quint32) 0), m_ip);
			m_arp->arpWrite(QByteArray((const char*) &packet, sizeof(packet)));
			
			return true;
		}
	}
	
	void ARPProbe::gotPacket(const libnutcommon::MacAddress &mac) {
		if (mac.zero()) return;
		m_state = CONFLICT;
		finish();
		emit conflict(m_ip, mac);
	}
	
	void ARPProbe::gotProbe(const libnutcommon::MacAddress &mac) {
		if (mac.zero()) return;
		m_state = CONFLICT;
		finish();
		emit conflict(m_ip, mac);
	}
	
	void ARPProbe::finish() {
		m_finished = true;
		m_arp->m_probes.remove(m_ip);
		m_arp->arpTimerDelete(this);
		deleteLater();
	}
	
	ARPAnnounce::ARPAnnounce(ARP *arp, const QHostAddress &ip)
	: ARPTimer(arp), m_ip(ip), m_remaining_announces(ARPConst::ANNOUNCE_NUM) {
		timeEvent();
	}
	ARPAnnounce::~ARPAnnounce() {
		if (m_arp)
			m_arp->arpTimerDelete(this);
	}
	bool ARPAnnounce::timeEvent() {
		if (m_remaining_announces == 0) {
			m_arp->arpTimerDelete(this);
			m_arp = 0;
			emit ready(m_ip);
			deleteLater();
			return false;
		}
		m_remaining_announces--;
		
		arp_packet_ipv4 packet;
		arp_ipv4_request(packet, m_arp->m_device->getMacAddress(), m_ip, m_ip);
		m_arp->arpWrite(QByteArray((const char*) &packet, sizeof(packet)));
		
		m_nextTimeout = Time::wait(ARPConst::ANNOUNCE_INTERVAL);
		return true;
	}
	
	ARPWatch::~ARPWatch() {
		if (m_arp) m_arp->m_watches.remove(m_ip);
	}
	
	void ARPWatch::gotPacket(const libnutcommon::MacAddress &mac) {
		if (m_arp) m_arp->m_watches.remove(m_ip);
		m_arp = 0;
		emit conflict(m_ip, mac);
		deleteLater();
	}
	
	ARP::ARP(Device* device)
	: QObject(device), m_device(device), m_arp_socket(-1), m_timer_id(0) {
	}

	ARP::~ARP() {
		stop();
	}
	
	bool ARP::start() {
		if (m_arp_socket != -1) return true;
		int if_index = m_device->m_interfaceIndex;
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
		if (m_arp_socket != -1) {
			if (m_arp_read_nf) {
				delete m_arp_read_nf;
				m_arp_read_nf = 0;
			}
			if (m_arp_write_nf) {
				delete m_arp_write_nf;
				m_arp_write_nf = 0;
			}
			close(m_arp_socket);
			m_arp_socket = -1;
		}
		if (m_timer_id) {
			killTimer(m_timer_id);
			m_timer_id = 0;
		}
		m_probes.clear(); m_requests.clear();
		QLinkedList<ARPTimer*> arp_timers(m_arp_timers);
		m_arp_timers.clear();
		foreach (ARPTimer *t, arp_timers) {
			t->m_arp = 0;
			delete t;
		}
	}
	
	ARPRequest *ARP::requestIPv4(const QHostAddress &source_addr, const QHostAddress &target_addr) {
		if (m_arp_socket == -1) return 0;
		ARPRequest *r;
		if (0 != (r = m_requests.value(target_addr, 0))) return r;
		r  = new ARPRequest(this, source_addr, target_addr);
		arpTimerAdd(r);
		m_requests.insert(target_addr, r);
		return r;
	}

	ARPProbe *ARP::probeIPv4(const QHostAddress &addr) {
		if (m_arp_socket == -1) return 0;
		ARPProbe *p;
		if (0 != (p = m_probes.value(addr, 0))) return p;
		p = new ARPProbe(this, addr);
		arpTimerAdd(p);
		m_probes.insert(addr, p);
		return p;
	}
	
	ARPAnnounce* ARP::announceIPv4(const QHostAddress &addr) {
		if (m_arp_socket == -1) return 0;
		ARPAnnounce *a;
		a = new ARPAnnounce(this, addr);
		arpTimerAdd(a);
		return a;
	}
	
	ARPWatch* ARP::watchIPv4(const QHostAddress &addr) {
		if (m_arp_socket == -1) return 0;
		ARPWatch *w;
		w = new ARPWatch(this, addr);
		m_watches.insert(addr, w);
		return w;
	}
	
	void ARP::arpReadNF() {
		QByteArray data = readARPPacket(m_arp_socket);
		quint8 proto_len;
		ArpOperation op;
		if (!arp_parse(data, proto_len, op))
			return;
		switch (proto_len) {
			case 4: { // IPv4:
				libnutcommon::MacAddress sender_mac, target_mac;
				QHostAddress sender_ip, target_ip;
				if (!arp_ipv4_parse(data, sender_mac, sender_ip, target_mac, target_ip))
					return;
				ARPRequest *r;
				ARPProbe *p;
				ARPWatch *w;
				if (0 != (r = m_requests.value(sender_ip, 0)))
					r->gotPacket(sender_mac);
				if (0 != (p = m_probes.value(sender_ip, 0)))
					p->gotPacket(sender_mac);
				if (0 != (w = m_watches.value(sender_ip, 0)))
					w->gotPacket(sender_mac);
				switch (op) {
					case ARP_REQUEST:
						if ((sender_ip.toIPv4Address() == 0) && (sender_mac != m_device->getMacAddress())
							&& (0 != (p = m_probes.value(target_ip, 0))))
							p->gotProbe(sender_mac);
						break;
					case ARP_REPLY:
						break;
				}
				} break;
		}
	}
	
	void ARP::arpWriteNF() {
//		log << "writeARPSocket" << endl;
		if (!m_arp_write_buf.empty()) {
			QByteArray msgdata = m_arp_write_buf.takeFirst();
			writeARPPacket(m_arp_socket, m_device->m_interfaceIndex, msgdata);
		}
		m_arp_write_nf->setEnabled(!m_arp_write_buf.empty());
	}
	
	void ARP::arpWrite(const QByteArray &buf) {
		if (m_arp_write_buf.empty() && m_arp_write_nf)
			m_arp_write_nf->setEnabled(true);
		m_arp_write_buf.append(buf);
	}
	
	void ARP::recalcTimer() {
		if (m_timer_id) {
			killTimer(m_timer_id);
			m_timer_id = 0;
		}
		if (!m_arp_timers.isEmpty()) {
			Time dt = m_arp_timers.first()->m_nextTimeout - Time::current();
			int interval = qMax(250, dt.msecs());	// wait at least 250ms
//			qDebug() << QString("recalc Timer: %1 (%2) ms").arg(interval).arg(dt.toString()) << endl;
			m_timer_id = startTimer(interval);
		}
	}
	
	void ARP::arpTimerAdd(ARPTimer *t) {
		if (m_arp_timers.isEmpty() || t->m_nextTimeout < m_arp_timers.first()->m_nextTimeout) {
			m_arp_timers.push_front(t);
			recalcTimer();
		} else {
			QLinkedList<ARPTimer*>::iterator i = m_arp_timers.begin();
			while (i != m_arp_timers.end()) {
				if ((*i)->m_nextTimeout > t->m_nextTimeout) {
					m_arp_timers.insert(i, t);
					return;
				}
				++i;
			}
			m_arp_timers.push_back(t);
		}
	}
	
	void ARP::arpTimerDelete(ARPTimer *t) {
		bool recalc = !m_arp_timers.isEmpty() && (t == m_arp_timers.first());
		m_arp_timers.removeAll(t);
		if (recalc) recalcTimer();
	}
	
	void ARP::timerEvent(QTimerEvent *) {
		Time now(Time::current());
		if (m_arp_timers.isEmpty()) {
			recalcTimer();
			return;
		}
		ARPTimer *t;
		while ( (!m_arp_timers.isEmpty()) && (t = m_arp_timers.first()) && (now >= t->m_nextTimeout) ) {
			if (t->timeEvent()) {
				m_arp_timers.pop_front();
				arpTimerAdd(t);
			}
		}
	}
}
