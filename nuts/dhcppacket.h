//
// C++ Interface: dhcppacket
//
// Description: 
//
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef _NUTS_DHCPPACKET_H
#define _NUTS_DHCPPACKET_H

#include <netinet/udp.h>
#include <netinet/ip.h>

namespace nuts {
	class DHCPPacket;
};

enum bootp_op {
	BOOT_REQUEST = 1,
	BOOT_REPLY   = 2
};

/* DHCP message types */
enum dhcp_message_type {
	DHCP_DISCOVER   = 1,
	DHCP_OFFER      = 2,
	DHCP_REQUEST    = 3,
	DHCP_DECLINE    = 4,
	DHCP_ACK        = 5,
	DHCP_NAK        = 6,
	DHCP_RELEASE    = 7,
	DHCP_INFORM     = 8
};

/* DHCP option codes (partial list) */
enum dhcp_options {
	DHCP_PADDING         = 0x00,
	DHCP_SUBNET          = 0x01,
	DHCP_TIME_OFFSET     = 0x02,
	DHCP_ROUTER          = 0x03,
	DHCP_TIME_SERVER     = 0x04,
	DHCP_NAME_SERVER     = 0x05,
	DHCP_DNS_SERVER      = 0x06,
	DHCP_LOG_SERVER      = 0x07,
	DHCP_COOKIE_SERVER   = 0x08,
	DHCP_LPR_SERVER      = 0x09,
	DHCP_HOST_NAME       = 0x0c,
	DHCP_BOOT_SIZE       = 0x0d,
	DHCP_DOMAIN_NAME     = 0x0f,
	DHCP_SWAP_SERVER     = 0x10,
	DHCP_ROOT_PATH       = 0x11,
	DHCP_IP_TTL          = 0x17,
	DHCP_MTU             = 0x1a,
	DHCP_BROADCAST       = 0x1c,
	DHCP_NTP_SERVER      = 0x2a,
	DHCP_WINS_SERVER     = 0x2c,
	DHCP_REQUESTED_IP    = 0x32,
	DHCP_LEASE_TIME      = 0x33,
	DHCP_OPTION_OVER     = 0x34,
	DHCP_MESSAGE_TYPE    = 0x35,
	DHCP_SERVER_ID       = 0x36,
	DHCP_PARAM_REQ       = 0x37,
	DHCP_MESSAGE         = 0x38,
	DHCP_MAX_SIZE        = 0x39,
	DHCP_T1              = 0x3a,
	DHCP_T2              = 0x3b,
	DHCP_VENDOR          = 0x3c,
	DHCP_CLIENT_ID       = 0x3d,
	DHCP_FQDN            = 0x51,
		
	DHCP_END             = 0xFF
};

#define DHCP_MAGIC 0x63825363

#include <QVector>
#include "device.h"

namespace nuts {
	/**
		@author Stefan Bühler <stbuehler@web.de>
	*/
	class DHCPPacket {
		public:
			struct dhcp_msg {
				quint8  op, htype, hlen, hops;
				quint32 xid;
				quint16 secs, flags;
				quint32 ciaddr, yiaddr, siaddr, giaddr;
				quint8  chaddr[16], sname[64], file[128];
				quint32 cookie;
				// 308 byte options
			} __attribute__ ((__packed__));
			struct udp_dhcp_packet {
				struct iphdr ip;
				struct udphdr udp;
			} __attribute__ ((__packed__));
			struct dhcp_msg msg;
			struct udp_dhcp_packet headers;
			QHash< quint8, QVector<quint8> > options;
			QByteArray msgdata;
			
		public:
			DHCPPacket(QDataStream &in);
			DHCPPacket(bool client);
			virtual ~DHCPPacket();
			
			static DHCPPacket* parseRaw(QByteArray &buf);
			
			bool check();
			
			void setClientMac(const MacAddress &chaddr);
			void setXID(quint32 xid);
			void setOption(quint8 op, const QVector<quint8>& data);
			void setOption(quint8 op, const quint8* data, int size);
			void setOptionString(quint8 op, const QString& s);
			void setOptionFrom(quint8 op, const DHCPPacket &p);
			template< typename T >
			inline void setOptionData(quint8 op, const T& data) {
				setOption(op, (quint8*) &data, sizeof(data));
			}
			
			MacAddress getClientMac();
			inline quint32 getXID() {
				return msg.xid;
			}
			inline QVector<quint8> getOption(quint8 op) {
				return options.value(op);
			}
			inline QString getOptionString(quint8 op) {
				const QVector<quint8> &buf(getOption(op));
				const char *s = (char*) buf.data();
				return QString::fromUtf8(s, qstrnlen(s, buf.size()));
			}
			template< typename T >
			inline const T& getOptionData(quint8 op, const T& def) {
				const QVector<quint8> &buf(getOption(op));
				if (buf.size() >= (signed int) sizeof(T)) {
					const T *data = (const T *) buf.constData();
					return *data;
				}
				return def;
			}
			
			inline void setBootOp(enum bootp_op bop) {
				msg.op = bop;
			}
			inline enum bootp_op getBootOp() {
				return (enum bootp_op) msg.op;
			}
			
			inline void setMessageType(enum dhcp_message_type msgt) {
				setOptionData(DHCP_MESSAGE_TYPE, (quint8) msgt);
			}
			inline enum dhcp_message_type getMessageType() {
				return (enum dhcp_message_type) getOptionData<quint8>(DHCP_MESSAGE_TYPE, -1);
			}
			
			// id is htonl(IPv4)
			inline void setDHCPServerID(quint32 id) {
				setOptionData(DHCP_SERVER_ID, id);
			}
			inline quint32 getDHCPServerID() {
				return getOptionData<quint32>(DHCP_SERVER_ID, -1);
			}
			
			inline void send(Interface_IPv4 *iface) {
				iface->env->device->sendDHCPClientPacket(this);
			}
	};
	
	class DHCPClientPacket : public DHCPPacket {
			Interface_IPv4 *iface;
		
		public:
			DHCPClientPacket(Interface_IPv4 *iface);
			
			void doDHCPDiscover();
			void doDHCPRequest(const DHCPPacket &reply);
			
			void requestIP(const QHostAddress &ip);
			void send();
	};
}

#endif
