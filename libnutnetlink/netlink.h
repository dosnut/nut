#ifndef _NUT_NETLINK_H
#define _NUT_NETLINK_H

#pragma once

#include <QByteArray>

#include <system_error>

#include "netlink_templates.h"

extern "C" {
	#include <netlink/netlink.h>

	/* basic netlink types */
	struct nl_addr;
	struct nl_cache;
	struct nl_cb;
	struct nl_data;
	struct nl_msg;
	struct nl_sock;

	/* base object type */
	struct nl_object;

	/* fib lookup */
	struct flnl_request;
	struct flnl_result;

	/* generic netlink objects: */
	struct genl_family;

	/* idiag objects: */
	struct idiagnl_meminfo;
	struct idiagnl_vegasinfo;
	struct idiagnl_msg;
	struct idiagnl_req;

	/* netfilter objects: */
	struct nfnl_ct;
	struct nfnl_exp;
	struct nfnl_log;
	struct nfnl_log_msg;
	struct nfnl_queue;
	struct nfnl_queue_msg;

	/* route objects: */
	struct rtnl_act;
	struct rtnl_addr;
	struct rtnl_class;
	struct rtnl_cls;
	struct rtnl_link;
	struct rtnl_neigh;
	struct rtnl_neightbl;
	struct rtnl_qdisc;
	struct rtnl_route;
	struct rtnl_rule;
	struct rtnl_tc;

	/* route other types: */
	struct rtnl_nexthop;

	/* xfrm objects */
	struct xfrmnl_ae;
	struct xfrmnl_sa;
	struct xfrmnl_sp;
}

namespace netlink {
	enum class errc : int {
		success = NLE_SUCCESS,
		failure = NLE_FAILURE,
		intr = NLE_INTR,
		bad_sock = NLE_BAD_SOCK,
		again = NLE_AGAIN,
		nomem = NLE_NOMEM,
		exist = NLE_EXIST,
		inval = NLE_INVAL,
		range = NLE_RANGE,
		msgsize = NLE_MSGSIZE,
		opnotsupp = NLE_OPNOTSUPP,
		af_nosupport = NLE_AF_NOSUPPORT,
		obj_notfound = NLE_OBJ_NOTFOUND,
		noattr = NLE_NOATTR,
		missing_attr = NLE_MISSING_ATTR,
		af_mismatch = NLE_AF_MISMATCH,
		seq_mismatch = NLE_SEQ_MISMATCH,
		msg_overflow = NLE_MSG_OVERFLOW,
		msg_trunc = NLE_MSG_TRUNC,
		noaddr = NLE_NOADDR,
		srcrt_nosupport = NLE_SRCRT_NOSUPPORT,
		msg_tooshort = NLE_MSG_TOOSHORT,
		msgtype_nosupport = NLE_MSGTYPE_NOSUPPORT,
		obj_mismatch = NLE_OBJ_MISMATCH,
		nocache = NLE_NOCACHE,
		busy = NLE_BUSY,
		proto_mismatch = NLE_PROTO_MISMATCH,
		noaccess = NLE_NOACCESS,
		perm = NLE_PERM,
		pktloc_file = NLE_PKTLOC_FILE,
		parse_err = NLE_PARSE_ERR,
		nodev = NLE_NODEV,
		immutable = NLE_IMMUTABLE,
		dump_intr = NLE_DUMP_INTR,
	};
}

namespace std {
	template<>
	struct is_error_code_enum<netlink::errc> : true_type {};
}

namespace netlink {
	std::error_category& get_error_category() noexcept;
	std::error_code make_error_code(errc e) noexcept;
	std::error_code make_netlink_error_code(int code) noexcept;

	using uint = unsigned int;

	class nl_addr_ref;
	class nl_callback_ref;
	class nl_cache_untyped_ref;
	class nl_msg_ref;

	class flnl_request_ref;
	class flnl_result_ref;
	class genl_family_ref;
	class idiagnl_meminfo_ref;
	class idiagnl_vegasinfo_ref;
	class idiagnl_msg_ref;
	class idiagnl_req_ref;
	class nfnl_ct_ref;
	class nfnl_exp_ref;
	class nfnl_log_ref;
	class nfnl_log_msg_ref;
	class nfnl_queue_ref;
	class nfnl_queue_msg_ref;
	class rtnl_act_ref;
	class rtnl_addr_ref;
	class rtnl_class_ref;
	class rtnl_cls_ref;
	class rtnl_link_ref;
	class rtnl_neigh_ref;
	class rtnl_neightbl_ref;
	class rtnl_qdisc_ref;
	class rtnl_route_ref;
	class rtnl_rule_ref;
	class xfrmnl_ae_ref;
	class xfrmnl_sa_ref;
	class xfrmnl_sp_ref;

	class nl_socket_ptr;

	namespace internal {
		struct desc_nl_addr {
			using type = ::nl_addr;
			using managed_t = nl_addr_ref;

			static void acquire(type* v) noexcept;
			static void release(type* v) noexcept;
			static type* clone(type* v) noexcept;
		};

		struct desc_nl_cache {
			using type = ::nl_cache;
			using managed_t = nl_cache_untyped_ref;

			static void acquire(type* v) noexcept;
			static void release(type* v) noexcept;
			static type* clone(type* v) noexcept;
		};

		struct desc_nl_cb {
			using type = ::nl_cb;
			using managed_t = nl_callback_ref;

			static void acquire(type* v) noexcept;
			static void release(type* v) noexcept;
			static type* clone(type* v) noexcept;
		};

		struct desc_nl_msg {
			using type = ::nl_msg;
			using managed_t = nl_msg_ref;

			static void acquire(type* v) noexcept;
			static void release(type* v) noexcept;
		};

		extern template class generic_ref_no_clone<desc_nl_addr>;
		extern template class generic_ref_no_clone<desc_nl_cache>;
		extern template class generic_ref_no_clone<desc_nl_cb>;

		struct desc_rtnl_addr {
			using object_t = ::rtnl_addr;
			using managed_t = rtnl_addr_ref;
			static constexpr char const* name = "route/addr";
			static constexpr char const* cache_name = "route/addr";
		};
		struct desc_rtnl_link {
			using object_t = ::rtnl_link;
			using managed_t = rtnl_link_ref;
			static constexpr char const* name = "route/link";
			static constexpr char const* cache_name = "route/link";
		};
		struct desc_rtnl_route {
			using object_t = ::rtnl_route;
			using managed_t = rtnl_route_ref;
			static constexpr char const* name = "route/route";
			static constexpr char const* cache_name = "route/route";
		};

		extern template class object_ref<desc_rtnl_addr>;
		extern template class object_ref<desc_rtnl_link>;
		extern template class object_ref<desc_rtnl_route>;
	}

	QByteArray from_nl_data(nl_data const* data) noexcept;
}

#endif /* _NUT_NETLINK_H */
