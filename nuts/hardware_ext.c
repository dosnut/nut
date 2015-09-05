/*
// C++ Implementation: hardware_ext.c
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
*/

#include <asm/types.h>
/* socket, AF_INET, SOCK_RAW */
#include <sys/socket.h>
/* IPPROTO_RAW */
#include <arpa/inet.h>
/* fcntl, F_SETFD, FD_CLOEXEC */
#include <fcntl.h>

/* suppress warning in netlink/addr.h */
struct addrinfo;

#include <netlink/netlink.h>
#include <netlink/msg.h>

#include <net/if.h>
#include <arpa/inet.h>

struct nla_policy ifla_policy[IFLA_MAX+1] = {
	[IFLA_IFNAME]           = { .type = NLA_STRING, .maxlen = IFNAMSIZ-1 },
	[IFLA_MAP]              = { .minlen = sizeof(struct rtnl_link_ifmap) },
	[IFLA_MTU]              = { .type = NLA_U32 },
	[IFLA_TXQLEN]           = { .type = NLA_U32 },
	[IFLA_WEIGHT]           = { .type = NLA_U32 },
/*    [IFLA_OPERSTATE]        = { .type = NLA_U8 },*/
/*    [IFLA_LINKMODE]         = { .type = NLA_U8 },*/
};

struct nla_policy ifa_ipv4_policy[IFA_MAX+1] = {
	[IFA_LOCAL]             = { .type = NLA_U32 },
	[IFA_ADDRESS]           = { .type = NLA_U32 },
	[IFA_BROADCAST]         = { .type = NLA_U32 },
	[IFA_ANYCAST]           = { .type = NLA_U32 },
	[IFA_LABEL]             = { .type = NLA_STRING, .maxlen = IFNAMSIZ - 1 },
};
