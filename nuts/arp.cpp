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
		memcpy(packet.sender_hw_addr, &sender_mac.data, ETH_ALEN);
		packet.sender_p_addr = htonl(sender_ip.toIPv4Address());
		packet.target_p_addr = htonl(target_ip.toIPv4Address());
	}

	bool arp_ipv4_parse(QByteArray const& data, nuts::ARPPacket* packet) {
		if (data.size() < (int) sizeof(arp_packet_ipv4)) return false;
		arp_packet_ipv4 const* raw_packet = reinterpret_cast<arp_packet_ipv4 const*>(data.data());
		if (   (raw_packet->hwtype != htons(ARPHRD_ETHER))
			|| (raw_packet->ptype != htons(ETH_P_IP))
			|| (raw_packet->hlen != ETH_ALEN)
			|| (raw_packet->plen != 4)) {
			return false;
		}
		packet->op = static_cast<nuts::ArpOperation>(ntohs(raw_packet->operation));
		packet->sender_mac = libnutcommon::MacAddress(raw_packet->sender_hw_addr);
		packet->sender_ip = QHostAddress(ntohl(raw_packet->sender_p_addr));
		packet->target_mac = libnutcommon::MacAddress(raw_packet->target_hw_addr);
		packet->target_ip = QHostAddress(ntohl(raw_packet->target_p_addr));
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
	ARPNotifier::ARPNotifier(ARP* arp, QHostAddress const& addr)
	: m_arp(arp), m_addr(addr) {
	}

	ARPNotifier::~ARPNotifier() {
		if (m_arp) m_arp->removeNotifier(this);
	}

	QHostAddress ARPNotifier::address() const {
		return m_addr;
	}

	ARP::ARP(Device* device)
	: QObject(device), m_device(device) {
	}

	ARP::~ARP() {
		stop();
	}

	ARPNotifierPointer ARP::watchForPacketsFrom(QHostAddress const& addr) {
		auto it = m_notifiers_from.find(addr);
		if (it != m_notifiers_from.end()) {
			return ARPNotifierPointer(it.value());
		} else {
			ARPNotifierPointer n{new ARPNotifier(this, addr)};
			m_notifiers_from.insert(addr, n.data());
			return n;
		}
	}

	ARPNotifierPointer ARP::watchForPacketsTo(QHostAddress const& addr) {
		auto it = m_notifiers_to.find(addr);
		if (it != m_notifiers_to.end()) {
			return ARPNotifierPointer(it.value());
		} else {
			ARPNotifierPointer n{new ARPNotifier(this, addr)};
			m_notifiers_to.insert(addr, n.data());
			return n;
		}
	}

	void ARP::sendRequest(const QHostAddress& sender_ip, const QHostAddress& target_ip) {
		arp_packet_ipv4 packet;
		arp_ipv4_request(packet, m_device->getMacAddress(), sender_ip, target_ip);
		arpWrite(QByteArray(reinterpret_cast<char const*>(&packet), sizeof(packet)));
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

		m_arp_read_nf.reset(new QSocketNotifier(m_arp_socket, QSocketNotifier::Read));
		m_arp_write_nf.reset(new QSocketNotifier(m_arp_socket, QSocketNotifier::Write));
		m_arp_write_nf->setEnabled(!m_arp_write_buf.empty());
		connect(m_arp_read_nf.get(), &QSocketNotifier::activated, this, &ARP::arpReadNF);
		connect(m_arp_write_nf.get(), &QSocketNotifier::activated, this, &ARP::arpWriteNF);
		return true;
	}

	void ARP::stop() {
		m_arp_read_nf.reset();
		m_arp_write_nf.reset();

		if (-1 != m_arp_socket) {
			close(m_arp_socket);
			m_arp_socket = -1;
		}
	}

	void ARP::arpReadNF() {
		QByteArray data = readARPPacket(m_arp_socket);
		ARPPacket packet;
		if (!arp_ipv4_parse(data, &packet))
			return;

		if (packet.sender_mac != m_device->getMacAddress()) {
			if (ARPNotifier* n = m_notifiers_from.value(packet.sender_ip, nullptr)) {
				n->receivedPacket(packet);
			}
			if (ARPNotifier* n = m_notifiers_to.value(packet.target_ip, nullptr)) {
				n->receivedPacket(packet);
			}
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

	void ARP::removeNotifier(ARPNotifier* notifier) {
		{
			auto it = m_notifiers_from.find(notifier->m_addr);
			if (it != m_notifiers_from.end() && it.value() == notifier) {
				m_notifiers_from.erase(it);
			}
		}
		{
			auto it = m_notifiers_to.find(notifier->m_addr);
			if (it != m_notifiers_to.end() && it.value() == notifier) {
				m_notifiers_to.erase(it);
			}
		}
	}

	void ARP::arpWrite(QByteArray const& buf) {
		if (m_arp_write_buf.empty() && m_arp_write_nf)
			m_arp_write_nf->setEnabled(true);
		m_arp_write_buf.append(buf);
	}

	ARPWatch::ARPWatch(ARP* arp, QHostAddress const& ip, QObject* parent)
	: QObject(parent)
	, m_notifier_from(arp->watchForPacketsFrom(ip)) {
		connect(m_notifier_from.data(), &ARPNotifier::receivedPacket, this, &ARPWatch::receivedPacket);
	}

	void ARPWatch::receivedPacket(ARPPacket const& packet) {
		QHostAddress ip = m_notifier_from->address();
		emit conflict(ip, packet.sender_mac);
	}

	ARPRequest::ARPRequest(ARP* arp, QHostAddress const& senderip, QHostAddress const& ip, QObject* parent)
	: QObject(parent)
	, m_arp(arp)
	, m_notifier_from(arp->watchForPacketsFrom(ip))
	, m_senderip(senderip)
	, m_ip(ip)
	{
		connect(m_notifier_from.data(), &ARPNotifier::receivedPacket, this, &ARPRequest::receivedPacket);
		sendRequest();
	}

	void ARPRequest::timerEvent(QTimerEvent* event) {
		if (event->timerId() == m_timer.timerId()) {
			m_timer.stop();
			sendRequest();
		}
	}

	void ARPRequest::receivedPacket(ARPPacket const& packet) {
		if (m_finished) return;
		if (packet.sender_mac.zero()) return;

		release();
		emit foundMac(packet.sender_mac, m_ip);
	}

	void ARPRequest::sendRequest() {
		if (m_finished) return;

		if (m_remaining_trys <= 0) {
			release();
			emit timeout(m_ip);
			return;
		}

		--m_remaining_trys;
		m_timer.start(Duration::randomSecs(ARPConst::PROBE_MIN, ARPConst::PROBE_MAX).msecs(), this);
		m_arp->sendRequest(m_senderip, m_ip);
	}

	void ARPRequest::release() {
		m_finished = true;
		disconnect(m_notifier_from.data(), &ARPNotifier::receivedPacket, this, &ARPRequest::receivedPacket);
		m_notifier_from.reset();
		m_timer.stop();
		deleteLater();
	}

	ARPProbe::ARPProbe(ARP* arp, QHostAddress const& ip, QObject* parent)
	: QObject(parent)
	, m_arp(arp)
	, m_ip(ip)
	, m_notifier_from(arp->watchForPacketsFrom(ip))
	, m_notifier_to(arp->watchForPacketsTo(ip)) {
		m_timer.start(Duration::randomSecs(0, ARPConst::PROBE_WAIT).msecs(), this);
		connect(m_notifier_from.data(), &ARPNotifier::receivedPacket, this, &ARPProbe::receivedPacketFrom);
		connect(m_notifier_to.data(), &ARPNotifier::receivedPacket, this, &ARPProbe::receivedPacketTo);
	}

	void ARPProbe::timerEvent(QTimerEvent* event) {
		if (event->timerId() == m_timer.timerId()) {
			m_timer.stop();
			sendProbe();
		}
	}

	void ARPProbe::sendProbe() {
		if (ARPProbeState::CONFLICT == m_state) return;

		if (ARPProbeState::RESERVING == m_state) {
			/* the address is already successfully claimed, but we're not using it yet.
			 * send more probes to keep our claim.
			 */
			m_timer.start(Duration::randomSecs(ARPConst::PROBE_MIN, ARPConst::PROBE_MAX).msecs(), this);
			m_arp->sendRequest(QHostAddress((quint32) 0), m_ip);
			return;
		}

		/* the last probe didn't lead to any conflicts - successfully claimed the address */
		if (0 >= m_remaining_trys) {
			/* start reserving (sending more probes) unless this gets deleted before.
			 * might still emit conflict() later.
			 *
			 * This is needed if we'd like to wait for DHCP timeout before actually using
			 * the zeroconf address.
			 */
			m_state = ARPProbeState::RESERVING;
			m_timer.start(0, this); // delay it a little bit.
			emit ready(m_ip);
			return;
		}

		--m_remaining_trys;
		if (0 >= m_remaining_trys) {
			/* after the last probe we need to wait ANNOUNCE_WAIT seconds */
			m_timer.start(1000 * ARPConst::ANNOUNCE_WAIT, this);
		} else {
			/* wait PROBE_MIN - PROBE_MAX seconds between probes */
			m_timer.start(Duration::randomSecs(ARPConst::PROBE_MIN, ARPConst::PROBE_MAX).msecs(), this);
		}
		// send probe packet
		m_arp->sendRequest(QHostAddress((quint32) 0), m_ip);
	}

	void ARPProbe::receivedPacketFrom(ARPPacket const& packet) {
		if (ARPProbeState::CONFLICT == m_state) return;
		handleConflict(packet.sender_mac);
	}

	void ARPProbe::receivedPacketTo(ARPPacket const& packet) {
		if (ARPProbeState::CONFLICT == m_state) return;
		// only probes lead to conflict, i.e. requests where sender ip must be 0
		if (packet.op != ArpOperation::REQUEST) return;
		if (0 != packet.sender_ip.toIPv4Address()) return;

		handleConflict(packet.target_mac);
	}

	void ARPProbe::handleConflict(libnutcommon::MacAddress const& mac) {
		disconnect(m_notifier_from.data(), &ARPNotifier::receivedPacket, this, &ARPProbe::receivedPacketFrom);
		disconnect(m_notifier_to.data(), &ARPNotifier::receivedPacket, this, &ARPProbe::receivedPacketTo);

		m_state = ARPProbeState::CONFLICT;
		m_timer.stop();

		m_notifier_from.reset();
		m_notifier_to.reset();

		emit conflict(m_ip, mac);
	}

	ARPAnnounce::ARPAnnounce(ARP* arp, QHostAddress const& ip, QObject* parent)
	: QObject(parent), m_arp(arp), m_ip(ip) {
		m_timer.start(1000 * ARPConst::ANNOUNCE_INTERVAL, this);
		sendAnnounce();
	}

	void ARPAnnounce::timerEvent(QTimerEvent* event) {
		if (event->timerId() == m_timer.timerId()) {
			sendAnnounce();
		}
	}

	void ARPAnnounce::sendAnnounce() {
		if (!m_arp) {
			m_timer.stop();
			return;
		}
		if (0 >= m_remaining_announces) {
			m_timer.stop();
			emit ready(m_ip);
			return;
		}
		--m_remaining_announces;

		// send gratitous ARP "request" telling everyone our own IP
		m_arp->sendRequest(m_ip, m_ip);
	}
}
