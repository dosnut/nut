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

extern "C" {
	struct nl_addr;
	struct nl_cache;
	struct nl_cb;
	struct nl_sock;
	struct rtnl_addr;
	struct rtnl_link;
	struct rtnl_nexthop;
	struct rtnl_route;

	struct ifreq;
}

namespace nuts {
	namespace internal {
		struct free_nl_data {
			void operator()(::nl_addr* addr);
			void operator()(::nl_cache* cache);
			void operator()(::nl_cb* cb);
			void operator()(::nl_sock* sock);
			void operator()(::rtnl_addr *addr);
			void operator()(::rtnl_link *link);
			void operator()(::rtnl_route *route);
			void operator()(::rtnl_nexthop *nh);
		};

		template<typename ObjectType>
		struct free_nl_cache {
			void operator()(::nl_cache* cache) {
				free_nl_data{}(cache);
			}
		};
	}
	using nl_addr_ptr = std::unique_ptr<::nl_addr, internal::free_nl_data>;
	template<typename ObjectType>
	using nl_cache_ptr = std::unique_ptr<::nl_cache, internal::free_nl_cache<ObjectType>>;
	using nl_cb_ptr = std::unique_ptr<::nl_cb, internal::free_nl_data>;
	using nl_sock_ptr = std::unique_ptr<::nl_sock, internal::free_nl_data>;
	using rtnl_addr_ptr = std::unique_ptr<::rtnl_addr, internal::free_nl_data>;
	using rtnl_link_ptr = std::unique_ptr<::rtnl_link, internal::free_nl_data>;
	using rtnl_route_ptr = std::unique_ptr<::rtnl_route, internal::free_nl_data>;
	using rtnl_nexthop_ptr = std::unique_ptr<::rtnl_nexthop, internal::free_nl_data>;

	class HardwareManager final : public QObject {
		Q_OBJECT
	public:
		explicit HardwareManager();
		~HardwareManager();

		bool controlOn(int ifIndex, bool force = false);
		bool controlOff(int ifIndex);

		/* initial scan for interfaces */
		void discover();

		libnutcommon::MacAddress getMacAddress(QString const& ifName);

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

		void cleanupIPv6AutoAssigned(int ifIndex);
		void reenableIPv6(int ifIndex);

	signals:
		void gotCarrier(QString const& ifName, int ifIndex, QString const& essid);
		void lostCarrier(QString const& ifName);

		void newDevice(QString const& ifName, int ifIndex);
		void delDevice(QString const& ifName);

	private slots:
		void read_netlinkmsgs();

	private:
		bool init_netlink();
		void free_netlink();
		bool init_ethtool();
		void free_ethtool();

		bool isControlled(int ifIndex);

		bool ifreq_init(struct ifreq& ifr, QString const& ifname);
		void ifreq_init(struct ifreq& ifr);

	private: /* vars */
		int ethtool_fd{-1};

		nl_sock_ptr m_nlh_control;

		nl_sock_ptr m_nlh_watcher;
		std::unique_ptr<QSocketNotifier> m_nlh_watcher_notifier;
		struct ifstate {
			explicit ifstate() = default;

			void on();
			void off();

			bool active{false};
			bool carrier{false};
			bool exists{false};
			QString name;

			rtnl_addr_ptr link_local_ipv6_reenable;
		};
		std::vector<struct ifstate> ifStates;
	};
}

#endif /* _NUTS_HARDWARE_H */
