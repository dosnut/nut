#ifndef _NUT_NETLINK_SOCK_H
#define _NUT_NETLINK_SOCK_H

#pragma once

#include "netlink.h"
extern "C" {
#include <linux/netlink.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/rtnetlink.h>
#include <linux/xfrm.h>
}

#include <QString>
#include <QFlags>

#include <memory>

namespace netlink {
	enum class new_flag : int {
		replace = NLM_F_REPLACE, /* Override existing */
		excl = NLM_F_EXCL,       /* Do not touch, if it exists */
		create = NLM_F_CREATE,   /* Create, if it does not exist */
		append = NLM_F_APPEND,   /* Add to end of list */
	};
	using new_flags = QFlags<new_flag>;
	Q_DECLARE_OPERATORS_FOR_FLAGS(new_flags)

	enum class protocol : int {
		route = NETLINK_ROUTE, /* Routing/device hook */
		unused = NETLINK_UNUSED, /* Unused number */
		usersock = NETLINK_USERSOCK, /* Reserved for user mode socket protocols */
		firewall = NETLINK_FIREWALL, /* Unused number, formerly ip_queue */
		sock_diag = NETLINK_SOCK_DIAG, /* socket monitoring */
		nflog = NETLINK_NFLOG, /* netfilter/iptables ULOG */
		xfrm = NETLINK_XFRM, /* ipsec */
		selinux = NETLINK_SELINUX, /* SELinux event notifications */
		iscsi = NETLINK_ISCSI, /* Open-iSCSI */
		audit = NETLINK_AUDIT, /* auditing */
		fib_lookup = NETLINK_FIB_LOOKUP,
		connector = NETLINK_CONNECTOR,
		netfilter = NETLINK_NETFILTER, /* netfilter subsystem */
		ip6_fw = NETLINK_IP6_FW,
		dnrtmsg = NETLINK_DNRTMSG, /* DECnet routing messages */
		kobject_uevent = NETLINK_KOBJECT_UEVENT, /* Kernel messages to userspace */
		generic = NETLINK_GENERIC,
		scsitransport = NETLINK_SCSITRANSPORT, /* SCSI Transports */
		ecryptfs = NETLINK_ECRYPTFS,
		rdma = NETLINK_RDMA,
		crypto = NETLINK_CRYPTO, /* Crypto layer */
		inet_diag = NETLINK_INET_DIAG,
	};

	enum class nfnetlink_group : int {
		none = NFNLGRP_NONE,
		conntrack_new = NFNLGRP_CONNTRACK_NEW,
		conntrack_update = NFNLGRP_CONNTRACK_UPDATE,
		conntrack_destroy = NFNLGRP_CONNTRACK_DESTROY,
		conntrack_exp_new = NFNLGRP_CONNTRACK_EXP_NEW,
		conntrack_exp_update = NFNLGRP_CONNTRACK_EXP_UPDATE,
		conntrack_exp_destroy = NFNLGRP_CONNTRACK_EXP_DESTROY,
	};

	enum class rtnetlink_group : int {
		none = RTNLGRP_NONE,
		link = RTNLGRP_LINK,
		notify = RTNLGRP_NOTIFY,
		neigh = RTNLGRP_NEIGH,
		tc = RTNLGRP_TC,
		ipv4_ifaddr = RTNLGRP_IPV4_IFADDR,
		ipv4_mroute = RTNLGRP_IPV4_MROUTE,
		ipv4_route = RTNLGRP_IPV4_ROUTE,
		ipv4_rule = RTNLGRP_IPV4_RULE,
		ipv6_ifaddr = RTNLGRP_IPV6_IFADDR,
		ipv6_mroute = RTNLGRP_IPV6_MROUTE,
		ipv6_route = RTNLGRP_IPV6_ROUTE,
		ipv6_ifinfo = RTNLGRP_IPV6_IFINFO,
		decnet_ifaddr = RTNLGRP_DECnet_IFADDR,
		nop2 = RTNLGRP_NOP2,
		decnet_route = RTNLGRP_DECnet_ROUTE,
		decnet_rule = RTNLGRP_DECnet_RULE,
		nop4 = RTNLGRP_NOP4,
		ipv6_prefix = RTNLGRP_IPV6_PREFIX,
		ipv6_rule = RTNLGRP_IPV6_RULE,
		nd_useropt = RTNLGRP_ND_USEROPT,
		phonet_ifaddr = RTNLGRP_PHONET_IFADDR,
		phonet_route = RTNLGRP_PHONET_ROUTE,
	};

	enum class xfrm_nlgroups : int {
		none = XFRMNLGRP_NONE,
		acquire = XFRMNLGRP_ACQUIRE,
		expire = XFRMNLGRP_EXPIRE,
		sa = XFRMNLGRP_SA,
		policy = XFRMNLGRP_POLICY,
		aevents = XFRMNLGRP_AEVENTS,
		report = XFRMNLGRP_REPORT,
		migrate = XFRMNLGRP_MIGRATE,
		mapping = XFRMNLGRP_MAPPING,
	};

	enum class callback_kind : int {
		default_ = NL_CB_DEFAULT,
		verbose = NL_CB_VERBOSE,
		debug = NL_CB_DEBUG,
	};

	enum class callback_type : int {
		valid = NL_CB_VALID,
		finish = NL_CB_FINISH,
		overrun = NL_CB_OVERRUN,
		skipped = NL_CB_SKIPPED,
		ack = NL_CB_ACK,
		msg_in = NL_CB_MSG_IN,
		msg_out = NL_CB_MSG_OUT,
		invalid = NL_CB_INVALID,
		seq_check = NL_CB_SEQ_CHECK,
		send_ack = NL_CB_SEND_ACK,
		dump_intr = NL_CB_DUMP_INTR,
	};

	class nl_callback_ref : protected internal::generic_ref<internal::desc_nl_cb> {
	private:
		struct func_storage;
		using parent = internal::generic_ref<internal::desc_nl_cb>;

	public:
		using func_t = std::function<int(::nl_msg* msg)>;
		using err_func_t = std::function<int(::sockaddr_nl* nla, ::nlmsgerr* nlerr)>;

		explicit constexpr nl_callback_ref() noexcept = default;

		static nl_callback_ref alloc(callback_kind kind = callback_kind::default_) noexcept;
		static nl_callback_ref take_own(::nl_cb* cb) noexcept;
		static nl_callback_ref take_inc_ref(::nl_cb* cb) noexcept;

		using parent::get;
		using parent::operator bool;

		nl_callback_ref clone() const noexcept;

		std::error_code set(callback_type type, callback_kind kind) const noexcept;
		std::error_code set(callback_type type, ::nl_recvmsg_msg_cb_t cb, void* arg) const noexcept;
		std::error_code set(callback_type type, func_t const& cb) const noexcept;
		std::error_code err(callback_kind kind) const noexcept;
		std::error_code err(::nl_recvmsg_err_cb_t cb, void* arg) const noexcept;
		std::error_code err(err_func_t const& cb) const noexcept;

	private:
		std::shared_ptr<func_storage> m_storage;
	};

	class nl_socket_ptr {
	private:
		struct deleter_t {
			void operator()(::nl_sock* sock) const noexcept;
		};

	public:
		explicit constexpr nl_socket_ptr() noexcept = default;
		explicit nl_socket_ptr(::nl_sock* sock) noexcept;

		static nl_socket_ptr alloc() noexcept;

		void reset() noexcept;

		nl_sock* get() const noexcept;
		nl_sock* release() noexcept;
		explicit operator bool() const noexcept;

		std::error_code connect(protocol prot) const noexcept;
		void close() const noexcept;

		//! connect protocol::netfilter and add memberships
		std::error_code connect_groups(std::initializer_list<nfnetlink_group> groups) const noexcept;
		//! connect protocol::route and add memberships
		std::error_code connect_groups(std::initializer_list<rtnetlink_group> groups) const noexcept;
		//! connect protocol::xfrm and add memberships
		std::error_code connect_groups(std::initializer_list<xfrm_nlgroups> groups) const noexcept;

		void set_local_port(uint32_t local_port) const noexcept;
		uint32_t get_local_port() const noexcept;

		std::error_code add_membership(nfnetlink_group group) const noexcept;
		std::error_code drop_membership(nfnetlink_group group) const noexcept;

		std::error_code add_membership(rtnetlink_group group) const noexcept;
		std::error_code drop_membership(rtnetlink_group group) const noexcept;

		std::error_code add_membership(xfrm_nlgroups group) const noexcept;
		std::error_code drop_membership(xfrm_nlgroups group) const noexcept;

		std::error_code add_membership_raw(int group) const noexcept;
		std::error_code drop_membership_raw(int group) const noexcept;

		genl_family_ref family_by_name(const char* name, std::error_code& ec) const noexcept;
		genl_family_ref family_by_name(const char* name) const;

		std::error_code add_memberships(genl_family_ref const& family, std::initializer_list<char const*> groups) const noexcept;
		std::error_code drop_memberships(genl_family_ref const& family, std::initializer_list<char const*> groups) const noexcept;

		void set_peer_port(uint32_t peer_port) const noexcept;
		uint32_t get_peer_port() const noexcept;

		void set_peer_groups(uint32_t peer_groups) const noexcept;
		uint32_t get_peer_groups() const noexcept;

		void set_cb(nl_callback_ref const& cb) noexcept;
		nl_callback_ref const& get_cb() const noexcept;

		std::error_code set_buffer_size(int rxbuf, int txbuf) const noexcept;

		std::error_code set_msg_buf_size(size_t msg_buf_size) const noexcept;
		size_t get_msg_buf_size() const noexcept;

		std::error_code set_passcred(int passcred) const noexcept;

		std::error_code recv_pktinfo(int state) const noexcept;

		void disable_seq_check() const noexcept;
		unsigned int use_seq() const noexcept;
		void disable_auto_ack() const noexcept;
		void enable_auto_ack() const noexcept;

		std::error_code set_fd(int protocol, int fd) const noexcept;
		int get_fd() const noexcept;

		std::error_code set_nonblocking() const noexcept;

		void enable_msg_peek() const noexcept;
		void disable_msg_peek() const noexcept;

		std::error_code recvmsgs(nl_callback_ref const& cb) const noexcept;
		std::error_code recvmsgs() const noexcept;

		std::error_code send_simple(int type, int flags, void* buf, size_t size) const noexcept;
		std::error_code send_auto(nl_msg* msg) const noexcept;

		std::error_code wait_for_ack() const noexcept;

	private:
		std::unique_ptr<::nl_sock, deleter_t> m_sock;
		nl_callback_ref m_callback;
		mutable bool m_auto_ack{false};
	};
}

#endif /* _NUT_NETLINK_SOCK_H */
