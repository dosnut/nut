//
// C++ Implementation: hardware
//
// Description: 
//
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "hardware.h"
#include "exception.h"
#include "log.h"

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
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/ethtool.h>
#include <linux/sysctl.h>
// hardware_ext.c
extern struct nla_policy ifla_policy[IFLA_MAX+1];
extern struct nla_policy ifa_ipv4_policy[IFA_MAX+1];
};

#include <QSocketNotifier>
#include <QApplication>

namespace nuts {
	HardwareManager::HardwareManager()
	: netlink_fd(-1), ethtool_fd(-1) {
		if (!init_netlink()) {
			throw NetlinkInitException("HardwareManager: Couldn't init netlink");
		}
		if (!init_ethtool()) {
			free_netlink();
			throw EthtoolInitException("HardwareManager: Couldn't init ethtool");
		}
		fcntl(netlink_fd, F_SETFD, FD_CLOEXEC);
		fcntl(ethtool_fd, F_SETFD, FD_CLOEXEC);
		QSocketNotifier *nln = new QSocketNotifier(netlink_fd, QSocketNotifier::Read, this);
		connect(nln, SIGNAL(activated(int)), this, SLOT(read_netlinkmsgs()));
	}
	
	HardwareManager::~HardwareManager() {
		free_ethtool();
		free_netlink();
	}
	
	bool HardwareManager::controlOn(int ifIndex, bool force) {
		if (ifIndex < 0) return false;
		if (!ifup(ifIndex2Name(ifIndex), force))
			return false;
		if (ifIndex >= ifStates.size())
			ifStates.resize(ifIndex+1);
		ifStates[ifIndex] = ifstate(true);
		return true;
	}
	bool HardwareManager::controlOff(int ifIndex) {
		if (ifIndex < 0) return false;
		if (ifIndex < ifStates.size() && ifStates[ifIndex].active) {
			ifStates[ifIndex].active = false;
			ifdown(ifIndex2Name(ifIndex));
		}
		return true;
	}
	
	bool HardwareManager::init_netlink() {
		nlh = nl_handle_alloc();
		if (!nlh) return false;
		nl_handle_set_peer_pid(nlh, 0);
		
		if (nl_connect(nlh, NETLINK_ROUTE) != 0) goto cleanup;
		if (nl_join_group(nlh, RTNLGRP_LINK) != 0) goto cleanup;
		if (nl_join_group(nlh, RTNLGRP_IPV4_IFADDR) != 0) goto cleanup;
		
		netlink_fd = nl_handle_get_fd(nlh);
		return true;
	cleanup:
		free_netlink();
		return false;
	}
	void HardwareManager::free_netlink() {
		nl_close(nlh);
		nl_handle_destroy(nlh);
	}
	bool HardwareManager::init_ethtool() {
		ethtool_fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
		if (ethtool_fd >= 0) return true;
/*
		ethtool_fd = socket(PF_INET, SOCK_DGRAM, 0);
		if (ethtool_fd >= 0) return true;
		ethtool_fd = socket(PF_PACKET, SOCK_DGRAM, 0);
		if (ethtool_fd >= 0) return true;
		ethtool_fd = socket(PF_INET6, SOCK_DGRAM, 0);
		if (ethtool_fd >= 0) return true;
*/
		return false;
	}
	void HardwareManager::free_ethtool() {
		close(ethtool_fd);
	}
	
	bool HardwareManager::ifup(const QString &ifname, bool force) {
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
				err << QString("Couldn't set flags for interface '%1'").arg(ifname);
				return false;
			}
		}
		ifr.ifr_flags |= IFF_UP;
		if (ioctl(ethtool_fd, SIOCSIFFLAGS, &ifr) < 0) {
			err << QString("Couldn't set flags for interface '%1'").arg(ifname);
			return false;
		}
		return true;
	}
	bool HardwareManager::ifdown(const QString &ifname) {
		return true;
	}
	QString HardwareManager::ifIndex2Name(int ifIndex) {
		return "";
	}
	int HardwareManager::ifName2Index(const QString &ifName) {
		return 0;
	}
	
	bool HardwareManager::ifreq_init(struct ifreq &ifr, const QString &ifname) {
		QByteArray buf = ifname.toUtf8();
		if (buf.size() >= IFNAMSIZ) return false;
		memset((char*) &ifr, 0, sizeof(ifr));
		strncpy (ifr.ifr_name, buf.constData(), sizeof(ifr.ifr_name)-1);
		return true;
	}
	
	void HardwareManager::read_netlinkmsgs() {
		struct sockaddr_nl peer;
		unsigned char *msg;
		int n;
		struct nlmsghdr *hdr;
		
		n = nl_recv(nlh, &peer, &msg, 0);
		for (hdr = (struct nlmsghdr*) msg; nlmsg_ok(hdr, n); hdr = (struct nlmsghdr*) nlmsg_next(hdr, &n)) {
			log << QString("Message type 0x%1").arg(hdr->nlmsg_type, 0, 16) << endl;
			switch (hdr->nlmsg_type) {
				case RTM_NEWLINK:
					struct ifinfomsg *ifm = (struct ifinfomsg*) nlmsg_data(hdr);
					struct nlattr *tb[IFLA_MAX+1];
					if (nlmsg_parse(hdr, sizeof(*ifm), tb, IFLA_MAX, ifla_policy) < 0) {
						break;
					}
					if (!isControlled(ifm->ifi_index))
						break;
					log << QString("RTM_NEWLINK: %1 (%2)\n")
							.arg(tb[IFLA_IFNAME] ? (char*) nla_data(tb[IFLA_IFNAME]) : "<unknown>")
							.arg(ifm->ifi_index);
					bool carrier = (ifm->ifi_flags & IFF_LOWER_UP) > 0;
					if (carrier != ifStates[ifm->ifi_index].carrier) {
						ifStates[ifm->ifi_index].carrier = carrier;
						if (carrier)
							emit gotCarrier(ifm->ifi_index);
						else
							emit lostCarrier(ifm->ifi_index);
					}
					break;
			}
		}
	}
	bool HardwareManager::isControlled(int ifIndex) {
		if (ifIndex < 0) return false;
		if (ifIndex >= ifStates.size()) return false;
		return ifStates[ifIndex].active;
	}
};
