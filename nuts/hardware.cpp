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

	nuts::nl_addr_ptr NLAddrDefaultIPv4() {
		quint32 i = 0;
		nuts::nl_addr_ptr a{nl_addr_build(AF_INET, &i, sizeof(i))};
		nl_addr_set_prefixlen(a.get(), 0);
		return a;
	}

	nuts::nl_addr_ptr toNLAddrIPv4(QHostAddress const& addr, QHostAddress const& netmask = QHostAddress()) {
		quint32 i = htonl(addr.toIPv4Address());
		nuts::nl_addr_ptr a{nl_addr_build(AF_INET, &i, sizeof(i))};
		if (!netmask.isNull()) {
			nl_addr_set_prefixlen(a.get(), getPrefixLenIPv4(netmask));
		}
		return a;
	}

	nuts::nl_addr_ptr toNLBroadcastIPv4(const QHostAddress &addr, const QHostAddress &netmask) {
		quint32 nm = 0;
		if (!netmask.isNull()) nm = htonl(netmask.toIPv4Address());
		quint32 i = htonl(addr.toIPv4Address());
		quint32 bcast = (i & nm) | (~nm);
		nuts::nl_addr_ptr a{nl_addr_build(AF_INET, &bcast, sizeof(bcast))};
		return a;
	}

	nuts::rtnl_addr_ptr makeRTNLAddrIPv4(int ifIndex, QHostAddress const& ip, QHostAddress const& netmask) {
		// zeroconf: in 169.254.0.0/16
		bool const isZeroconf = (netmask.toIPv4Address() == 0xFFFF0000u)
			&& ((ip.toIPv4Address() & 0xFFFF0000u) == 0xA9FE0000);

		nuts::rtnl_addr_ptr addr{rtnl_addr_alloc()};
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

	nuts::rtnl_route_ptr makeRTNLRouteIPv4(int ifIndex, QHostAddress const& gateway, int metric) {
		nuts::rtnl_nexthop_ptr nh{rtnl_route_nh_alloc()};
		rtnl_route_nh_set_ifindex(nh.get(), ifIndex);
		rtnl_route_nh_set_gateway(nh.get(), toNLAddrIPv4(gateway).get());

		nuts::rtnl_route_ptr route{rtnl_route_alloc()};
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
}

namespace nuts {
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
		return controlOn(ifIndex, ifIndex2Name(ifIndex), force);
	}
	bool HardwareManager::controlOff(int ifIndex) {
		return controlOff(ifIndex, ifIndex2Name(ifIndex));
	}
	bool HardwareManager::controlOn(QString const& ifName, bool force) {
		return controlOn(ifName2Index(ifName), ifName, force);
	}
	bool HardwareManager::controlOff(QString const& ifName) {
		return controlOff(ifName2Index(ifName), ifName);
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

		{
			struct ::nl_cache* cache_ptr{nullptr};
			if (0 == rtnl_link_alloc_cache(m_nlh_watcher.get(), AF_UNSPEC, &cache_ptr)) {
				m_nlcache.reset(cache_ptr);
			}
		}
		if (!m_nlcache) {
			goto cleanup;
		}

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
/*		nl_cache_update(nlh, nlcache);
		struct rtnl_link* request = rtnl_link_alloc();
		rtnl_link_set_flags(request, rtnl_link_str2flags("up"));
		QByteArray buf = ifname.toUtf8();
		struct rtnl_link* old = rtnl_link_get_by_name(nlcache, buf.constData());
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
		unsigned char* msg;
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
						QString ifname = ifIndex2Name(ifindex);
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
			log << "Adding address failed[ndx=" << ifIndex << "]: " << nl_geterror(res) << " (" <<  res << ")" << endl;
		}

		if (!gateway.isNull()) {
			/* NLM_F_EXCL would fail if there is already a route with the
			 * same target,tos,priority
			 *
			 * For now overwrite existing routes.
			 */
			res = rtnl_route_add(m_nlh_control.get(), makeRTNLRouteIPv4(ifIndex, gateway, metric).get(), 0 /* NLM_F_EXCL */);
			if (0 > res) {
				log << "Adding default gateway failed[ndx=" << ifIndex << "]: " << nl_geterror(res) << " (" <<  res << ")" << endl;
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
		int res;

		if (!gateway.isNull()) {
			res = rtnl_route_delete(m_nlh_control.get(), makeRTNLRouteIPv4(ifIndex, gateway, metric).get(), 0);
			if (0 > res) {
				log << "Deleting default gateway failed[ndx=" << ifIndex << "]: " << nl_geterror(res) << " (" <<  res << ")" << endl;
			}
		}

		res = rtnl_addr_delete(m_nlh_control.get(), makeRTNLAddrIPv4(ifIndex, ip, netmask).get(), 0);
		if (0 > res) {
			log << "Deleting address failed[ndx=" << ifIndex << "]: " << nl_geterror(res) << " (" <<  res << ")" << endl;
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
