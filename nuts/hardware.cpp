#include "hardware.h"
#include "exception.h"
#include "log.h"
#include "device.h"

#include <libnutnetlink/netlink_addr.h>
#include <libnutnetlink/netlink_cache.h>
#include <libnutnetlink/netlink_rtnl_addr.h>
#include <libnutnetlink/netlink_rtnl_link.h>
#include <libnutnetlink/netlink_rtnl_route.h>

extern "C" {
#include <asm/types.h>
// socket, AF_INET, SOCK_RAW
#include <sys/socket.h>
// IPPROTO_RAW
#include <arpa/inet.h>
// fcntl, F_SETFD, FD_CLOEXEC
#include <fcntl.h>
#include <sys/ioctl.h>

#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/ethtool.h>
#include <linux/sysctl.h>
#include <linux/wireless.h>
#include <unistd.h>
}

#include <QSocketNotifier>
#include <QFile>

namespace nuts {
	namespace {
		/* various */

		int getPrefixLenIPv4(const QHostAddress &netmask) {
			quint32 val = netmask.toIPv4Address();
			int i = 32;
			while (val && !(val & 0x1)) {
				i--;
				val >>= 1;
			}
			return i;
		}

		netlink::nl_addr_ref NLAddrDefaultIPv4() {
			quint32 i = 0;
			auto a = netlink::nl_addr_ref::create(AF_INET, &i, sizeof(i));
			a.set_prefixlen(0);
			return a;
		}

		netlink::nl_addr_ref toNLAddrIPv4(QHostAddress const& addr, QHostAddress const& netmask = QHostAddress()) {
			auto a = netlink::nl_addr_ref::create(addr);
			if (!netmask.isNull()) {
				a.set_prefixlen(getPrefixLenIPv4(netmask));
			}
			return a;
		}

		netlink::nl_addr_ref toNLBroadcastIPv4(const QHostAddress &addr, const QHostAddress &netmask) {
			quint32 nm = 0;
			if (!netmask.isNull()) nm = htonl(netmask.toIPv4Address());
			quint32 i = htonl(addr.toIPv4Address());
			quint32 bcast = (i & nm) | (~nm);
			return netlink::nl_addr_ref::create(AF_INET, &bcast, sizeof(bcast));
		}

		netlink::rtnl_addr_ref makeRTNLAddrIPv4(int ifIndex, QHostAddress const& ip, QHostAddress const& netmask) {
			// zeroconf: in 169.254.0.0/16
			bool const isZeroconf = (netmask.toIPv4Address() == 0xFFFF0000u)
				&& ((ip.toIPv4Address() & 0xFFFF0000u) == 0xA9FE0000);

			auto addr = netlink::rtnl_addr_ref::alloc();
			addr.set_ifindex(ifIndex);
			addr.set_family(AF_INET);
			addr.set_local(toNLAddrIPv4(ip, netmask));
			addr.set_broadcast(toNLBroadcastIPv4(ip, netmask));
			if (isZeroconf) {
				addr.set_scope(RT_SCOPE_LINK);
				addr.set_flags(netlink::rtnl_addr_flag::permanent);
			}

			return addr;
		}

		netlink::rtnl_route_ref makeRTNLRouteIPv4(int ifIndex, QHostAddress const& gateway, int metric) {
			auto nh = netlink::rtnl_nexthop::alloc();
			nh.set_ifindex(ifIndex);
			nh.set_gateway(toNLAddrIPv4(gateway));

			auto route = netlink::rtnl_route_ref::alloc();
			route.set_family(AF_INET);
			route.set_dst(NLAddrDefaultIPv4());
			route.set_protocol(RTPROT_BOOT);
			route.set_scope(RT_SCOPE_UNIVERSE);

			if (-1 != metric) {
				route.set_priority(static_cast<uint32_t>(metric));
			}
			route.add_nexthop(std::move(nh));

			return route;
		}

		std::error_code appendRoutes(netlink::nl_socket_ptr const& sock, int family, std::list<netlink::rtnl_route_ref>& list) {
			struct rtmsg rhdr;
			memset(&rhdr, 0, sizeof(rhdr));
			rhdr.rtm_family = family;

			std::error_code err;
			do {
				err = sock.send_simple(RTM_GETROUTE, NLM_F_DUMP, &rhdr, sizeof(rhdr));
			} while (netlink::errc::intr == err);

			auto local_cb = sock.get_cb().clone();
			local_cb.set(netlink::callback_type::valid, [&list](::nl_msg* msg) -> int {
				auto route = netlink::rtnl_route_ref::parse_msg(msg);
				list.push_back(route);
				return 0;
			});

			return sock.recvmsgs(local_cb);
		}

		QString routeTargetToString(netlink::nl_addr_ref const& addr) {
			if (0 == addr.get_prefixlen()) return QString("default");
			return addr.toString();
		}

		libnutcommon::MacAddress linkAddress(netlink::rtnl_link_ref const& link) {
			auto addr = link.get_addr();
			if (!addr || 6 != addr.get_len()) return libnutcommon::MacAddress();
			return libnutcommon::MacAddress(reinterpret_cast<::ether_addr*>(addr.get_binary_addr()));
		}
	} /* anonymous namespace */

	HardwareManager::HardwareManager() {
		if (!init_netlink()) {
			throw NetlinkInitException("HardwareManager: Couldn't init netlink");
		}
		if (!init_ethtool()) {
			free_netlink();
			throw EthtoolInitException("HardwareManager: Couldn't init ethtool");
		}
	}

	HardwareManager::~HardwareManager() {
		free_ethtool();
		free_netlink();
	}

	bool HardwareManager::controlOn(int ifIndex, bool force) {
		if (ifIndex < 0 || (size_t)ifIndex >= ifStates.size()) return false;

		auto change = netlink::rtnl_link_ref::alloc();
		change.set_ifindex(ifIndex);

		if (force) {
			change.unset_flags(netlink::link_flag_t::up);
			if (auto ec = netlink::rtnl_link_ref::change(m_nlh_control, change, change)) {
				log << "Couldn't turn device off (forcing restart): " << ec.message().data() << endl;
				return false;
			}
		}

		change.set_flags(netlink::link_flag_t::up);
		if (auto ec = netlink::rtnl_link_ref::change(m_nlh_control, change, change)) {
			log << "Couldn't turn device on: " << ec.message().data() << endl;
			return false;
		}

		ifStates[ifIndex].on();
//		log << "activated interface " << ifIndex << endl;
		return true;
	}

	bool HardwareManager::controlOff(int ifIndex) {
		if (ifIndex < 0) return false;
		if ((size_t)ifIndex >= ifStates.size()) return true;
		if (ifStates[ifIndex].active) {
			ifStates[ifIndex].off();

			auto change = netlink::rtnl_link_ref::alloc();
			change.set_ifindex(ifIndex);

			change.unset_flags(netlink::link_flag_t::up);
			if (auto ec = netlink::rtnl_link_ref::change(m_nlh_control, change, change)) {
				log << "Couldn't turn device off: " << ec.message().data() << endl;
				return false;
			}
		}
		return true;
	}

	bool HardwareManager::init_netlink() {
		m_nlh_control = netlink::nl_socket_ptr::alloc();
		m_nlh_watcher = netlink::nl_socket_ptr::alloc();

		if (!m_nlh_watcher || !m_nlh_control) goto cleanup;
		m_nlh_watcher.disable_seq_check();
		m_nlh_watcher.get_cb().set(netlink::callback_type::valid,
			&HardwareManager::wrap_handle_netlinkmsg,
			reinterpret_cast<void*>(this));

		if (auto ec = m_nlh_control.connect(netlink::protocol::route)) {
			log << "Failed to connect netlink control" << ec.message().data() << endl;
			goto cleanup;
		}

		if (auto ec = m_nlh_watcher.connect_groups({netlink::rtnetlink_group::link, netlink::rtnetlink_group::ipv4_route, netlink::rtnetlink_group::ipv6_route})) {
			log << "Failed to connect netlink watcher / add group memberships" << ec.message().data() << endl;
			goto cleanup;

		}

		{
			fcntl(m_nlh_control.get_fd(), F_SETFD, FD_CLOEXEC);
		}
		{
			int netlink_fd = m_nlh_watcher.get_fd();
			fcntl(netlink_fd, F_SETFD, FD_CLOEXEC);
			// QSocketNotifier constructor makes netlink_fd non-blocking
			m_nlh_watcher_notifier.reset(new QSocketNotifier(netlink_fd, QSocketNotifier::Read));
			connect(m_nlh_watcher_notifier.get(), &QSocketNotifier::activated, this, &HardwareManager::read_netlinkmsgs);
		}

		return true;

cleanup:
		free_netlink();
		return false;
	}

	void HardwareManager::free_netlink() {
		m_nlh_watcher_notifier.reset();
		m_nlh_watcher.reset();
		m_nlh_control.reset();
	}

	bool HardwareManager::init_ethtool() {
		ethtool_fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
		if (ethtool_fd >= 0) {
			fcntl(ethtool_fd, F_SETFD, FD_CLOEXEC);
			return true;
		}
		return false;
	}
	void HardwareManager::free_ethtool() {
		close(ethtool_fd);
	}

	void HardwareManager::discover() {
		auto linkCache = netlink::rtnl_link_cache_ref::alloc();
		if (auto ec = linkCache.refill(m_nlh_control)) {
			log << "Failed to list links: " << ec.message().data() << endl;
			return;
		}
		for (netlink::rtnl_link_ref const& link: linkCache) {
			int const ifIndex{link.get_ifindex()};
			QString const ifName = link.get_name();
			libnutcommon::MacAddress const linkAddr = linkAddress(link);
			if (ifIndex <= 0) return;
			if ((size_t)ifIndex >= ifStates.size()) ifStates.resize(ifIndex+1);
			if (!ifStates[ifIndex].exists) {
				ifStates[ifIndex].exists = true;
				ifStates[ifIndex].name = ifName;
				ifStates[ifIndex].linkAddress = linkAddr;
				log << QString("discovered interface: %1 [index %2]").arg(ifName).arg(ifIndex) << endl;
				emit newDevice(ifName, ifIndex, linkAddr);
			}
		}
	}

	bool HardwareManager::ifreq_init(struct ifreq& ifr, QString const& ifname) {
		QByteArray buf = ifname.toUtf8();
		if (buf.size() >= IFNAMSIZ) return false;
		ifreq_init(ifr);
		strncpy(ifr.ifr_name, buf.constData(), buf.size());
		return true;
	}
	void HardwareManager::ifreq_init(struct ifreq& ifr) {
		memset(reinterpret_cast<char*>(&ifr), 0, sizeof(ifr));
	}

	void HardwareManager::read_netlinkmsgs() {
		if (auto ec = m_nlh_watcher.recvmsgs()) {
			log << "nl_recvmsgs failed: " << ec.message().data() << endl;
		}
	}

	int HardwareManager::wrap_handle_netlinkmsg(::nl_msg *msg, void* arg) {
		return reinterpret_cast<HardwareManager*>(arg)->handle_netlinkmsg(msg);
	}

	int HardwareManager::handle_netlinkmsg(::nl_msg *msg) {
		switch (nlmsg_hdr(msg)->nlmsg_type) {
		case RTM_NEWLINK:
			{
				auto link = netlink::rtnl_link_ref::parse_msg(msg);
				int const ifIndex = link.get_ifindex();
				if (ifIndex <= 0) {
					err << QString("invalid interface index %1").arg(ifIndex) << endl;
					return 0;
				}
				if ((size_t) ifIndex >= ifStates.size()) ifStates.resize(ifIndex+1);
				QString const ifName = link.get_name();
				if (ifName.isEmpty()) {
					err << QString("couldn't find name for interface %1").arg(ifIndex) << endl;
					return 0;
				}
				libnutcommon::MacAddress const linkAddr = linkAddress(link);

				if (!ifStates[ifIndex].exists) {
					ifStates[ifIndex].exists = true;
					ifStates[ifIndex].linkAddress = linkAddr;
					ifStates[ifIndex].name = ifName;
					log << QString("new interface detected: %1 [index %2]").arg(ifName).arg(ifIndex) << endl;
					emit newDevice(ifName, ifIndex, linkAddr);
				} else if (ifStates[ifIndex].name != ifName) {
					QString oldName = ifStates[ifIndex].name;
					ifStates[ifIndex].name = ifName;
					log << QString("interface rename detected: %1 -> %2 [index %3]").arg(oldName).arg(ifName).arg(ifIndex) << endl;
					emit delDevice(oldName);
					// in case the address changed update it here without notification - it is a "new" device anyway
					ifStates[ifIndex].linkAddress = linkAddr;
					emit newDevice(ifName, ifIndex, linkAddr);
				} else if (ifStates[ifIndex].linkAddress != linkAddr) {
					ifStates[ifIndex].linkAddress = linkAddr;
					emit changedMacAddress(ifName, linkAddr);
				}
				bool const carrier = (link.get_flags() & netlink::link_flag_t::lower_up);
				if (carrier != ifStates[ifIndex].carrier) {
					ifStates[ifIndex].carrier = carrier;
					if (!isControlled(ifIndex))
						return 0;
					if (carrier) {
						reenableIPv6(ifIndex);
						QString essid;
						getEssid(ifName, essid);
						emit gotCarrier(ifName, ifIndex, essid);
					} else {
						emit lostCarrier(ifName);
						cleanupIPv6AutoAssigned(ifIndex);
					}
				}
			};
			break;
		case RTM_DELLINK:
			{
				auto link = netlink::rtnl_link_ref::parse_msg(msg);
				int const ifIndex = link.get_ifindex();
				if (ifIndex <= 0) {
					err << QString("invalid interface index %1").arg(ifIndex) << endl;
					return 0;
				}
				QString const ifName = link.get_name();
				if (ifName.isNull()) return 0;
				log << QString("lost interface: %1 [index %2]").arg(ifName).arg(ifIndex) << endl;
				if ((size_t) ifIndex < ifStates.size()) {
					if (ifStates[ifIndex].exists) {
						if (ifStates[ifIndex].carrier)
							emit lostCarrier(ifName);
						ifStates[ifIndex].exists = false;
						ifStates[ifIndex].carrier = false;
						ifStates[ifIndex].active = false;
						ifStates[ifIndex].linkAddress.clear();
						ifStates[ifIndex].name.clear();
					}
				}
				emit delDevice(ifName);
			};
			break;
		case RTM_NEWROUTE:
			{
				auto route = netlink::rtnl_route_ref::parse_msg(msg);
#if 0
				bool const replacedRoute = 0 != (NLM_F_REPLACE & nlmsg_hdr(msg)->nlmsg_flags);
				log << (replacedRoute ? "Replaced " : "New ") << "route with metric " << route.get_priority() << ": " << route.toString() << endl;
#endif
				checkRouteMetric(route);
			}
			break;
		}

		return 0;
	}

	void HardwareManager::checkRouteMetric(netlink::rtnl_route_ref const& route) {
		switch (route.get_family()) {
		case AF_INET6:
			{
				/* don't touch IPv6 link-local fe80::/64 */
				static const uint8_t link_local_prefix[8] =
					{ 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
				auto dst = route.get_dst();
				if (64 == dst.get_prefixlen() && 0 == memcmp(link_local_prefix, dst.get_binary_addr(), 8)) return;
			}
			break;
		case AF_INET:
			/* don't touch default routes: the metric for those is
			 * set explicitly
			 */
			{
				auto dst = route.get_dst();
				if (0 == dst.get_prefixlen()) return;
			}
			break;
		default:
			return;
		}

		if (RT_TABLE_MAIN != route.get_table()) return;
		if (RTN_UNICAST != route.get_type()) return;

		if (1 != route.get_nnexthops()) return;
		auto nh = route.nexthop_n(0);
		int const ifIndex = nh.get_ifindex();

		if (!isControlled(ifIndex)) return;

		int const metric = ifStates[ifIndex].metric;
		if (-1 == metric || static_cast<uint32_t>(metric) == route.get_priority()) return;

		log << "Replacing route with updated metric " << metric << ": " << route.toString() << endl;
		log.flush();

		if (auto ec = route.remove(m_nlh_control)) {
			if (netlink::errc::obj_notfound != ec) {
				log << "Removing route failed: " << ec.message().data() << endl;
			}
			return;
		}

		route.set_priority(metric);

		if (auto ec = route.add(m_nlh_control, netlink::new_flag::replace)) {
			if (netlink::errc::obj_notfound != ec) {
				log << "Adding route with updated metric failed: " << ec.message().data() << endl;
			}
		}
	}

	bool HardwareManager::isControlled(int ifIndex) {
		if (ifIndex < 0 || (size_t)ifIndex >= ifStates.size()) return false;
		return ifStates[ifIndex].active;
	}

	static void iwreq_init(struct iwreq& iwr) {
		memset(reinterpret_cast<char*>(&iwr), 0, sizeof(iwr));
	}

	static bool iwreq_init(struct iwreq& iwr, QString const& ifname) {
		QByteArray buf = ifname.toUtf8();
		if (buf.size() >= IFNAMSIZ) return false;
		iwreq_init(iwr);
		strncpy(iwr.ifr_ifrn.ifrn_name, buf.constData(), buf.size());
		return true;
	}

	bool HardwareManager::hasWLAN(QString const& ifName) {
		// NL80211_ATTR_SSID
		struct iwreq iwr;
		iwreq_init(iwr, ifName);
		if (ioctl(ethtool_fd, SIOCGIWNAME, &iwr) < 0) return false; // cfg80211_wext_giwname
		return true;
	}

	bool HardwareManager::getEssid(QString const& ifName, QString& essid) {
		essid = "";
		struct iwreq iwr;
		iwreq_init(iwr, ifName);
		char buf[32];
		memset(buf, 0, sizeof(buf));
		iwr.u.essid.pointer = buf;
		iwr.u.essid.length = sizeof(buf);
		if (ioctl(ethtool_fd, SIOCGIWESSID, &iwr) < 0) return false; // cfg80211_wext_giwessid
		essid = QString::fromUtf8(buf, qstrnlen(buf, iwr.u.essid.length));
		return true;
	}

	void HardwareManager::ipv4AddressAdd(
			int ifIndex,
			const QHostAddress& ip,
			const QHostAddress& netmask,
			const QHostAddress& gateway,
			int metric)
	{
		if (auto ec = makeRTNLAddrIPv4(ifIndex, ip, netmask).add(m_nlh_control)) {
			log << "Adding address failed [ndx=" << ifIndex << "]: " << ec.message().data() << endl;
		}

		if (!gateway.isNull()) {
			/* NLM_F_EXCL would fail if there is already a route with the
			 * same target,tos,priority
			 *
			 * For now overwrite existing routes.
			 */
			if (auto ec = makeRTNLRouteIPv4(ifIndex, gateway, metric).add(m_nlh_control)) {
				log << "Adding default gateway failed [ndx=" << ifIndex << "]: " << ec.message().data() << endl;
			}
		}
	}

	void HardwareManager::ipv4AddressDelete(
			int ifIndex,
			const QHostAddress& ip,
			const QHostAddress& netmask,
			const QHostAddress& gateway,
			int metric)
	{
		if (!isControlled(ifIndex)) return; /* probably got deleted, skip cleanup */

				if (!gateway.isNull()) {
			if (auto ec = makeRTNLRouteIPv4(ifIndex, gateway, metric).remove(m_nlh_control)) {
				log << "Removing default gateway failed [ndx=" << ifIndex << "]: " << ec.message().data() << endl;
			}
		}

		if (auto ec = makeRTNLAddrIPv4(ifIndex, ip, netmask).remove(m_nlh_control)) {
			log << "Removing address failed [ndx=" << ifIndex << "]: " << ec.message().data() << endl;
		}
	}

	void HardwareManager::setMetric(int ifIndex, int metric) {
		if (!isControlled(ifIndex)) return;
		if (metric == ifStates[ifIndex].metric) return;
		ifStates[ifIndex].metric = metric;

		/* update "all" routes */
		std::list<netlink::rtnl_route_ref> routes;
		if (auto ec = appendRoutes(m_nlh_control, AF_INET, routes)) {
			log << "appendRoutes failed: " << ec.message().data() << endl;
		}
		if (auto ec = appendRoutes(m_nlh_control, AF_INET6, routes)) {
			log << "appendRoutes failed: " << ec.message().data() << endl;
		}
		for (auto const& route: routes) {
			checkRouteMetric(route);
		}
	}

	/* Linux IPv6 (auto) address configuration:
	 *
	 * When the interface comes up the kernel assigns a link-local
	 * address and runs DAD (duplicate address detection). When DAD
	 * completes successfully, the kernel sends 3 (sysctl
	 * router_solicitations) router solicitations, waiting for 4
	 * seconds (sysctl router_solicitation_interval) between them.
	 *
	 * Usually the only way to restart this process is to take the
	 * interface down completely (ip link set dev $DEV down) and up
	 * again.
	 *
	 * As long as the device stays on the same link this is not a
	 * problem as the router is supposed to broadcast router advertisments
	 * in regular intervals, and the kernel will always use them
	 * and update the local configuration.
	 *
	 * But if you lost the carrier you actually want to drop all
	 * (automatically) configured addresses and routes for that
	 * interface, and restart sending solicitations when the carrier
	 * comes back.
	 *
	 * (While the carrier is down the routes are not actually used
	 * but they are still present with the "linkdown" flag).
	 *
	 * So we kill all IPv6 addresses and routes for an interface,
	 * remember the link-local IPv6 address, and if the carrier comes
	 * back, we add the remembered link-local IPv6 address again,
	 * which magically triggers sending new router solicitations (after
	 * DAD finishes).
	 *
	 * Yay.
	 */
	void HardwareManager::cleanupIPv6AutoAssigned(int ifIndex) {
		if (!isControlled(ifIndex)) return;

		{
			auto removeIPv6AddressWithFlag = [this, ifIndex](netlink::rtnl_addr_ref const& addr, bool removeTemporaries) {
#if 0
				log << "Found address: " << addr.toString() << endl;
#endif

				/* remove all IPv6 addresses on interface (we "own" it) */
				if (addr.get_ifindex() != ifIndex) return;
				if (addr.get_family() != AF_INET6) return;

				switch (addr.get_scope()) {
				case RT_SCOPE_LINK:
					/* remember link-local address to restart router solicitation */
					ifStates[ifIndex].link_local_ipv6_reenable = addr;
					break;
				case RT_SCOPE_UNIVERSE:
					/* IFA_F_TEMPORARY (from privacy extension) should get removed automatically after its IFA_F_MANAGETEMPADDR is deleted */
					if (!removeTemporaries && 0 == (addr.get_flags() & netlink::rtnl_addr_flag::managetempaddr)) return;
					break;
				default:
					break;
				}

				log << "Removing IPv6 address "
					<< addr.get_local().toString()
					<< " [ndx=" << ifIndex << "]"
					<< endl;

				if (auto ec = addr.remove(m_nlh_control)) {
					log << "Removing IPv6 address "
						<< addr.get_local().toString()
						<< " failed [ndx=" << ifIndex << "]: " << ec.message().data() << endl;
				}
			};

			auto cache = netlink::rtnl_addr_cache_ref::alloc();
			if (auto ec = cache.refill(m_nlh_control)) {
				log << "Getting addresses failed: " << ec.message().data() << endl;
			}
			for (netlink::rtnl_addr_ref const& route: cache) {
				removeIPv6AddressWithFlag(route, /* removeTemporaries */ false);
			}

			cache.reset();

			if (auto ec = cache.refill(m_nlh_control)) {
				log << "Getting addresses failed: " << ec.message().data() << endl;
			}
			for (netlink::rtnl_addr_ref const& route: cache) {
				removeIPv6AddressWithFlag(route, /* removeTemporaries */ true);
			}
		}

		{
			std::list<netlink::rtnl_route_ref> routes;
			if (auto ec = appendRoutes(m_nlh_control, AF_INET6, routes)) {
				log << "appendRoutes failed: " << ec.message().data() << endl;
			}
			for (netlink::rtnl_route_ref const& route: routes) {
#if 1
				log << "Found IPv6 route: " << route.toString() << endl;
#endif

				auto dst = route.get_dst();

				if (1 != route.get_nnexthops()) continue;

				netlink::rtnl_nexthop_ptr nh = route.nexthop_n(0);
				if (ifIndex != nh.get_ifindex()) continue;

				log << "Removing IPv6 route to "
					<< routeTargetToString(dst)
					<< " via "
					<< nh.get_gateway().toString()
					<< " [ndx=" << ifIndex << "]"
					<< endl;

				if (auto ec = route.remove(m_nlh_control)) {
					log << "Removing IPv6 route to "
						<< routeTargetToString(dst)
						<< " via "
						<< nh.get_gateway().toString()
						<< " failed [ndx=" << ifIndex << "]: " << ec.message().data() << endl;
				}
			}
		}
	}

	void HardwareManager::reenableIPv6(int ifIndex) {
		if (!isControlled(ifIndex)) return;

		netlink::rtnl_addr_ref addr = std::move(ifStates[ifIndex].link_local_ipv6_reenable);
		if (!addr) return;

		log << "Re-adding IPv6 link-local address "
			<< addr.get_local().toString()
			<< " [ndx=" << ifIndex << "]"
			<< endl;

		if (auto ec = addr.add(m_nlh_control)) {
			log << "Readding IPv6 link-local address "
				<< addr.get_local().toString()
				<< " failed [ndx=" << ifIndex << "]: " << ec.message().data() << endl;
		}
	}

	void HardwareManager::ifstate::on() {
		active = true;
		exists = true;
		carrier = false;
	}

	void HardwareManager::ifstate::off() {
		active = false;
		metric = -1;
	}
}
