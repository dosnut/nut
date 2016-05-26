#ifndef _NUTS_DHCPPACKET_H
#define _NUTS_DHCPPACKET_H

#pragma once

#include <QVector>
#include <map>

#include <netinet/udp.h>
#include <netinet/ip.h>

namespace nuts {
	class DHCPPacket;

	enum class bootp_op : quint8 {
		REQUEST = 1,
		REPLY   = 2,
	};

	/* DHCP message types */
	enum class dhcp_message_type : quint8 {
		DISCOVER   = 1,
		OFFER      = 2,
		REQUEST    = 3,
		DECLINE    = 4,
		ACK        = 5,
		NAK        = 6,
		RELEASE    = 7,
		INFORM     = 8,
	};

	/* DHCP option codes (partial list) */
	enum class dhcp_option : quint8 {
		PADDING         = 0x00,
		SUBNET          = 0x01,
		TIME_OFFSET     = 0x02,
		ROUTER          = 0x03,
		TIME_SERVER     = 0x04,
		NAME_SERVER     = 0x05,
		DNS_SERVER      = 0x06,
		LOG_SERVER      = 0x07,
		COOKIE_SERVER   = 0x08,
		LPR_SERVER      = 0x09,
		HOST_NAME       = 0x0c,
		BOOT_SIZE       = 0x0d,
		DOMAIN_NAME     = 0x0f,
		SWAP_SERVER     = 0x10,
		ROOT_PATH       = 0x11,
		DHCP_IP_TTL     = 0x17,
		MTU             = 0x1a,
		BROADCAST       = 0x1c,
		NTP_SERVER      = 0x2a,
		WINS_SERVER     = 0x2c,
		REQUESTED_IP    = 0x32,
		LEASE_TIME      = 0x33,
		OPTION_OVER     = 0x34,
		MESSAGE_TYPE    = 0x35,
		SERVER_ID       = 0x36,
		PARAM_REQ       = 0x37,
		MESSAGE         = 0x38,
		MAX_SIZE        = 0x39,
		T1              = 0x3a,
		T2              = 0x3b,
		VENDOR          = 0x3c,
		CLIENT_ID       = 0x3d,
		FQDN            = 0x51,

		END             = 0xFF,
	};

	quint32 const DHCP_MAGIC = 0x63825363u;
}

#include "device.h"

namespace nuts {
	class DHCPPacket {
	public:
		explicit DHCPPacket(QDataStream& in, quint32 from_ip /* network order */);
		explicit DHCPPacket(bool client);
		explicit DHCPPacket(bool client, QHostAddress const& unicast_addr);

		static DHCPPacket* parseRaw(QByteArray const& buf);
		static DHCPPacket* parseData(QByteArray const& buf, struct sockaddr_in const& from);

		bool check();

		void setClientMac(libnutcommon::MacAddress const& chaddr);
		void setClientAddress(QHostAddress const& addr);
		void setXID(quint32 xid);
		void setOption(dhcp_option op, QVector<quint8> const& data);
		void setOption(dhcp_option op, quint8 const* data, int size);
		void setOptionString(dhcp_option op, QString const& s);
		void setOptionFrom(dhcp_option op, DHCPPacket const& p);
		template<typename T>
		inline void setOptionData(dhcp_option op, T const& data) {
			setOption(op, reinterpret_cast<quint8 const*>(&data), sizeof(data));
		}

		libnutcommon::MacAddress getClientMac();
		inline quint32 getXID() {
			return msg.xid;
		}
		inline QVector<quint8> getOption(dhcp_option op) const {
			auto it = options.find(op);
			return it != options.end() ? it->second : QVector<quint8>();
		}
		inline QString getOptionString(dhcp_option op) const {
			if (0 == options.count(op)) return QString();
			QVector<quint8> const& buf(getOption(op));
			char const* s = reinterpret_cast<char const*>(buf.data());
			return QString::fromUtf8(s, qstrnlen(s, buf.size()));
		}
		template< typename T >
		inline const T& getOptionData(dhcp_option op, T const& def) const {
			QVector<quint8> const buf = getOption(op);
			if (buf.size() >= (signed int) sizeof(T)) {
				T const* data = reinterpret_cast<T const*>(buf.constData());
				return *data;
			}
			return def;
		}

		inline QHostAddress getOptionAddress(dhcp_option op) const {
			if (options.count(op) > 0)
				return QHostAddress(ntohl(getOptionData<quint32>(op,0)));
			return QHostAddress();
		}

		inline QList<QHostAddress> getOptionAddresses(dhcp_option op) const {
			QList<QHostAddress> l;
			QVector<quint8> const data = getOption(op);
			quint32 const* d = reinterpret_cast<quint32 const*>(data.constData());
			int size = data.size() / sizeof(*d);
			for (int i = 0; i < size; i++) {
				l.push_back(QHostAddress(ntohl(d[i])));
			}
			return l;
		}

		inline void setBootOp(bootp_op bop) {
			msg.op = bop;
		}
		inline bootp_op getBootOp() {
			return msg.op;
		}

		inline void setMessageType(dhcp_message_type msgt) {
			setOptionData(dhcp_option::MESSAGE_TYPE, static_cast<quint8>(msgt));
		}
		inline dhcp_message_type getMessageType() {
			return static_cast<dhcp_message_type>(getOptionData<quint8>(dhcp_option::MESSAGE_TYPE, 0xffu));
		}

		// id is htonl(IPv4)
		inline void setDHCPServerID(quint32 id) {
			setOptionData(dhcp_option::SERVER_ID, id);
		}
		inline quint32 getDHCPServerID() {
			return getOptionData<quint32>(dhcp_option::SERVER_ID, 0xffffffffu);
		}

		// This is the client ip address returned from the server
		inline QHostAddress getYourIP() {
			return QHostAddress(ntohl(msg.yiaddr));
		}

		inline void send(Interface_IPv4* iface) {
			if (sendUnicast) {
				iface->sendUnicastDHCP(this);
			} else {
				iface->m_env->m_device->sendDHCPClientPacket(this);
			}
		}

	public:
		struct dhcp_msg {
			bootp_op op;
			quint8 htype, hlen, hops;
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
		std::map<dhcp_option, QVector<quint8>> options;
		QByteArray msgdata;

		bool sendUnicast = false;
		bool creationFailed = false;
		QHostAddress unicast_addr;
	};

	class DHCPClientPacket : public DHCPPacket {
	public:
		// broadcast packet
		explicit DHCPClientPacket(Interface_IPv4* iface);

		// unicast packet
		explicit DHCPClientPacket(Interface_IPv4* iface, QHostAddress const& unicast_addr);

		// set vendor option ("nuts-0.1")
		void setVendor();
		// set requested parameter option
		void setParamRequest();

		// broadcasts
		void doDHCPDiscover();
		void doDHCPRequest(DHCPPacket const& reply);
		void doDHCPRebind(QHostAddress const& ip);

		// unicasts
		void doDHCPRenew(QHostAddress const& ip);
		void doDHCPRelease(QHostAddress const& ip, QVector<quint8> server_id);

		void requestIP(QHostAddress const& ip);
		void send();

	private:
		Interface_IPv4* iface = nullptr;
	};
}

#endif /* _NUTS_DHCPPACKET_H */
