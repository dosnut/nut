#include "arp.h"

#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>

#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <unistd.h>

#include "log.h"
#include "device.h"

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

namespace {
	struct arp_packet_base {
		ARP_BASE_STRUCT
	} __attribute__ ((__packed__));

	struct arp_packet_ipv4 {
		ARP_BASE_STRUCT
		quint8 sender_hw_addr[ETH_ALEN];
		quint32 sender_p_addr;
		quint8 target_hw_addr[ETH_ALEN];
		quint32 target_p_addr;
	} __attribute__ ((__packed__));

	bool arp_parse(QByteArray const& data, quint8& proto_len, nuts::ArpOperation& operation) {
		if (data.size() < (int) sizeof(arp_packet_base))
			return false;
		arp_packet_base const* packet = reinterpret_cast<arp_packet_base const*>(data.data());
		if (   (packet->hwtype != htons(ARPHRD_ETHER))
			|| (packet->ptype != htons(ETH_P_IP))
			|| (packet->hlen != ETH_ALEN))
			return false;
		proto_len = packet->plen;
		operation = static_cast<nuts::ArpOperation>(ntohs(packet->operation));
		return true;
	}

	template<typename Packet>
	void arp_base_request(Packet& packet, quint8 proto_len, nuts::ArpOperation operation) {
		memset(&packet, 0, sizeof(packet));
		packet.hwtype = htons(ARPHRD_ETHER);
		packet.ptype = htons(ETH_P_IP);
		packet.hlen = ETH_ALEN;
		packet.plen = proto_len;
		packet.operation = htons(static_cast<quint16>(operation));
	}

	void arp_ipv4_request(arp_packet_ipv4& packet, libnutcommon::MacAddress const& sender_mac, QHostAddress const& sender_ip, QHostAddress const& target_ip) {
		arp_base_request(packet, 4, nuts::ArpOperation::REQUEST);
		memcpy(packet.sender_hw_addr, sender_mac.data.bytes, ETH_ALEN);
		packet.sender_p_addr = htonl(sender_ip.toIPv4Address());
		packet.target_p_addr = htonl(target_ip.toIPv4Address());
	}

	bool arp_ipv4_parse(QByteArray const& data, libnutcommon::MacAddress& sender_mac, QHostAddress& sender_ip, libnutcommon::MacAddress& target_mac, QHostAddress& target_ip) {
		if (data.size() < (int) sizeof(arp_packet_ipv4))
			return false;
		arp_packet_ipv4 const* packet = reinterpret_cast<arp_packet_ipv4 const*>(data.data());
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

	void writeARPPacket(int socket, int if_index, QByteArray const& data) {
		struct sockaddr_ll sock;
		memset(&sock, 0, sizeof(sock));
		sock.sll_family = AF_PACKET;
		sock.sll_protocol = htons(ETH_P_ARP);
		sock.sll_ifindex = if_index;
		sock.sll_halen = ETH_ALEN;
		memset(sock.sll_addr, 0xff, ETH_ALEN);
		sendto(socket, data.data(), data.size(), 0, reinterpret_cast<struct sockaddr*>(&sock), sizeof(sock));
	}

	QByteArray readARPPacket(int socket) {
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
		size = recvfrom(socket, buf.data(), buf.size(), 0, reinterpret_cast<struct sockaddr*>(&sock), &slen);
		if (size < 0) {
			nuts::err << QString("readARPPacket: recvfrom(...) failed: %1").arg(strerror(errno)) << endl;
			return QByteArray();
		}
		buf.resize(size);
		return buf;
	}
}

namespace nuts {
	ARP::ARP(Device* device)
	: QObject(device), m_device(device) {
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
		if (bind(m_arp_socket, reinterpret_cast<struct sockaddr*>(&sock), sizeof(sock)) < 0) {
			log << "Couldn't bind socket for arp" << endl;
			close(m_arp_socket);
			m_arp_socket = -1;
			return false;
		}

		m_arp_read_nf = new QSocketNotifier(m_arp_socket, QSocketNotifier::Read);
		m_arp_write_nf = new QSocketNotifier(m_arp_socket, QSocketNotifier::Write);
		m_arp_write_nf->setEnabled(!m_arp_write_buf.empty());
		connect(m_arp_read_nf, &QSocketNotifier::activated, this, &ARP::arpReadNF);
		connect(m_arp_write_nf, &QSocketNotifier::activated, this, &ARP::arpWriteNF);
		return true;
	}

	void ARP::stop() {
		if (m_arp_socket != -1) {
			if (m_arp_read_nf) {
				delete m_arp_read_nf;
				m_arp_read_nf = nullptr;
			}
			if (m_arp_write_nf) {
				delete m_arp_write_nf;
				m_arp_write_nf = nullptr;
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
		for (ARPTimer* t: arp_timers) {
			t->m_arp = nullptr;
			delete t;
		}
	}

	ARPRequest* ARP::requestIPv4(QHostAddress const& source_addr, QHostAddress const& target_addr) {
		if (m_arp_socket == -1) return nullptr;
		ARPRequest* r;
		if (nullptr != (r = m_requests.value(target_addr, nullptr))) return r;
		r  = new ARPRequest(this, source_addr, target_addr);
		arpTimerAdd(r);
		m_requests.insert(target_addr, r);
		return r;
	}

	ARPProbe* ARP::probeIPv4(QHostAddress const& addr) {
		if (m_arp_socket == -1) return nullptr;
		ARPProbe* p;
		if (nullptr != (p = m_probes.value(addr, nullptr))) return p;
		p = new ARPProbe(this, addr);
		arpTimerAdd(p);
		m_probes.insert(addr, p);
		return p;
	}

	ARPAnnounce* ARP::announceIPv4(QHostAddress const& addr) {
		if (m_arp_socket == -1) return nullptr;
		ARPAnnounce* a;
		a = new ARPAnnounce(this, addr);
		arpTimerAdd(a);
		return a;
	}

	ARPWatch* ARP::watchIPv4(QHostAddress const& addr) {
		if (m_arp_socket == -1) return nullptr;
		ARPWatch* w;
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
				ARPRequest* r;
				ARPProbe* p;
				ARPWatch* w;
				if (nullptr != (r = m_requests.value(sender_ip, nullptr)))
					r->gotPacket(sender_mac);
				if (nullptr != (p = m_probes.value(sender_ip, nullptr)))
					p->gotPacket(sender_mac);
				if (nullptr != (w = m_watches.value(sender_ip, nullptr)))
					w->gotPacket(sender_mac);
				switch (op) {
					case ArpOperation::REQUEST:
						if ((sender_ip.toIPv4Address() == 0) && (sender_mac != m_device->getMacAddress())
							&& (nullptr != (p = m_probes.value(target_ip, nullptr))))
							p->gotProbe(sender_mac);
						break;
					case ArpOperation::REPLY:
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

	void ARP::arpWrite(QByteArray const& buf) {
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

	void ARP::arpTimerAdd(ARPTimer* t) {
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

	void ARP::arpTimerDelete(ARPTimer* t) {
		bool recalc = !m_arp_timers.isEmpty() && (t == m_arp_timers.first());
		m_arp_timers.removeAll(t);
		if (recalc) recalcTimer();
	}

	void ARP::timerEvent(QTimerEvent*) {
		Time now(Time::current());
		if (m_arp_timers.isEmpty()) {
			recalcTimer();
			return;
		}
		ARPTimer* t;
		while ( (!m_arp_timers.isEmpty()) && (t = m_arp_timers.first()) && (now >= t->m_nextTimeout) ) {
			if (t->timeEvent()) {
				m_arp_timers.pop_front();
				arpTimerAdd(t);
			}
		}
	}

	ARPWatch::~ARPWatch() {
		if (m_arp) m_arp->m_watches.remove(m_ip);
	}

	void ARPWatch::gotPacket(libnutcommon::MacAddress const& mac) {
		if (m_arp) m_arp->m_watches.remove(m_ip);
		m_arp = nullptr;
		emit conflict(m_ip, mac);
		deleteLater();
	}

	ARPTimer::ARPTimer(ARP* arp)
	: QObject(arp), m_arp(arp), m_nextTimeout(Time::current()) { }

	ARPTimer::ARPTimer(ARP* arp, Time const& firstTimeout)
	: QObject(arp), m_arp(arp), m_nextTimeout(firstTimeout) { }

	ARPRequest::ARPRequest(ARP* arp, QHostAddress const& sourceip, QHostAddress const& targetip)
	: ARPTimer(arp, Time()), m_sourceip(sourceip), m_targetip(targetip) {
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
			m_arp->arpWrite(QByteArray(reinterpret_cast<char const*>(&packet), sizeof(packet)));

			return true;
		}
	}

	void ARPRequest::gotPacket(libnutcommon::MacAddress const& mac) {
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

	ARPProbe::ARPProbe(ARP* arp, QHostAddress const& ip)
	: ARPTimer(arp, Time::waitRandom(0, ARPConst::PROBE_WAIT)), m_ip(ip) {
	}

	ARPProbe::~ARPProbe() {
		if (!m_arp || m_finished) return;
		finish();
	}

	void ARPProbe::setReserve(bool reserve) {
		m_reserve = reserve;
	}

	bool ARPProbe::timeEvent() {
		if (m_state == ARPProbeState::CONFLICT) return false;
		if (m_remaining_trys <= 0 && !m_reserve) {
			m_state = ARPProbeState::RESERVING;
			finish();
			emit ready(m_ip);
			return false;
		} else {
			if (m_remaining_trys <= 0) {
				if (m_state != ARPProbeState::RESERVING) {
					m_state = ARPProbeState::RESERVING;
					emit ready(m_ip);
				}
			} else {
				--m_remaining_trys;
			}
			m_nextTimeout = Time::waitRandom(ARPConst::PROBE_MIN, ARPConst::PROBE_MAX);

			arp_packet_ipv4 packet;
			arp_ipv4_request(packet, m_arp->m_device->getMacAddress(), QHostAddress((quint32) 0), m_ip);
			m_arp->arpWrite(QByteArray(reinterpret_cast<char const*>(&packet), sizeof(packet)));

			return true;
		}
	}

	void ARPProbe::gotPacket(libnutcommon::MacAddress const& mac) {
		if (mac.zero()) return;
		m_state = ARPProbeState::CONFLICT;
		finish();
		emit conflict(m_ip, mac);
	}

	void ARPProbe::gotProbe(libnutcommon::MacAddress const& mac) {
		if (mac.zero()) return;
		m_state = ARPProbeState::CONFLICT;
		finish();
		emit conflict(m_ip, mac);
	}

	void ARPProbe::finish() {
		m_finished = true;
		m_arp->m_probes.remove(m_ip);
		m_arp->arpTimerDelete(this);
		deleteLater();
	}

	ARPAnnounce::ARPAnnounce(ARP* arp, QHostAddress const& ip)
	: ARPTimer(arp), m_ip(ip) {
		timeEvent();
	}
	ARPAnnounce::~ARPAnnounce() {
		if (m_arp)
			m_arp->arpTimerDelete(this);
	}
	bool ARPAnnounce::timeEvent() {
		if (m_remaining_announces == 0) {
			m_arp->arpTimerDelete(this);
			m_arp = nullptr;
			emit ready(m_ip);
			deleteLater();
			return false;
		}
		m_remaining_announces--;

		arp_packet_ipv4 packet;
		arp_ipv4_request(packet, m_arp->m_device->getMacAddress(), m_ip, m_ip);
		m_arp->arpWrite(QByteArray(reinterpret_cast<char const*>(&packet), sizeof(packet)));

		m_nextTimeout = Time::wait(ARPConst::ANNOUNCE_INTERVAL);
		return true;
	}
}
