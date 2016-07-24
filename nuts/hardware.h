#ifndef _NUTS_HARDWARE_H
#define _NUTS_HARDWARE_H

#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QVector>
#include <QHostAddress>
#include <QSocketNotifier>

#include <memory>

#include <libnutcommon/macaddress.h>

#include <libnutnetlink/netlink_sock.h>
#include <libnutnetlink/netlink_rtnl_addr.h>

extern "C" {
	struct ifreq;
}

namespace nuts {
	class HardwareManager final : public QObject {
		Q_OBJECT
	public:
		explicit HardwareManager();
		~HardwareManager();

		bool controlOn(int ifIndex, bool force = false);
		bool controlOff(int ifIndex);

		/* initial scan for interfaces */
		void discover();

		bool hasWLAN(QString const& ifName);
		bool getEssid(QString const& ifName, QString& essid);

		void ipv4AddressAdd(
			int ifIndex,
			QHostAddress const& ip,
			QHostAddress const& netmask,
			QHostAddress const& gateway,
			int metric);
		void ipv4AddressDelete(
			int ifIndex,
			QHostAddress const& ip,
			QHostAddress const& netmask,
			QHostAddress const& gateway,
			int metric);

		void setMetric(int ifIndex, int metric);

	signals:
		void gotCarrier(QString const& ifName, int ifIndex, QString const& essid);
		void lostCarrier(QString const& ifName);

		void newDevice(QString const& ifName, int ifIndex, libnutcommon::MacAddress const& addr);
		void delDevice(QString const& ifName);

		void changedMacAddress(QString const& ifName, libnutcommon::MacAddress const& addr);

	private slots:
		void read_netlinkmsgs();

	private:
		static int wrap_handle_netlinkmsg(::nl_msg *msg, void* arg);
		int handle_netlinkmsg(::nl_msg *msg);
		void checkRouteMetric(const netlink::rtnl_route_ref& route);

		void cleanupIPv6AutoAssigned(int ifIndex);
		void reenableIPv6(int ifIndex);

		bool init_netlink();
		void free_netlink();
		bool init_ethtool();
		void free_ethtool();

		bool isControlled(int ifIndex);

		bool ifreq_init(struct ifreq& ifr, QString const& ifname);
		void ifreq_init(struct ifreq& ifr);

	private: /* vars */
		int ethtool_fd{-1};

		netlink::nl_socket_ptr m_nlh_control;
		netlink::nl_socket_ptr m_nlh_watcher;

		std::unique_ptr<QSocketNotifier> m_nlh_watcher_notifier;
		struct ifstate {
			explicit ifstate() = default;

			void on();
			void off();

			bool active{false};
			bool carrier{false};
			bool exists{false};
			int metric{-1};
			libnutcommon::MacAddress linkAddress;
			QString name;

			netlink::rtnl_addr_ref link_local_ipv6_reenable;
		};
		std::vector<struct ifstate> ifStates;
	};
}

#endif /* _NUTS_HARDWARE_H */
