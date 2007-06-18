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
// hardware_ext.c
extern struct nla_policy ifla_policy[IFLA_MAX+1];
extern struct nla_policy ifa_ipv4_policy[IFA_MAX+1];
};

#include <QSocketNotifier>
#include <QApplication>
#include <iostream>

namespace nuts {
	HardwareManager::HardwareManager()
	: netlink_fd(-1), ethtool_fd(-1) {
		if (!init_netlink()) {
			std::cerr << "Couldn't init netlink" << std::endl;
			QApplication::exit(-1);
			return;
		}
		if (!init_ethtool()) {
			free_netlink();
			std::cerr << "Couldn't init ethtool" << std::endl;
			QApplication::exit(-1);
			return;
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
	
	void HardwareManager::setMonitor(int ifIndex) {
		if (ifIndex < 0) return;
		if (ifIndex >= ifMonitor.size())
			ifMonitor.resize(ifIndex+1);
		ifMonitor.setBit(ifIndex);
	}
	void HardwareManager::clearMonitor(int ifIndex) {
		if (ifIndex < 0) return;
		if (ifIndex < ifMonitor.size())
			ifMonitor.clearBit(ifIndex);
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
	
	void HardwareManager::read_netlinkmsgs() {
		printf("Netlink message\n");
		struct sockaddr_nl peer;
		unsigned char *msg;
		int n;
		struct nlmsghdr *hdr;
		
		n = nl_recv(nlh, &peer, &msg, 0);
		for (hdr = (struct nlmsghdr*) msg; nlmsg_ok(hdr, n); hdr = (struct nlmsghdr*) nlmsg_next(hdr, &n)) {
			printf("Message type 0x%x\n", hdr->nlmsg_type);
			switch (hdr->nlmsg_type) {
				case RTM_NEWLINK:
					struct ifinfomsg *ifm = (struct ifinfomsg*) nlmsg_data(hdr);
					struct nlattr *tb[IFLA_MAX+1];
					if (nlmsg_parse(hdr, sizeof(*ifm), tb, IFLA_MAX, ifla_policy) < 0) {
						break;
					}
					if (!isMonitored(ifm->ifi_index))
						break;
					printf("RTM_NEWLINK: %s (%i)\n",
						   tb[IFLA_IFNAME] ? (char*) nla_data(tb[IFLA_IFNAME]) : "<unknown>", ifm->ifi_index);

					int carrier = (ifm->ifi_flags & IFF_LOWER_UP) > 0;
					if (carrier)
						emit gotCarrier(ifm->ifi_index);
					else
						emit lostCarrier(ifm->ifi_index);
					break;
			}
		}
	}
	bool HardwareManager::isMonitored(int ifIndex) {
		if (ifIndex < 0) return false;
		if (ifIndex >= ifMonitor.size()) return false;
		return ifMonitor[ifIndex];
	}
};
