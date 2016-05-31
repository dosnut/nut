#ifndef _NUTS_ARP_H
#define _NUTS_ARP_H

#pragma once

#include <QObject>
#include <QPointer>
#include <QSharedData>
#include <QBasicTimer>
#include <QExplicitlySharedDataPointer>
#include <QHostAddress>
#include <QSocketNotifier>
#include <QLinkedList>
#include <QByteArray>

#include <memory>

#include <libnutcommon/macaddress.h>

#include "timecls.h"

extern "C" {
#include <net/ethernet.h>
}

namespace nuts {
	// same values as in protocol itself
	enum class ArpOperation : quint16 {
		REQUEST = 1,
		REPLY = 2,
	};

	namespace ARPConst {
		// Zeroconf consts, RFC 3927, section 9.
		int const PROBE_WAIT = 1;
		int const PROBE_NUM  = 3;
		int const PROBE_MIN  = 1;
		int const PROBE_MAX  = 3;
		int const ANNOUNCE_WAIT = 2;
		int const ANNOUNCE_NUM  = 2;
		int const ANNOUNCE_INTERVAL = 2;
		int const MAX_CONFLICTS = 10;
		int const RATE_LIMIT_INTERVAL = 60;
		int const DEFEND_INTERVAL = 10;
	}

	struct ARPPacket;
	class ARPNotifier;
	class ARP;

	class ARPWatch;
	class ARPRequest;
	class ARPProbe;
	class ARPAnnounce;

	class Device;
}

namespace nuts {
	struct ARPPacket {
		// always IPv4
		ArpOperation op{ArpOperation::REQUEST};
		libnutcommon::MacAddress sender_mac, target_mac;
		QHostAddress sender_ip, target_ip;
	};

	class ARPNotifier : public QObject, public QSharedData {
		Q_OBJECT
	private:
		explicit ARPNotifier(ARP* arp, QHostAddress const& addr);

	public:
		~ARPNotifier();

		QHostAddress address() const;

	signals:
		void receivedPacket(ARPPacket const& packet);

	private:
		friend class ARP;

		QPointer<ARP> const m_arp;
		QHostAddress const m_addr;
	};
	using ARPNotifierPointer = QExplicitlySharedDataPointer<ARPNotifier>;

	/**
		This class helps you to make arp requests and
		watch the results.

		Recommended order:
		 - connect to the signal
		 - call start
		 - probe ips
	*/
	class ARP final : public QObject {
		Q_OBJECT
	public:
		explicit ARP(Device* device);
		~ARP();

		/**
		 * @brief filter for ARP packets with the given host address as sender
		 *
		 * Ignores all packets which were sent by ourself (i.e.
		 * sender_mac == interface hardware address).
		 *
		 * You need to keep the pointer alive to make sure to receive signals from it.
		 */
		ARPNotifierPointer watchForPacketsFrom(QHostAddress const& addr);

		/**
		 * @brief filter for ARP packets with the given host address as target
		 *
		 * Ignores all packets which were sent by ourself (i.e.
		 * sender_mac == interface hardware address).
		 *
		 * You need to keep the pointer alive to make sure to receive signals from it.
		 */
		ARPNotifierPointer watchForPacketsTo(QHostAddress const& addr);

		/**
		 * @brief broadcast ARP request for target_ip
		 *
		 * sender_mac = local hardware address
		 * target_mac = 00:00:00:00:00:00
		 */
		void sendRequest(QHostAddress const& sender_ip, QHostAddress const& target_ip);

		bool start();
		void stop();

	private slots:
		void arpReadNF();
		void arpWriteNF();

	private:
		friend class ARPNotifier;

		void removeNotifier(ARPNotifier* notifier);

		void arpWrite(QByteArray const& buf);

	private: /* vars */
		Device* m_device = nullptr;
		int m_arp_socket = -1;
		std::unique_ptr<QSocketNotifier> m_arp_read_nf;
		std::unique_ptr<QSocketNotifier> m_arp_write_nf;
		QLinkedList<QByteArray> m_arp_write_buf;

		QHash<QHostAddress, ARPNotifier*> m_notifiers_from;
		QHash<QHostAddress, ARPNotifier*> m_notifiers_to;
	};

	/**
	 * only a passive component: monitors all ARP packets sent by
	 * the requested IP.
	 *
	 * This is used to detect IP address conflicts with zeroconf;
	 * with DHCP we don't care about this after the address was
	 * assigned successfully.
	 */
	class ARPWatch final : public QObject {
		Q_OBJECT
	public:
		explicit ARPWatch(ARP* arp, QHostAddress const& ip, QObject* parent = nullptr);

	signals:
		void conflict(QHostAddress const& ip, libnutcommon::MacAddress const& mac);

	private slots:
		void receivedPacket(ARPPacket const& packet);

	private:
		ARPNotifierPointer m_notifier_from;
	};

	/**
	 * @brief tries to resolve IP address to MAC address
	 *
	 * deletes itself after it finishes (either foundMac or timeout).
	 * TODO: don't delete itself
	 */
	class ARPRequest : public QObject {
		Q_OBJECT
	public:
		explicit ARPRequest(ARP* arp, QHostAddress const& senderip, QHostAddress const& ip, QObject* parent = nullptr);

	signals:
		void foundMac(libnutcommon::MacAddress const& mac, QHostAddress const& ip);
		void timeout(QHostAddress const& ip);

	protected:
		void timerEvent(QTimerEvent* event) override;

	private slots:
		void receivedPacket(ARPPacket const& packet);

	private:
		void sendRequest();
		void release();

	private: /* vars */
		QPointer<ARP> m_arp;
		ARPNotifierPointer m_notifier_from;

		QHostAddress m_senderip;
		QHostAddress m_ip;
		int m_remaining_trys = ARPConst::PROBE_NUM;
		bool m_finished = false;

		QBasicTimer m_timer;
	};

	enum class ARPProbeState {
		PROBING,
		RESERVING,
		CONFLICT,
	};

	/**
	 * @brief send ARP probes to claim a zeroconf address. keeps sending
	 * probes until explicitly deleted (or hitting a conflict).
	 *
	 * might emit conflict() even after ready() was emitted.
	 */
	class ARPProbe final : public QObject {
		Q_OBJECT
	public:
		explicit ARPProbe(ARP* arp, QHostAddress const& ip, QObject* parent = nullptr);

		ARPProbeState getState() { return m_state; }

	signals:
		void conflict(QHostAddress const& ip, libnutcommon::MacAddress const& mac);
		void ready(QHostAddress const& ip);

	protected:
		void timerEvent(QTimerEvent* event) override;

	private slots:
		/**
		 * got ARP packet from existing host "m_ip"
		 *   -> potential conflict
		 */
		void receivedPacketFrom(ARPPacket const& packet);

		/**
		 * got ARP packet to m_ip, documenting somebody is interested in it
		 *   -> potential conflict
		 */
		void receivedPacketTo(ARPPacket const& packet);

	private:
		void sendProbe();
		void handleConflict(libnutcommon::MacAddress const& mac);

	private: /* vars */
		QPointer<ARP> m_arp;
		QHostAddress m_ip;

		int m_remaining_trys = ARPConst::PROBE_NUM;
		ARPProbeState m_state = ARPProbeState::PROBING;

		QBasicTimer m_timer;

		ARPNotifierPointer m_notifier_from;
		ARPNotifierPointer m_notifier_to;
	};

	class ARPAnnounce final : public QObject {
		Q_OBJECT
	public:
		explicit ARPAnnounce(ARP* arp, QHostAddress const& ip, QObject* parent = nullptr);

	signals:
		void ready(QHostAddress const& ip);

	protected:
		void timerEvent(QTimerEvent* event) override;

	private:
		void sendAnnounce();

	private: /* vars */
		QPointer<ARP> m_arp;
		QHostAddress m_ip;
		int m_remaining_announces = ARPConst::ANNOUNCE_NUM;

		QBasicTimer m_timer;
	};
}

#endif /* _NUTS_ARP_H */
