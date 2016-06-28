#include "hardware.h"
#include "exception.h"
#include "log.h"
#include "device.h"

extern "C" {
#include <asm/types.h>
// socket, AF_INET, SOCK_RAW
#include <sys/socket.h>
// IPPROTO_RAW
#include <arpa/inet.h>
// fcntl, F_SETFD, FD_CLOEXEC
#include <fcntl.h>
#include <sys/ioctl.h>

#include <netlink/netlink.h>
#include <netlink/msg.h>
#include <netlink/route/addr.h>
#include <netlink/route/link.h>
#include <netlink/route/route.h>
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
		/* netlink types handling */

		template<typename Object>
		struct nl_object_traits {
			static constexpr char const* type = nullptr;
		};

		template<>
		struct nl_object_traits<::rtnl_route> {
			static constexpr char const* type = "route/route";
		};

		template<>
		struct nl_object_traits<::rtnl_link> {
			static constexpr char const* type = "route/link";
		};

		template<>
		struct nl_object_traits<::rtnl_addr> {
			static constexpr char const* type = "route/addr";
		};

		template<typename Object>
		Object* nl_safe_object_cast(::nl_object* obj) {
			static_assert(nl_object_traits<Object>::type, "type not an nl_object or not supported");
			if (obj && 0 == strcmp(nl_object_traits<Object>::type, nl_object_get_type(obj))) {
				return reinterpret_cast<Object*>(obj);
			}
			return nullptr;
		}

		template<typename Object>
		::nl_object* nl_safe_to_object(Object* obj) {
			static_assert(nl_object_traits<Object>::type, "type not an nl_object or not supported");
			return reinterpret_cast<::nl_object*>(obj);
		}

		template<typename Object>
		std::unique_ptr<Object, internal::free_nl_data> nlCopyObjectPtr(Object* ptr) {
			static_assert(nl_object_traits<Object>::type, "type not an nl_object or not supported");
			if (ptr) nl_object_get(reinterpret_cast<nl_object*>(ptr));
			return std::unique_ptr<Object, internal::free_nl_data>(ptr);
		}

		template<typename Object>
		std::unique_ptr<Object, internal::free_nl_data> nlCopyObjectPtr(std::unique_ptr<Object, internal::free_nl_data> const& ptr) {
			return nlCopyObjectPtr(ptr.get());
		}

		/* helpers to deal with netlink callbacks */

		template<typename Object>
		void internal_call_msg_type_cb_function(::nl_object* obj, void *arg) {
			auto o = nl_safe_object_cast<Object>(obj);
			if (!o) {
				log
					<< "Unexpected object type: '" << nl_object_get_type(obj)
					<< "', wanted '" << nl_object_traits<Object>::type << "'\n";
				return;
			}
			auto const cb = reinterpret_cast<std::function<void(Object*)> const*>(arg);
			(*cb)(o);
		}
		template<typename Object>
		int nlMsgParseType(::nl_msg* msg, std::function<void(Object*)> const& func) {
			return nl_msg_parse(msg, internal_call_msg_type_cb_function<Object>, const_cast<void*>(reinterpret_cast<void const*>(&func)));
		}

		template<typename Object>
		int internal_call_nl_cb_msg_parse_function(::nl_msg *msg, void *arg) {
			auto const cb = reinterpret_cast<std::function<void(Object*)> const*>(arg);
			return nlMsgParseType(msg, *cb);
		}
		template<typename Object>
		int nlSetMsgParseCallback(::nl_cb* cb, ::nl_cb_type type, std::function<void(Object*)> const& func) {
			return ::nl_cb_set(cb, type, NL_CB_CUSTOM, internal_call_nl_cb_msg_parse_function<Object>, const_cast<void*>(reinterpret_cast<void const*>(&func)));
		}

		template<typename ObjectType>
		void internal_cacheForeach(::nl_object* obj, void *cb_ptr) {
			std::function<void(ObjectType*)> const& cb = *reinterpret_cast<std::function<void(ObjectType*)> const*>(cb_ptr);
			cb(reinterpret_cast<ObjectType*>(obj));
		}

		template<typename ObjectType>
		void cacheForeach(nl_cache_ptr<ObjectType>const& cache, std::function<void(ObjectType*)> const& cb) {
			if (!cache) return;
			void *cb_ptr = const_cast<void*>(reinterpret_cast<void const*>(&cb));
			nl_cache_foreach(cache.get(), &internal_cacheForeach<ObjectType>, cb_ptr);
		}

		template<typename ObjectType>
		void cacheForeachFilter(nl_cache_ptr<ObjectType>const& cache, ObjectType const* filter, std::function<void(ObjectType*)> const& cb) {
			if (!cache) return;
			void *cb_ptr = const_cast<void*>(reinterpret_cast<void const*>(&cb));
			nl_cache_foreach_filter(cache.get(), const_cast<nl_object*>(reinterpret_cast<nl_object const*>(filter)), &internal_cacheForeach<ObjectType>, cb_ptr);
		}

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

		nl_addr_ptr NLAddrDefaultIPv4() {
			quint32 i = 0;
			nl_addr_ptr a{nl_addr_build(AF_INET, &i, sizeof(i))};
			nl_addr_set_prefixlen(a.get(), 0);
			return a;
		}

		nl_addr_ptr toNLAddrIPv4(QHostAddress const& addr, QHostAddress const& netmask = QHostAddress()) {
			quint32 i = htonl(addr.toIPv4Address());
			nl_addr_ptr a{nl_addr_build(AF_INET, &i, sizeof(i))};
			if (!netmask.isNull()) {
				nl_addr_set_prefixlen(a.get(), getPrefixLenIPv4(netmask));
			}
			return a;
		}

		nl_addr_ptr toNLBroadcastIPv4(const QHostAddress &addr, const QHostAddress &netmask) {
			quint32 nm = 0;
			if (!netmask.isNull()) nm = htonl(netmask.toIPv4Address());
			quint32 i = htonl(addr.toIPv4Address());
			quint32 bcast = (i & nm) | (~nm);
			nl_addr_ptr a{nl_addr_build(AF_INET, &bcast, sizeof(bcast))};
			return a;
		}

		rtnl_addr_ptr makeRTNLAddrIPv4(int ifIndex, QHostAddress const& ip, QHostAddress const& netmask) {
			// zeroconf: in 169.254.0.0/16
			bool const isZeroconf = (netmask.toIPv4Address() == 0xFFFF0000u)
				&& ((ip.toIPv4Address() & 0xFFFF0000u) == 0xA9FE0000);

			rtnl_addr_ptr addr{rtnl_addr_alloc()};
			rtnl_addr_set_ifindex(addr.get(), ifIndex);
			rtnl_addr_set_family(addr.get(), AF_INET);
			rtnl_addr_set_local(addr.get(), toNLAddrIPv4(ip, netmask).get());
			rtnl_addr_set_broadcast(addr.get(), toNLBroadcastIPv4(ip, netmask).get());
			if (isZeroconf) {
				rtnl_addr_set_scope(addr.get(), RT_SCOPE_LINK);
				rtnl_addr_set_flags(addr.get(), IFA_F_PERMANENT);
			}

			return addr;
		}

		rtnl_route_ptr makeRTNLRouteIPv4(int ifIndex, QHostAddress const& gateway, int metric) {
			rtnl_nexthop_ptr nh{rtnl_route_nh_alloc()};
			rtnl_route_nh_set_ifindex(nh.get(), ifIndex);
			rtnl_route_nh_set_gateway(nh.get(), toNLAddrIPv4(gateway).get());

			rtnl_route_ptr route{rtnl_route_alloc()};
			rtnl_route_set_family(route.get(), AF_INET);
			rtnl_route_set_dst(route.get(), NLAddrDefaultIPv4().get());
			rtnl_route_set_protocol(route.get(), RTPROT_BOOT);
			rtnl_route_set_scope(route.get(), RT_SCOPE_UNIVERSE);

			if (-1 != metric) {
				rtnl_route_set_priority(route.get(), static_cast<uint32_t>(metric));
			}
			// route owns added nexthops, nexthop doesn't have ref count
			rtnl_route_add_nexthop(route.get(), nh.release());

			return route;
		}

		nl_cache_ptr<::rtnl_link> makeRTNLLinkCache(::nl_sock* sock, int family = AF_UNSPEC) {
			::nl_cache* cache{nullptr};
			if (0 == rtnl_link_alloc_cache(sock, family, &cache)) {
				return nl_cache_ptr<::rtnl_link>{cache};
			}
			return nl_cache_ptr<::rtnl_link>{};
		}

		rtnl_link_ptr RTNLLinkAlloc() {
			return rtnl_link_ptr{rtnl_link_alloc()};
		}

		int makeRTNLAddrCache(nl_cache_ptr<::rtnl_addr> &cacheResult, ::nl_sock* sock) {
			::nl_cache* cache{nullptr};
			int res = rtnl_addr_alloc_cache(sock, &cache);
			cacheResult.reset(cache);
			return res;
		}

#if 0
		/* the libnl route cache has a serious bug: it only lists one route per
		 * [addr,tos,table,prio] tuple, but the kernel may have more than one.
		 *
		 * for example if you have a wired and a wireless connection to the same
		 * router you are often in the same network with two interfaces.
		 */
		nl_cache_ptr<::rtnl_route> makeRTNLRouteCache(::nl_sock* sock, int family = AF_UNSPEC, int flags = 0) {
			::nl_cache* cache{nullptr};
			if (0 == rtnl_route_alloc_cache(sock, family, flags, &cache)) {
				return nl_cache_ptr<::rtnl_route>{cache};
			}
			return nl_cache_ptr<::rtnl_route>{};
		}
#endif

		int appendRoutes(::nl_sock* sock, int family, std::list<rtnl_route_ptr>& list) {
			struct rtmsg rhdr;
			memset(&rhdr, 0, sizeof(rhdr));
			rhdr.rtm_family = family;

			int err;
			do {
				err = nl_send_simple(sock, RTM_GETROUTE, NLM_F_DUMP, &rhdr, sizeof(rhdr));
			} while (-NLE_DUMP_INTR == err);

			nl_cb_ptr local_cb{nl_cb_clone(nl_socket_get_cb(sock))};
			std::function<void(::rtnl_route*)> append_cb = [&list](::rtnl_route *route) {
				list.push_back(nlCopyObjectPtr(route));
			};
			nlSetMsgParseCallback<::rtnl_route>(local_cb.get(), NL_CB_VALID, append_cb);

			return nl_recvmsgs(sock, local_cb.get());
		}

		QString toString(nl_addr* addr) {
			char addrStringBuf[128];
			nl_addr2str(addr, addrStringBuf, sizeof(addrStringBuf));
			return QString(addrStringBuf);
		}

		QString routeTargetToString(nl_addr* addr) {
			if (0 == nl_addr_get_prefixlen(addr)) return QString("default");
			char addrStringBuf[128];
			nl_addr2str(addr, addrStringBuf, sizeof(addrStringBuf));
			return QString(addrStringBuf);
		}

	} /* anonymous namespace */

	namespace internal {
		void free_nl_data::operator()(nl_addr* addr) {
			nl_addr_put(addr);
		}

		void free_nl_data::operator()(nl_cache* cache) {
			nl_cache_free(cache);
		}

		void free_nl_data::operator()(::nl_cb* cb) {
			nl_cb_put(cb);
		}

		void free_nl_data::operator()(nl_sock* sock) {
			nl_close(sock);
			nl_socket_free(sock);
		}

		void free_nl_data::operator()(rtnl_addr* addr) {
			rtnl_addr_put(addr);
		}

		void free_nl_data::operator()(rtnl_link* link) {
			rtnl_link_put(link);
		}

		void free_nl_data::operator()(rtnl_route* route) {
			rtnl_route_put(route);
		}

		void free_nl_data::operator()(rtnl_nexthop* nh) {
			rtnl_route_nh_free(nh);
		}
	}

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

		rtnl_link_ptr change = RTNLLinkAlloc();
		rtnl_link_set_ifindex(change.get(), ifIndex);

		if (force) {
			rtnl_link_unset_flags(change.get(), IFF_UP);
			if (int err = rtnl_link_change(m_nlh_control.get(), change.get(), change.get(), 0)) {
				log << "Couldn't turn device off (forcing restart): " << nl_geterror(err) << " (" <<  err << ")" << endl;
				return false;
			}
		}

		rtnl_link_set_flags(change.get(), IFF_UP);
		if (int err = rtnl_link_change(m_nlh_control.get(), change.get(), change.get(), 0)) {
			log << "Couldn't turn device on: " << nl_geterror(err) << " (" <<  err << ")" << endl;
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

			rtnl_link_ptr change = RTNLLinkAlloc();
			rtnl_link_set_ifindex(change.get(), ifIndex);

			rtnl_link_unset_flags(change.get(), IFF_UP);
			if (int err = rtnl_link_change(m_nlh_control.get(), change.get(), change.get(), 0)) {
				log << "Couldn't turn device off: " << nl_geterror(err) << " (" <<  err << ")" << endl;
				return false;
			}
		}
		return true;
	}

	bool HardwareManager::init_netlink() {
		m_nlh_control.reset(nl_socket_alloc());
		m_nlh_watcher.reset(nl_socket_alloc());
		if (!m_nlh_watcher || !m_nlh_control) goto cleanup;
		::nl_socket_disable_seq_check(m_nlh_watcher.get());
		::nl_socket_modify_cb(
			m_nlh_watcher.get(), NL_CB_VALID, NL_CB_CUSTOM,
			&HardwareManager::wrap_handle_netlinkmsg,
			reinterpret_cast<void*>(this));

		if (nl_connect(m_nlh_control.get(), NETLINK_ROUTE) != 0) goto cleanup;

		if (nl_connect(m_nlh_watcher.get(), NETLINK_ROUTE) != 0) goto cleanup;
		if (nl_socket_add_membership(m_nlh_watcher.get(), RTNLGRP_LINK) != 0) goto cleanup;
		if (nl_socket_add_membership(m_nlh_watcher.get(), RTNLGRP_IPV4_ROUTE) != 0) goto cleanup;
		if (nl_socket_add_membership(m_nlh_watcher.get(), RTNLGRP_IPV6_ROUTE) != 0) goto cleanup;

		{
			int netlink_fd = nl_socket_get_fd(m_nlh_control.get());
			fcntl(netlink_fd, F_SETFD, FD_CLOEXEC);
		}
		{
			int netlink_fd = nl_socket_get_fd(m_nlh_watcher.get());
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
		nl_cache_ptr<::rtnl_link> linkCache = makeRTNLLinkCache(m_nlh_control.get(), AF_UNSPEC);
		cacheForeach<::rtnl_link>(linkCache, [this](::rtnl_link* link) {
			int const ifIndex{rtnl_link_get_ifindex(link)};
			QString const ifName = QString::fromUtf8(rtnl_link_get_name(link));
			if (ifIndex <= 0) return;
			if ((size_t)ifIndex >= ifStates.size()) ifStates.resize(ifIndex+1);
			if (!ifStates[ifIndex].exists) {
				ifStates[ifIndex].exists = true;
				ifStates[ifIndex].name = ifName;
				log << QString("discovered interface: %1 [index %2]").arg(ifName).arg(ifIndex) << endl;
				emit newDevice(ifName, ifIndex);
			}
		});
	}

	libnutcommon::MacAddress HardwareManager::getMacAddress(QString const& ifName) {
		struct ifreq ifr;
		if (!ifreq_init(ifr, ifName)) {
			err << QString("Interface name too long") << endl;
			return libnutcommon::MacAddress();
		}
		if (ioctl(ethtool_fd, SIOCGIFHWADDR, &ifr) < 0) {
			err << QString("Couldn't get hardware address of '%1'").arg(ifName) << endl;
			return libnutcommon::MacAddress();
		}
		return libnutcommon::MacAddress::fromBuffer(ifr.ifr_hwaddr.sa_data);
	}

	bool HardwareManager::ifreq_init(struct ifreq& ifr, QString const& ifname) {
		QByteArray buf = ifname.toUtf8();
		if (buf.size() >= IFNAMSIZ) return false;
		ifreq_init(ifr);
		strncpy (ifr.ifr_name, buf.constData(), buf.size());
		return true;
	}
	void HardwareManager::ifreq_init(struct ifreq& ifr) {
		memset(reinterpret_cast<char*>(&ifr), 0, sizeof(ifr));
	}

	void HardwareManager::read_netlinkmsgs() {
		int res = nl_recvmsgs_default(m_nlh_watcher.get());
		if (0 > res) {
			log << "nl_recvmsgs failed: " << nl_geterror(res) << " (" <<  res << ")" << endl;
		}
		return;
	}

	int HardwareManager::wrap_handle_netlinkmsg(::nl_msg *msg, void* arg) {
		return reinterpret_cast<HardwareManager*>(arg)->handle_netlinkmsg(msg);
	}

	int HardwareManager::handle_netlinkmsg(::nl_msg *msg) {
		switch (nlmsg_hdr(msg)->nlmsg_type) {
		case RTM_NEWLINK:
			nlMsgParseType<::rtnl_link>(msg, [this](::rtnl_link* link) {
				int const ifIndex = rtnl_link_get_ifindex(link);
				if (ifIndex <= 0) {
					err << QString("invalid interface index %1").arg(ifIndex) << endl;
					return;
				}
				if ((size_t) ifIndex >= ifStates.size()) ifStates.resize(ifIndex+1);
				QString ifName = QString::fromUtf8(rtnl_link_get_name(link));
				if (ifName.isEmpty()) {
					if (ifStates[ifIndex].exists) {
						ifName = ifStates[ifIndex].name;
					}
				}
				if (ifName.isEmpty()) {
					err << QString("couldn't find name for (new) interface %1").arg(ifIndex) << endl;
					return;
				}
				if (!ifStates[ifIndex].exists) {
					ifStates[ifIndex].exists = true;
					ifStates[ifIndex].name = ifName;
					log << QString("new interface detected: %1 [index %2]").arg(ifName).arg(ifIndex) << endl;
					emit newDevice(ifName, ifIndex);
				} else if (ifStates[ifIndex].name != ifName) {
					QString oldName = ifStates[ifIndex].name;
					ifStates[ifIndex].name = ifName;
					log << QString("interface rename detected: %1 -> %2 [index %3]").arg(oldName).arg(ifName).arg(ifIndex) << endl;
					emit delDevice(oldName);
					emit newDevice(ifName, ifIndex);
				}
				bool carrier = (rtnl_link_get_flags(link) & IFF_LOWER_UP) > 0;
				if (carrier != ifStates[ifIndex].carrier) {
					ifStates[ifIndex].carrier = carrier;
					if (!isControlled(ifIndex))
						return;
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

			});
			break;
		case RTM_DELLINK:
			nlMsgParseType<::rtnl_link>(msg, [this](::rtnl_link* link) {
				int const ifIndex = rtnl_link_get_ifindex(link);
				if (ifIndex <= 0) {
					err << QString("invalid interface index %1").arg(ifIndex) << endl;
					return;
				}
				QString const ifName = QString::fromUtf8(rtnl_link_get_name(link));
				if (ifName.isNull()) return;
				log << QString("lost interface: %1 [index %2]").arg(ifName).arg(ifIndex) << endl;
				if ((size_t) ifIndex < ifStates.size()) {
					if (ifStates[ifIndex].exists) {
						if (ifStates[ifIndex].carrier)
							emit lostCarrier(ifName);
						ifStates[ifIndex].exists = false;
						ifStates[ifIndex].carrier = false;
						ifStates[ifIndex].active = false;
						ifStates[ifIndex].name.clear();
					}
				}
				emit delDevice(ifName);
			});
			break;
		case RTM_NEWROUTE:
			bool const replacedRoute = 0 != (NLM_F_REPLACE & nlmsg_hdr(msg)->nlmsg_flags);
			nlMsgParseType<::rtnl_route>(msg, [this,replacedRoute](::rtnl_route* route) {
#if 0
				{
					char buf[512];
					nl_object_dump_buf(nl_safe_to_object(route), buf, sizeof(buf));
					log << (replacedRoute ? "Replaced " : "New ") << "route with metric " << rtnl_route_get_priority(route) << ": " << buf;
				}
#endif
				checkRouteMetric(route);
			});
			break;
		}

		return 0;
	}

	void HardwareManager::checkRouteMetric(::rtnl_route* route) {
		switch (rtnl_route_get_family(route)) {
		case AF_INET6:
			{
				/* don't touch IPv6 link-local fe80::/64 */
				static const uint8_t link_local_prefix[8] =
					{ 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
				nl_addr* dst = rtnl_route_get_dst(route);
				if (64 == nl_addr_get_prefixlen(dst) && 0 == memcmp(link_local_prefix, nl_addr_get_binary_addr(dst), 8)) return;
			}
			break;
		case AF_INET:
			/* don't touch default routes: the metric for those is
			 * set explicitly
			 */
			{
				nl_addr* dst = rtnl_route_get_dst(route);
				if (0 == nl_addr_get_prefixlen(dst)) return;
			}
			break;
		default:
			return;
		}

		if (RT_TABLE_MAIN != rtnl_route_get_table(route)) return;
		if (RTN_UNICAST != rtnl_route_get_type(route)) return;

		if (1 != rtnl_route_get_nnexthops(route)) return;
		::rtnl_nexthop* nh = rtnl_route_nexthop_n(route, 0);
		int const ifIndex = rtnl_route_nh_get_ifindex(nh);

		if (!isControlled(ifIndex)) return;

		int const metric = ifStates[ifIndex].metric;
		if (-1 == metric || static_cast<uint32_t>(metric) == rtnl_route_get_priority(route)) return;

		char route_str[512];
		nl_object_dump_buf(nl_safe_to_object(route), route_str, sizeof(route_str));

		log << "Replacing route with updated metric " << metric << ": " << route_str;
		log.flush();

		int err;
		err = rtnl_route_delete(m_nlh_control.get(), route, NLM_F_REPLACE);
		if (0 > err && -ENOENT != err) {
			log << "Removing route failed: " << nl_geterror(err) << " (" <<  err << ")" << endl;
			return;
		}

		rtnl_route_set_priority(route, metric);

		err = rtnl_route_add(m_nlh_control.get(), route, NLM_F_REPLACE);
		if (0 > err && -ENOENT != err) {
			log << "Adding route with updated metric failed: " << nl_geterror(err) << " (" <<  err << ")" << endl;
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
		strncpy (iwr.ifr_ifrn.ifrn_name, buf.constData(), buf.size());
		return true;
	}

	bool HardwareManager::hasWLAN(QString const& ifName) {
		struct iwreq iwr;
		iwreq_init(iwr, ifName);
		if (ioctl(ethtool_fd, SIOCGIWNAME, &iwr) < 0) return false;
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
		if (ioctl(ethtool_fd, SIOCGIWESSID, &iwr) < 0) return false;
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
		int res;

		res = rtnl_addr_add(m_nlh_control.get(), makeRTNLAddrIPv4(ifIndex, ip, netmask).get(), 0);
		if (0 > res) {
			log << "Adding address failed [ndx=" << ifIndex << "]: " << nl_geterror(res) << " (" <<  res << ")" << endl;
		}

		if (!gateway.isNull()) {
			/* NLM_F_EXCL would fail if there is already a route with the
			 * same target,tos,priority
			 *
			 * For now overwrite existing routes.
			 */
			res = rtnl_route_add(m_nlh_control.get(), makeRTNLRouteIPv4(ifIndex, gateway, metric).get(), 0 /* NLM_F_EXCL */);
			if (0 > res) {
				log << "Adding default gateway failed [ndx=" << ifIndex << "]: " << nl_geterror(res) << " (" <<  res << ")" << endl;
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

		int res;

		if (!gateway.isNull()) {
			res = rtnl_route_delete(m_nlh_control.get(), makeRTNLRouteIPv4(ifIndex, gateway, metric).get(), 0);
			if (0 > res) {
				log << "Removing default gateway failed [ndx=" << ifIndex << "]: " << nl_geterror(res) << " (" <<  res << ")" << endl;
			}
		}

		res = rtnl_addr_delete(m_nlh_control.get(), makeRTNLAddrIPv4(ifIndex, ip, netmask).get(), 0);
		if (0 > res) {
			log << "Removing address failed [ndx=" << ifIndex << "]: " << nl_geterror(res) << " (" <<  res << ")" << endl;
		}
	}

	void HardwareManager::setMetric(int ifIndex, int metric) {
		if (!isControlled(ifIndex)) return;
		if (metric == ifStates[ifIndex].metric) return;
		ifStates[ifIndex].metric = metric;

		/* update "all" routes */
		std::list<rtnl_route_ptr> routes;
		int res;
		res = appendRoutes(m_nlh_control.get(), AF_INET, routes);
		if (0 > res) {
			log << "appendRoutes failed: " << nl_geterror(res) << " (" <<  res << ")" << endl;
		}
		res = appendRoutes(m_nlh_control.get(), AF_INET6, routes);
		if (0 > res) {
			log << "appendRoutes failed: " << nl_geterror(res) << " (" <<  res << ")" << endl;
		}
		for (auto const& route: routes) {
			checkRouteMetric(route.get());
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
			bool removeTemporaries{false};

			auto removeIPv6AddressWithFlag = [this, ifIndex, &removeTemporaries](::rtnl_addr* addr) {
#if 0
				{
					char buf[512];
					nl_object_dump_buf(nl_safe_to_object(addr), buf, sizeof(buf));
					log << "Found address: " << buf;
				}
#endif

				/* remove all IPv6 addresses on interface (we "own" it) */
				if (rtnl_addr_get_ifindex(addr) != ifIndex) return;
				if (rtnl_addr_get_family(addr) != AF_INET6) return;

				switch (rtnl_addr_get_scope(addr)) {
				case RT_SCOPE_LINK:
					/* remember link-local address to restart router solicitation */
					ifStates[ifIndex].link_local_ipv6_reenable = nlCopyObjectPtr(addr);
					break;
				case RT_SCOPE_UNIVERSE:
					/* IFA_F_TEMPORARY (from privacy extension) should get removed automatically after its IFA_F_MANAGETEMPADDR is deleted */
					if (!removeTemporaries && 0 == (rtnl_addr_get_flags(addr) & IFA_F_MANAGETEMPADDR)) return;
					break;
				default:
					break;
				}

				log << "Removing IPv6 address "
					<< toString(rtnl_addr_get_local(addr))
					<< " [ndx=" << ifIndex << "]"
					<< endl;

				int res = rtnl_addr_delete(m_nlh_control.get(), addr, 0);
				if (0 > res) {
					log << "Removing IPv6 address "
						<< toString(rtnl_addr_get_local(addr))
						<< " failed [ndx=" << ifIndex << "]: " << nl_geterror(res) << " (" <<  res << ")" << endl;
				}
			};

			nl_cache_ptr<::rtnl_addr> cache;
			if (int res = makeRTNLAddrCache(cache, m_nlh_control.get())) {
				log << "Getting addresses failed: " << nl_geterror(res) << " (" <<  res << ")" << endl;
			}
			cacheForeach<::rtnl_addr>(cache, removeIPv6AddressWithFlag);

			cache.reset();
			removeTemporaries = true;

			if (int res = makeRTNLAddrCache(cache, m_nlh_control.get())) {
				log << "Getting addresses failed: " << nl_geterror(res) << " (" <<  res << ")" << endl;
			}
			cacheForeach<::rtnl_addr>(cache, removeIPv6AddressWithFlag);
		}

		{
			std::list<rtnl_route_ptr> routes;
			int res = appendRoutes(m_nlh_control.get(), AF_INET6, routes);
			if (0 > res) {
				log << "appendRoutes failed: " << nl_geterror(res) << " (" <<  res << ")" << endl;
			}
			for (auto const& route: routes) {
#if 0
				{
					char buf[512];
					nl_object_dump_buf(nl_safe_to_object(route.get()), buf, sizeof(buf));
					log << "Found IPv6 route: " << buf;
				}
#endif

				::nl_addr* dst = rtnl_route_get_dst(route.get());

				if (1 != rtnl_route_get_nnexthops(route.get())) continue;

				::rtnl_nexthop* nh = rtnl_route_nexthop_n(route.get(), 0);
				if (ifIndex != rtnl_route_nh_get_ifindex(nh)) continue;

				log << "Removing IPv6 route to "
					<< routeTargetToString(dst)
					<< " via "
					<< toString(rtnl_route_nh_get_gateway(nh))
					<< " [ndx=" << ifIndex << "]"
					<< endl;

				int res = rtnl_route_delete(m_nlh_control.get(), route.get(), 0);
				if (0 > res) {
					log << "Removing IPv6 route to "
						<< routeTargetToString(dst)
						<< " via "
						<< toString(rtnl_route_nh_get_gateway(nh))
						<< " failed [ndx=" << ifIndex << "]: " << nl_geterror(res) << " (" <<  res << ")" << endl;
				}
			}
		}
	}

	void HardwareManager::reenableIPv6(int ifIndex) {
		if (!isControlled(ifIndex)) return;

		rtnl_addr_ptr addr = std::move(ifStates[ifIndex].link_local_ipv6_reenable);
		if (!addr) return;

		log << "Readding IPv6 link-local address "
			<< toString(rtnl_addr_get_local(addr.get()))
			<< " [ndx=" << ifIndex << "]"
			<< endl;

		int res = rtnl_addr_add(m_nlh_control.get(), addr.get(), 0);
		if (0 > res) {
			log << "Readding IPv6 link-local address "
				<< toString(rtnl_addr_get_local(addr.get()))
				<< " failed [ndx=" << ifIndex << "]: " << nl_geterror(res) << " (" <<  res << ")" << endl;
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
