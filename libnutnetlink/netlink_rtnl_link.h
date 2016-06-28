#ifndef _NUT_NETLINK_RTNL_LINK_H
#define _NUT_NETLINK_RTNL_LINK_H

#pragma once

#include "netlink_sock.h"

extern "C" {
#include <netlink/route/link.h>

/* link flags: IFF_UP, ... */
#include <linux/if.h>
}

#include <QString>

namespace netlink {
	enum class link_flag_t : int {
		up = IFF_UP,                   /* interface is up */
		broadcast = IFF_BROADCAST,     /* broadcast address valid */
		debug = IFF_DEBUG,             /* turn on debugging */
		loopback = IFF_LOOPBACK,       /* is a loopback net */
		pointopoint = IFF_POINTOPOINT, /* interface is has p-p link */
		notrailers = IFF_NOTRAILERS,   /* avoid use of trailers */
		running = IFF_RUNNING,         /* interface RFC2863 OPER_UP */
		noarp = IFF_NOARP,             /* no ARP protocol */
		promisc = IFF_PROMISC,         /* receive all packets */
		allmulti = IFF_ALLMULTI,       /* receive all multicast packets*/
		master = IFF_MASTER,           /* master of a load balancer */
		slave = IFF_SLAVE,             /* slave of a load balancer */
		multicast = IFF_MULTICAST,     /* Supports multicast */
		portsel = IFF_PORTSEL,         /* can set media type */
		automedia = IFF_AUTOMEDIA,     /* auto media select active */
		dynamic = IFF_DYNAMIC,         /* dialup device with changing addresses*/
		lower_up = IFF_LOWER_UP,       /* driver signals L1 up */
		dormant = IFF_DORMANT,         /* driver signals dormant */
		echo = IFF_ECHO,               /* echo sent packets */
		volatile_ = IFF_VOLATILE,      /* */
	};
	using link_flags_t = QFlags<link_flag_t>;
	Q_DECLARE_OPERATORS_FOR_FLAGS(link_flags_t)
	QString toString(link_flags_t link_flags);

	enum class operstate_t : uint8_t {
		unknown = IF_OPER_UNKNOWN,
		notpresent = IF_OPER_NOTPRESENT,
		down = IF_OPER_DOWN,
		lowerlayerdown = IF_OPER_LOWERLAYERDOWN,
		testing = IF_OPER_TESTING,
		dormant = IF_OPER_DORMANT,
		up = IF_OPER_UP,
	};
	QString toString(operstate_t operstate);

	enum class linkmode_t : uint8_t {
		default_ = IF_LINK_MODE_DEFAULT,
		dormant = IF_LINK_MODE_DORMANT, /* limit upward transition to dormant */
	};
	QString toString(linkmode_t linkmode);

	enum class carrier_t : uint8_t {
		down = 0,
		up = 1,
	};
	QString toString(carrier_t carrier);

	class rtnl_link_ref : public internal::object_ref<internal::desc_rtnl_link> {
	public:
		static rtnl_link_ref alloc() noexcept;

		std::error_code add(nl_socket_ptr const& sock, new_flags flags = new_flags{}) const noexcept;
		static std::error_code change(nl_socket_ptr const& sock, rtnl_link_ref const& orig, rtnl_link_ref const& changes, new_flags flags = new_flags{}) noexcept;
		std::error_code remove(nl_socket_ptr const& sock) const noexcept;
		static rtnl_link_ref get_link(nl_socket_ptr const& sock, int ifIndex, std::error_code& ec) noexcept;
		static rtnl_link_ref get_link(nl_socket_ptr const& sock, char const* name, std::error_code& ec) noexcept;

		void set_qdisc(QString const& qdisc) const;
		QString get_qdisc() const;

		void set_name(QString const& name) const;
		QString get_name() const;

		void set_group(uint32_t group) const noexcept;
		uint32_t get_group() const noexcept;

		void set_flags(link_flags_t flags) const noexcept;
		void unset_flags(link_flags_t flags) const noexcept;
		link_flags_t get_flags() const noexcept;

		void set_mtu(uint mtu) const noexcept;
		uint get_mtu() const noexcept;

		void set_txqlen(uint txqlen) const noexcept;
		uint get_txqlen() const noexcept;

		void set_ifindex(int ifIndex) const noexcept;
		int get_ifindex() const noexcept;

		void set_family(int family) const noexcept;
		int get_family() const noexcept;

		void set_arptype(uint arpType) const noexcept;
		uint get_arptype() const noexcept;

		void set_addr(nl_addr_ref addr) const noexcept;
		nl_addr_ref get_addr() const noexcept;

		void set_broadcast(nl_addr_ref addr) const noexcept;
		nl_addr_ref get_broadcast() const noexcept;

		void set_link(int link) const noexcept;
		int get_link() const noexcept;

		void set_master(int master) const noexcept;
		int get_master() const noexcept;

		void set_carrier(carrier_t carrier) const noexcept;
		carrier_t get_carrier() const noexcept;

		void set_operstate(operstate_t operstate) const noexcept;
		operstate_t get_operstate() const noexcept;

		void set_linkmode(linkmode_t linkmode_t) const noexcept;
		linkmode_t get_linkmode() const noexcept;

		void set_link_netnsid(int32_t link_netnsid) const noexcept;
		int32_t get_link_netnsid(std::error_code& ec) const noexcept;

		void set_ifalias(QString const& ifAlias) const noexcept;
		QString get_ifalias() const noexcept;

		uint32_t get_num_vf(std::error_code& ec) const noexcept;

		std::error_code set_stat(rtnl_link_stat_id_t, uint64_t) const noexcept;
		uint64_t get_stat(rtnl_link_stat_id_t) const noexcept;

		std::error_code set_type(const QString& type) const noexcept;
		QString get_type() const noexcept;

		void set_promiscuity(uint32_t promiscuity) const noexcept;
		uint32_t get_promiscuity() const noexcept;

		void set_num_tx_queues(uint32_t num_tx_queues) const noexcept;
		uint32_t get_num_tx_queues() const noexcept;

		void set_num_rx_queues(uint32_t num_rx_queues) const noexcept;
		uint32_t get_num_rx_queues() const noexcept;

		QByteArray get_phys_port_id() const;

		void set_ns_fd(int ns_fd) const noexcept;
		int get_ns_fd() const noexcept;

		void set_ns_pid(pid_t ns_pid) const noexcept;
		pid_t get_ns_pid() const noexcept;
	};
}

#endif /* _NUT_NETLINK_RTNL_LINK_H */
