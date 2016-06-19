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

// hardware_ext.c
extern struct nla_policy ifla_policy[IFLA_MAX+1];
extern struct nla_policy ifa_ipv4_policy[IFA_MAX+1];
}

#include <QSocketNotifier>
#include <QFile>

namespace nuts {
	namespace {
		QString read_IFLA_IFNAME(struct nlattr* (&tb)[IFLA_MAX+1]) {
			struct nlattr* nla_ifname = tb[IFLA_IFNAME];
			if (nullptr == nla_ifname) return QString();
			return QString::fromUtf8((char*) nla_data(nla_ifname), nla_len(nla_ifname)-1);
		}

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
				rtnl_route_set_priority(route.get(), metric);
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

		nl_cache_ptr<::rtnl_addr> makeRTNLAddrCache(::nl_sock* sock) {
			::nl_cache* cache{nullptr};
			if (0 == rtnl_addr_alloc_cache(sock, &cache)) {
				return nl_cache_ptr<::rtnl_addr>{cache};
			}
			return nl_cache_ptr<::rtnl_addr>{};
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

			{
				struct sockaddr_nl peer;
				unsigned char* buf{nullptr};
				int n = nl_recv(sock, &peer, &buf, nullptr);
				if (n <= 0) return n;

				for (nlmsghdr* hdr = reinterpret_cast<nlmsghdr*>(buf); nlmsg_ok(hdr, n); hdr = nlmsg_next(hdr, &n)) {
					switch (hdr->nlmsg_type) {
					case NLMSG_DONE: /* terminates multipart messages */
						return 0;
					case NLMSG_NOOP: /* ignore */
						break;
					case NLMSG_OVERRUN:
						return -NLE_MSG_OVERFLOW;
					case NLMSG_ERROR:
						{
							nlmsgerr* e = reinterpret_cast<nlmsgerr*>(nlmsg_data(hdr));
							if (hdr->nlmsg_len < (unsigned int)nlmsg_size(sizeof(*e))) {
								return -NLE_MSG_TRUNC;
							} else if (e->error) {
								return -nl_syserr2nlerr(e->error);
							}
						}
						break;
					case RTM_NEWROUTE:
						{
							::rtnl_route* route{nullptr};
							err = rtnl_route_parse(hdr, &route);
							if (0 > err) return err;
							list.push_back(rtnl_route_ptr{route});
						}
						break;
					default:
						return -NLE_MSGTYPE_NOSUPPORT;
					}
				}
				free(buf);
			}
			return 0;
		}

		template<typename ObjectType>
		void internal_cacheForeach(::nl_object* obj, void *cb_ptr) {
			std::function<void(ObjectType*)> const& cb = *reinterpret_cast<std::function<void(ObjectType*)> const*>(cb_ptr);
			cb(reinterpret_cast<ObjectType*>(obj));
		}

		template<typename ObjectType>
		void cacheForeach(nl_cache_ptr<ObjectType>const& cache, std::function<void(ObjectType*)> const& cb) {
			void *cb_ptr = const_cast<void*>(reinterpret_cast<void const*>(&cb));
			nl_cache_foreach(cache.get(), &internal_cacheForeach<ObjectType>, cb_ptr);
		}

		template<typename ObjectType>
		void cacheForeachFilter(nl_cache_ptr<ObjectType>const& cache, ObjectType const* filter, std::function<void(ObjectType*)> const& cb) {
			void *cb_ptr = const_cast<void*>(reinterpret_cast<void const*>(&cb));
			nl_cache_foreach_filter(cache.get(), const_cast<nl_object*>(reinterpret_cast<nl_object const*>(filter)), &internal_cacheForeach<ObjectType>, cb_ptr);
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

		void free_nl_data::operator()(nl_sock* sock) {
			nl_close(sock);
			nl_socket_free(sock);
		}

		void free_nl_data::operator()(rtnl_addr* addr) {
			rtnl_addr_put(addr);
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
		QString ifName = ifIndex2Name(ifIndex);
		if (ifName.isEmpty()) return false;
		return controlOn(ifIndex, ifName, force);
	}
	bool HardwareManager::controlOff(int ifIndex) {
		if (!isControlled(ifIndex)) return true; /* probably got deleted, skip cleanup */
		QString ifName = ifIndex2Name(ifIndex);
		if (ifName.isEmpty()) return false;
		return controlOff(ifIndex, ifName);
	}
	bool HardwareManager::controlOn(QString const& ifName, bool force) {
		int ifIndex = ifName2Index(ifName);
		if (-1 == ifIndex) return false;
		return controlOn(ifIndex, ifName, force);
	}
	bool HardwareManager::controlOff(QString const& ifName) {
		int ifIndex = ifName2Index(ifName);
		if (-1 == ifIndex) return false;
		return controlOff(ifIndex, ifName);
	}

	bool HardwareManager::controlOn(int ifIndex, QString const& ifName, bool force) {
		if (ifIndex < 0 || ifIndex >= ifStates.size()) return false;
		if (!ifup(ifName, force)) return false;

		ifStates[ifIndex].on();
//		log << "activated interface " << ifIndex << endl;
		return true;
	}

	bool HardwareManager::controlOff(int ifIndex, QString const& ifName) {
		if (ifIndex < 0) return false;
		if (ifIndex < ifStates.size() && ifStates[ifIndex].active) {
			ifStates[ifIndex].off();
			ifdown(ifName);
		}
		return true;
	}

	bool HardwareManager::init_netlink() {
		m_nlh_control.reset(nl_socket_alloc());
		m_nlh_watcher.reset(nl_socket_alloc());
		if (!m_nlh_watcher || !m_nlh_control) goto cleanup;
		nl_socket_set_peer_port(m_nlh_watcher.get(), 0);

		if (nl_connect(m_nlh_control.get(), NETLINK_ROUTE) != 0) goto cleanup;

		if (nl_connect(m_nlh_watcher.get(), NETLINK_ROUTE) != 0) goto cleanup;
		if (nl_socket_add_membership(m_nlh_watcher.get(), RTNLGRP_LINK) != 0) goto cleanup;
		if (nl_socket_add_membership(m_nlh_watcher.get(), RTNLGRP_IPV4_IFADDR) != 0) goto cleanup;

		m_nlcache = makeRTNLLinkCache(m_nlh_control.get(), AF_UNSPEC);
		if (!m_nlcache) goto cleanup;

		{
			int netlink_fd = nl_socket_get_fd(m_nlh_control.get());
			fcntl(netlink_fd, F_SETFD, FD_CLOEXEC);
		}
		{
			int netlink_fd = nl_socket_get_fd(m_nlh_watcher.get());
			fcntl(netlink_fd, F_SETFD, FD_CLOEXEC);
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
		m_nlcache.reset();
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

	bool HardwareManager::ifup(QString const& ifname, bool force) {
/*		nl_cache_update(nlh, m_nlcache);
		struct rtnl_link* request = rtnl_link_alloc();
		rtnl_link_set_flags(request, rtnl_link_str2flags("up"));
		QByteArray buf = ifname.toUtf8();
		struct rtnl_link* old = rtnl_link_get_by_name(m_nlcache, buf.constData());
		rtnl_link_change(nlh, old, request, 0);
		rtnl_link_put(old);
		rtnl_link_put(request);
*/
		struct ifreq ifr;
		if (!ifreq_init(ifr, ifname)) {
			err << QString("Interface name too long") << endl;
			return false;
		}
		if (ioctl(ethtool_fd, SIOCGIFFLAGS, &ifr) < 0) {
			err << QString("Couldn't get flags for interface '%1'").arg(ifname) << endl;
			return false;
		}
		if (ifr.ifr_flags & IFF_UP) {
			if (!force) return false;
			// "restart" interface to get carrier event
			ifr.ifr_flags &= ~IFF_UP;
			if (ioctl(ethtool_fd, SIOCSIFFLAGS, &ifr) < 0) {
				err << QString("Couldn't set flags for interface '%1'").arg(ifname) << endl;
				return false;
			}
		}
		ifr.ifr_flags |= IFF_UP;
		if (ioctl(ethtool_fd, SIOCSIFFLAGS, &ifr) < 0) {
			err << QString("Couldn't set flags for interface '%1'").arg(ifname) << endl;
			return false;
		}
		return true;
	}
	bool HardwareManager::ifdown(QString const& ifname) {
		struct ifreq ifr;
		if (!ifreq_init(ifr, ifname)) {
			err << QString("Interface name too long") << endl;
			return false;
		}
		if (ioctl(ethtool_fd, SIOCGIFFLAGS, &ifr) < 0) {
			err << QString("Couldn't get flags for interface '%1'").arg(ifname) << endl;
			return false;
		}
		if (!(ifr.ifr_flags & IFF_UP)) {
			err << QString("Interface '%1' is not up").arg(ifname) << endl;
			return false;
		}
		ifr.ifr_flags &= ~IFF_UP;
		if (ioctl(ethtool_fd, SIOCSIFFLAGS, &ifr) < 0) {
			err << QString("Couldn't set flags for interface '%1'").arg(ifname) << endl;
			return false;
			}
		return true;
	}
	QString HardwareManager::ifIndex2Name(int ifIndex) {
		struct ifreq ifr;
		ifreq_init(ifr);
		ifr.ifr_ifindex = ifIndex;
		if (ioctl(ethtool_fd, SIOCGIFNAME, &ifr) < 0) {
			err << QString("Couldn't get interface name for device %1").arg(ifIndex) << endl;
			return QString();
		}
		return QString::fromUtf8(ifr.ifr_name, qstrnlen(ifr.ifr_name, IFNAMSIZ));
	}

	void HardwareManager::discover() {
		auto discovered = [this](QString const& ifName, int ifIndex) {
			if (ifIndex < 0) return;
			if (ifIndex >= ifStates.size()) ifStates.resize(ifIndex+1);
			if (!ifStates[ifIndex].exists) {
				ifStates[ifIndex].exists = true;
				ifStates[ifIndex].name = ifName;
				log << QString("discovered interface: %1 [index %2]").arg(ifName).arg(ifIndex) << endl;
				emit newDevice(ifName, ifIndex);
			}
		};

		QFile file("/proc/net/dev");
		if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QTextStream in(&file);
			QString ifs = in.readLine();
			ifs = in.readLine();
			ifs = in.readLine();
			while (!ifs.isEmpty())  {
				ifs.truncate(ifs.indexOf(":"));
				QString ifName(ifs.mid(ifs.lastIndexOf(" ")+1,ifs.size()));
				int ifIndex = ifName2Index(ifName);
				discovered(ifName, ifIndex);
				ifs = in.readLine();
			}
		}
		else {
			struct ifreq ifr;
			// using nuts on a pc with more than 32 interfaces is insane
			for (int i=0; i<32;i++) {
				ifreq_init(ifr);
				ifr.ifr_ifindex = i;
				if (ioctl(ethtool_fd, SIOCGIFNAME, &ifr) < 0) {
					continue;
				}
				QString ifName(QString::fromUtf8(ifr.ifr_name, qstrnlen(ifr.ifr_name, IFNAMSIZ)));
				discovered(ifName, i);
			}
		}
	}

	int HardwareManager::ifName2Index(QString const& ifName) {
		struct ifreq ifr;
		if (!ifreq_init(ifr, ifName)) {
			err << QString("Interface name too long") << endl;
			return -1;
		}
		if (ioctl(ethtool_fd, SIOCGIFINDEX, &ifr) < 0) {
			err << QString("Couldn't get interface index of '%1'").arg(ifName) << endl;
			return -1;
		}
		return ifr.ifr_ifindex;
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
		struct sockaddr_nl peer;
		unsigned char* msg{nullptr};
		int n, msgsize;
		struct nlmsghdr* hdr;

		msgsize = n = nl_recv(m_nlh_watcher.get(), &peer, &msg, 0);
		for (hdr = (struct nlmsghdr*) msg; nlmsg_ok(hdr, n); hdr = (struct nlmsghdr*) nlmsg_next(hdr, &n)) {
//			log << QString("Message type 0x%1").arg(hdr->nlmsg_type, 0, 16) << endl;
			switch (hdr->nlmsg_type) {
			case RTM_NEWLINK:
				/* new or modified link */
				{
					struct ifinfomsg* ifm = (struct ifinfomsg*) nlmsg_data(hdr);
					struct nlattr* tb[IFLA_MAX+1];
					if (nlmsg_parse(hdr, sizeof(*ifm), tb, IFLA_MAX, ifla_policy) < 0) {
						break;
					}
					int ifindex = ifm->ifi_index;
					if (ifindex >= ifStates.size())
						ifStates.resize(ifindex+1);
					QString ifname = read_IFLA_IFNAME(tb);
					if (ifname.isNull()) {
						if (ifStates[ifindex].exists) {
							ifname = ifStates[ifindex].name;
						} else {
							ifname = ifIndex2Name(ifindex);
						}
					}
					if (!ifStates[ifindex].exists) {
						ifStates[ifindex].exists = true;
						ifStates[ifindex].name = ifname;
						log << QString("new interface detected: %1 [index %2]").arg(ifname).arg(ifindex) << endl;
						emit newDevice(ifname, ifindex);
					} else if (ifStates[ifindex].name != ifname) {
						QString oldName = ifStates[ifindex].name;
						ifStates[ifindex].name = ifname;
						log << QString("interface rename detected: %1 -> %2 [index %3]").arg(oldName).arg(ifname).arg(ifindex) << endl;
						emit delDevice(oldName);
						emit newDevice(ifname, ifindex);
					}
					bool carrier = (ifm->ifi_flags & IFF_LOWER_UP) > 0;
					if (carrier != ifStates[ifindex].carrier) {
						ifStates[ifindex].carrier = carrier;
						if (!isControlled(ifindex))
							break;
						if (carrier) {
							QString essid;
							getEssid(ifname, essid);
							emit gotCarrier(ifname, ifindex, essid);
						} else
							emit lostCarrier(ifname);
					}
				}
				break;
			case RTM_DELLINK:
				{
					struct ifinfomsg* ifm = (struct ifinfomsg*) nlmsg_data(hdr);
					struct nlattr* tb[IFLA_MAX+1];
					if (nlmsg_parse(hdr, sizeof(*ifm), tb, IFLA_MAX, ifla_policy) < 0) {
						break;
					}
					int ifindex = ifm->ifi_index;
					QString ifname = read_IFLA_IFNAME(tb);
					if (ifname.isNull()) break;
					log << QString("lost interface: %1 [index %2]").arg(ifname).arg(ifindex) << endl;
					if (ifindex < ifStates.size()) {
						if (ifStates[ifindex].exists) {
							if (ifStates[ifindex].carrier)
								emit lostCarrier(ifname);
							ifStates[ifindex].exists = false;
							ifStates[ifindex].carrier = false;
							ifStates[ifindex].active = false;
							ifStates[ifindex].name.clear();
						}
					}
					emit delDevice(ifname);
				} break;
			}
		}
		if (msgsize > 0) free(msg);
	}
	bool HardwareManager::isControlled(int ifIndex) {
		if (ifIndex < 0) return false;
		if (ifIndex >= ifStates.size()) return false;
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


	/* NOT READY FOR USE YET
	 *
	 * Touching the IPv6 configuration breaks auto configuration;
	 * it won't start again in a sane way.
	 * (When the previous auto configured address would get
	 * refreshed it probably would heal itself, but so far
	 * I couldn't see a way to restart that timer)
	 */
	void HardwareManager::cleanupIPv6AutoAssigned(int ifIndex) {
		{
			auto removeIPv6AddressWithFlag = [this, ifIndex](::rtnl_addr* addr) {
#if 0
				{
					char buf[512];
					nl_object_dump_buf(reinterpret_cast<::nl_object*>(addr), buf, sizeof(buf));
					log << "Found address: " << buf;
				}
#endif

				/* looking for temporary global IPv6 addresses on the interface with limited life time */
				if (rtnl_addr_get_ifindex(addr) != ifIndex) return;
				if (rtnl_addr_get_family(addr) != AF_INET6) return;
				switch (rtnl_addr_get_scope(addr)) {
				case RT_SCOPE_UNIVERSE:
					/* IFA_F_TEMPORARY (from privacy extension) gets removed automatically after is IFA_F_MANAGETEMPADDR is deleted */
					if (0 == (rtnl_addr_get_flags(addr) & IFA_F_MANAGETEMPADDR)) return;
					if (0xFFFFFFFFU == rtnl_addr_get_valid_lifetime(addr)) return; /* forever valid */
					break;
				default:
					return;
				}

				log << "Removing temporary IPv6 address "
					<< toString(rtnl_addr_get_local(addr))
					<< " [ndx=" << ifIndex << "]"
					<< endl;

#if 0
				/* reducing lifetime didn't fix the kernel timer either */
				rtnl_addr_set_preferred_lifetime(addr, 1);
				rtnl_addr_set_valid_lifetime(addr, 1);
				int res = rtnl_addr_add(m_nlh_control.get(), addr, NLM_F_REPLACE);
#else
				int res = rtnl_addr_delete(m_nlh_control.get(), addr, 0);
#endif
				if (0 > res) {
					log << "Removing temporary IPv6 address "
						<< toString(rtnl_addr_get_local(addr))
						<< " failed [ndx=" << ifIndex << "]: " << nl_geterror(res) << " (" <<  res << ")" << endl;
				}
			};

			cacheForeach<::rtnl_addr>(makeRTNLAddrCache(m_nlh_control.get()), removeIPv6AddressWithFlag);
		}

		{
			std::list<rtnl_route_ptr> routes;
			int res = appendRoutes(m_nlh_control.get(), AF_INET6, routes);
			if (0 > res) {
				log << "appendRoutes failed: " << nl_geterror(res) << " (" <<  res << ")" << endl;
			}
			for (auto const& route: routes) {
#if 1
				{
					char buf[512];
					nl_object_dump_buf(reinterpret_cast<::nl_object*>(route.get()), buf, sizeof(buf));
					log << "Found IPv6 route: " << buf;
				}
#endif

				//if (RT_TABLE_MAIN != rtnl_route_get_table(route.get())) continue;
				::nl_addr* dst = rtnl_route_get_dst(route.get());

				// ignore link local routes; it seems the kernel doesn't like messing with "deep" stuff
				// and won't do auto IPv6 anymore
				if (64 == nl_addr_get_prefixlen(dst)) {
					const quint8 link_local_prefix[8] = { 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
					if (0 == memcmp(link_local_prefix, nl_addr_get_binary_addr(dst), 8)) continue;
				}

				if (1 != rtnl_route_get_nnexthops(route.get())) continue;
				/* libnl doesn't expose cacheinfo yet, otherwise we would ignore the route if 0 == expire;
				 * the cacheinfo expire value is in jiffies, but is zero if the route doesn't expire
				 */

				::rtnl_nexthop* nh = rtnl_route_nexthop_n(route.get(), 0);
				if (ifIndex != rtnl_route_nh_get_ifindex(nh)) continue;

				/* remove "linkdown" and "proto ra" routes */
				if ((RTPROT_RA != rtnl_route_get_protocol(route.get())
					&& 0 == (RTNH_F_LINKDOWN & rtnl_route_nh_get_flags(nh)))) continue;

				log << "Removing temporary IPv6 route to "
					<< routeTargetToString(dst)
					<< " via "
					<< toString(rtnl_route_nh_get_gateway(nh))
					<< " [ndx=" << ifIndex << "]"
					<< endl;

				int res = rtnl_route_delete(m_nlh_control.get(), route.get(), 0);
				if (0 > res) {
					log << "Removing temporary IPv6 route to "
						<< routeTargetToString(dst)
						<< " via "
						<< toString(rtnl_route_nh_get_gateway(nh))
						<< " failed [ndx=" << ifIndex << "]: " << nl_geterror(res) << " (" <<  res << ")" << endl;
				}
			}
		}
	}

	void HardwareManager::ifstate::on() {
		active = true;
		exists = true;
		carrier = false;
	}

	void HardwareManager::ifstate::off() {
		active = false;
	}
}
