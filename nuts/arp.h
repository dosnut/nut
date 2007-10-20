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
	
	class Device;
}

namespace nuts {
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
	
	class ARPRequest : public QObject {
		Q_OBJECT
		signals:
			void foundMac(QHostAddress ip, nut::MacAddress mac);
			void timeout(QHostAddress ip);
	};
	
	class ARPProbe : public QObject {
		Q_OBJECT
		protected:
			friend class ARP;
			
			QHostAddress m_ip;
			float nextTime, retry;
			
			ARPProbe(const QHostAddress &ip)
			: m_ip(ip), retry(0) {
			}
			
		signals:
			void foundMac(QHostAddress ip, nut::MacAddress mac);
			void timeout(QHostAddress ip);
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
			
		public:
			ARP(Device* device);
			virtual ~ARP();

			/**
				Opens the socket and in case of incoming ARP packets emit signal.
			*/
			bool start();
			
			/**
				Closes the socket; stops emmiting signals.
			*/
			void stop();
			
			/**
				Prepare a request for an IPv4 address.
				source_mac is set to the device mac, target_mac = 0.
				
				The packet is sent asynchronously, i.e. it needs the event loop to be executed.
			*/
			bool requestIPv4(QHostAddress &source_addr, QHostAddress &target_addr);
			
			/**
				Prepare a probe for an IPv4 address. (Needed for zeroconf, same as requestIPv4(0, addr))
				
				The packet is sent asynchronously, i.e. it needs the event loop to be executed.
			*/
			bool probeIPv4(QHostAddress &addr);

		signals:
			// SIGNAL(gotRequestIPv4(nut::MacAddress, QHostAddress, QHostAddress))
			void gotRequestIPv4(nut::MacAddress sender_mac, QHostAddress sender_ip, QHostAddress target_ip);
			// SIGNAL(gotReplyIPv4(nut::MacAddress, QHostAddress, nut::MacAddress, QHostAddress))
			void gotReplyIPv4(nut::MacAddress sender_mac, QHostAddress sender_ip, nut::MacAddress target_mac, QHostAddress target_ip);
			
		private slots:
			void arpReadNF();
			void arpWriteNF();
		private:
			void arpWrite(const QByteArray &buf);
	};
}

#endif
