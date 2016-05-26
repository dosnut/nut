#ifndef _NUTS_ARP_H
#define _NUTS_ARP_H

#pragma once

#include <QObject>
#include <QHostAddress>
#include <QSocketNotifier>
#include <QLinkedList>
#include <QByteArray>

#include <libnutcommon/macaddress.h>

#include "timecls.h"

extern "C" {
#include <net/ethernet.h>
}

namespace nuts {
	class ARP;
	class ARPWatch;
	class ARPTimer;
	class ARPRequest;
	class ARPProbe;
	class ARPAnnounce;

	class Device;
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

	/**
		@author Stefan Bühler <stbuehler@web.de>

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
			Prepare a request for an IPv4 address.
			source_mac is set to the device mac, target_mac = 0.
		*/
		ARPRequest* requestIPv4(QHostAddress const& source_addr, QHostAddress const& target_addr);

		/**
			Prepare a probe for an IPv4 address. (Needed for zeroconf)
		*/
		ARPProbe* probeIPv4(QHostAddress const& addr);

		ARPAnnounce* announceIPv4(QHostAddress const& addr);

		ARPWatch* watchIPv4(QHostAddress const& addr);

	private slots:
		void arpReadNF();
		void arpWriteNF();

	private:
		friend class ARPRequest;
		friend class ARPProbe;
		friend class ARPAnnounce;
		friend class ARPWatch;

		void arpWrite(QByteArray const& buf);

		void recalcTimer();
		void arpTimerAdd(ARPTimer* t);
		void arpTimerDelete(ARPTimer* t);

		void timerEvent(QTimerEvent* event) override;

		friend class Device;
		bool start();
		void stop();

	private: /* vars */
		Device* m_device = nullptr;
		int m_arp_socket = -1;
		QSocketNotifier* m_arp_read_nf = nullptr;
		QSocketNotifier* m_arp_write_nf = nullptr;
		QLinkedList<QByteArray> m_arp_write_buf;

		int m_timer_id = 0;
		QLinkedList<ARPTimer*> m_arp_timers;

		QHash<QHostAddress, ARPProbe*> m_probes;
		QHash<QHostAddress, ARPRequest*> m_requests;
		QHash<QHostAddress, ARPWatch*> m_watches;
	};

	class ARPWatch final : public QObject {
		Q_OBJECT
	protected:
		friend class ARP;

		explicit ARPWatch(ARP* arp, QHostAddress const& ip)
		: m_arp(arp), m_ip(ip) { }

		// got ARP Packet which resolves watched ip to mac
		void gotPacket(libnutcommon::MacAddress const& mac);

	public:
		~ARPWatch();

	signals:
		void conflict(QHostAddress ip, libnutcommon::MacAddress mac);

	protected: /* vars */
		ARP* m_arp = nullptr;
		QHostAddress m_ip;
	};

	class ARPTimer : public QObject {
		Q_OBJECT
	protected:
		friend class ARP;

		explicit ARPTimer(ARP* arp);
		explicit ARPTimer(ARP* arp, Time const& firstTimeout);

		virtual bool timeEvent() = 0;

		bool operator<(ARPTimer const& t) const {
			return m_nextTimeout < t.m_nextTimeout;
		}

	protected: /* vars */
		ARP* m_arp = nullptr;
		Time m_nextTimeout;
	};

	class ARPRequest final : public ARPTimer {
		Q_OBJECT
	public:
		~ARPRequest();

	private:
		friend class ARP;

		explicit ARPRequest(ARP* arp, QHostAddress const& sourceip, QHostAddress const& targetip);

		bool timeEvent() override;

		// got ARP Packet which resolves m_targetip to mac
		void gotPacket(libnutcommon::MacAddress const& mac);

		void finish();

	signals:
		void foundMac(libnutcommon::MacAddress mac, QHostAddress ip);
		void timeout(QHostAddress ip);

	private: /* vars */
		QHostAddress m_sourceip, m_targetip;
		int m_remaining_trys = ARPConst::PROBE_NUM;
		bool m_finished = false;
	};

	enum class ARPProbeState {
		PROBING,
		RESERVING,
		CONFLICT,
	};

	class ARPProbe final : public ARPTimer {
		Q_OBJECT
	public:
		~ARPProbe();
		void setReserve(bool reserve);
		bool getReserve() { return m_reserve; }
		ARPProbeState getState() { return m_state; }

	private:
		friend class ARP;

		explicit ARPProbe(ARP *arp, QHostAddress const& ip);

		// got ARP Packet which resolves m_ip to mac
		void gotPacket(libnutcommon::MacAddress const& mac);
		// got ARP Probe from mac which probes for m_ip
		void gotProbe(libnutcommon::MacAddress const& mac);

		bool timeEvent() override;

		void finish();

	signals:
		void conflict(QHostAddress ip, libnutcommon::MacAddress mac);
		void ready(QHostAddress ip);

	private: /* vars */
		QHostAddress m_ip;
		int m_remaining_trys = ARPConst::PROBE_NUM;
		bool m_finished = false;
		bool m_reserve = false;
		ARPProbeState m_state = ARPProbeState::PROBING;
	};

	class ARPAnnounce final : public ARPTimer {
		Q_OBJECT
	public:
		~ARPAnnounce();

	private:
		friend class ARP;

		explicit ARPAnnounce(ARP* arp, QHostAddress const& ip);

		bool timeEvent() override;

	signals:
		void ready(QHostAddress ip);

	private: /* vars */
		QHostAddress m_ip;
		int m_remaining_announces = ARPConst::ANNOUNCE_NUM;
	};
}

#endif /* _NUTS_ARP_H */
