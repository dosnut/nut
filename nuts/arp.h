//
// C++ Interface: arp
//
// Description: 
//
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef NUTSARP_H
#define NUTSARP_H

#include <QObject>
#include <QHostAddress>
#include <QSocketNotifier>
#include <QLinkedList>
#include <QByteArray>

#include <common/macaddress.h>

namespace nuts {
	class ARP;
	class ARPTimer;
	class ARPRequest;
	class ARPProbe;
	
	class Device;
}

namespace nuts {
	typedef struct tv {
		time_t sec;
		suseconds_t usec;
		tv(time_t sec = 0, suseconds_t usec = 0)
		: sec(sec), usec(usec) { }
	} tv;
	
	class Time {
		private:
			tv m_tv;
			
			void fix() {
				while (m_tv.usec < 0) { m_tv.sec--; m_tv.usec += 1000000; }
				while (m_tv.usec > 1000000) { m_tv.sec++; m_tv.usec -= 1000000; }
			}
		
			static Time fix(time_t sec, suseconds_t usec) {
				while (usec < 0) { sec--; usec += 1000000; }
				while (usec > 1000000) { sec++; usec -= 1000000; }
				return Time(sec, usec);
			}
			
		public:
			Time(time_t sec = 0, suseconds_t usec = 0)
			: m_tv(sec, usec) { }
			
			static Time current();
			static Time random(int min, int max);
			static Time waitRandom(int min, int max);
			static Time wait(time_t sec = 0, suseconds_t usec = 0);
			Time operator +(const Time &a) const {
				return Time::fix(m_tv.sec + a.m_tv.sec, m_tv.usec + a.m_tv.usec);
			}
			Time operator -(const Time &a) const {
				return Time::fix(m_tv.sec - a.m_tv.sec, m_tv.usec - a.m_tv.usec);
			}
			Time& operator += (const Time &a) {
				m_tv.sec += a.m_tv.sec; m_tv.usec += a.m_tv.usec;
				fix();
				return *this;
			}
			Time& operator -= (const Time &a) {
				m_tv.sec -= a.m_tv.sec; m_tv.usec -= a.m_tv.usec;
				fix();
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
			
			int msecs() {
				return 1000*m_tv.sec + (m_tv.usec+999) / 1000;
			}
			
			QString toString() const {
				return QString("%1.%2").arg(m_tv.sec).arg(m_tv.usec, 6, 10, QLatin1Char('0'));
			}
	};
	
	// same values as in protocol itself
	typedef enum ArpOperation {
		ARP_REQUEST = 1,
		ARP_REPLY = 2,
	} ArpOperation;
	
	namespace ARPConst {
		// Zeroconf consts, RFC 3927, section 9.
		const int PROBE_WAIT = 1;
		const int PROBE_NUM  = 3;
		const int PROBE_MIN  = 1;
		const int PROBE_MAX  = 3;
		const int ANNOUNCE_WAIT = 2;
		const int ANNOUNCE_NUM  = 2;
		const int ANNOUNCE_INTERVAL = 2;
		const int MAX_CONFLICTS = 10;
		const int RATE_LIMIT_INTERVAL = 60;
		const int DEFEND_INTERVAL = 10;
	};
	
	/**
		@author Stefan Bühler <stbuehler@web.de>
		
		This class helps you to make arp requests and
		watch the results.
		
		Recommended order:
		 - connect to the signal
		 - call start
		 - probe ips
	*/
	class ARP : public QObject {
		Q_OBJECT
		private:
			Device *m_device;
			int m_arp_socket;
			QSocketNotifier *m_arp_read_nf, *m_arp_write_nf;
			QLinkedList< QByteArray > m_arp_write_buf;
			
			int m_timer_id;
			QLinkedList<ARPTimer*> m_arp_timers;
			
			QHash<QHostAddress, ARPProbe*> m_probes;
			QHash<QHostAddress, ARPRequest*> m_requests;
			
		public:
			ARP(Device* device);
			virtual ~ARP();

			/**
				Prepare a request for an IPv4 address.
				source_mac is set to the device mac, target_mac = 0.
			*/
			ARPRequest* requestIPv4(const QHostAddress &source_addr, const QHostAddress &target_addr);
			
			/**
				Prepare a probe for an IPv4 address. (Needed for zeroconf)
			*/
			ARPProbe* probeIPv4(const QHostAddress &addr);

		private slots:
			void arpReadNF();
			void arpWriteNF();
			
		private:
			friend class ARPRequest;
			friend class ARPProbe;
			
			void arpWrite(const QByteArray &buf);
			
			void recalcTimer();
			void arpTimerAdd(ARPTimer *t);
			void arpTimerDelete(ARPTimer *t);
			
			void timerEvent(QTimerEvent *event);
			
			friend class Device;
			bool start();
			void stop();
	};

	class ARPTimer : public QObject {
		Q_OBJECT
		protected:
			friend class ARP;
			
			ARPTimer(ARP *arp);
			ARPTimer(ARP *arp, const Time &firstTimeout);
			
			ARP *m_arp;
			Time m_nextTimeout;
			
			virtual bool timeEvent() = 0;
			
			bool operator <(const ARPTimer &t) const {
				return m_nextTimeout < t.m_nextTimeout;
			}
	};
	
	class ARPRequest : public ARPTimer {
		Q_OBJECT
		public:
			virtual ~ARPRequest();
		protected:
			friend class ARP;
			
			QHostAddress m_sourceip, m_targetip;
			int m_remaining_trys;
			bool m_finished;
			
			ARPRequest(ARP *arp, const QHostAddress &sourceip, const QHostAddress &targetip);
			
			virtual bool timeEvent();
			// got ARP Packet which resolves m_targetip to mac
			void gotPacket(const nut::MacAddress &mac);
			
		private:
			void finish();
			
		signals:
			void foundMac(nut::MacAddress mac, QHostAddress ip);
			void timeout(QHostAddress ip);
	};
	
	class ARPProbe : public ARPTimer {
		Q_OBJECT
		public:
			typedef enum ARPProbeState { PROBING, RESERVING, CONFLICT } ARPProbeState;
			
			virtual ~ARPProbe();
			void setReserve(bool reserve);
			ARPProbeState getState() { return m_state; }
			
		protected:
			friend class ARP;
			
			QHostAddress m_ip;
			int m_remaining_trys;
			bool m_finished, m_reserve;
			ARPProbeState m_state;
			
			ARPProbe(ARP *arp, const QHostAddress &ip);
			
			// got ARP Packet which resolves m_ip to mac
			void gotPacket(const nut::MacAddress &mac);
			// got ARP Probe from mac which probes for m_ip
			void gotProbe(const nut::MacAddress &mac);
			
			virtual bool timeEvent();
			
		private:
			void finish();
			
		signals:
			void conflict(QHostAddress ip, nut::MacAddress mac);
			void timeout(QHostAddress ip);
	};
}

#endif
