#ifndef _NUT_NETLINK_OTHERS_H
#define _NUT_NETLINK_OTHERS_H

#pragma once

#include "netlink.h"
#include "netlink_cache.h"

namespace netlink {
	namespace internal {
		struct desc_flnl_request {
			using object_t = ::flnl_request;
			using managed_t = flnl_request_ref;
			static constexpr char const* name = "fib_lookup/request";
			static constexpr char const* cache_name = "fib_lookup/request";
		};
		struct desc_flnl_result {
			using object_t = ::flnl_result;
			using managed_t = flnl_result_ref;
			static constexpr char const* name = "fib_lookup/result";
			static constexpr char const* cache_name = "fib_lookup/result";
		};
		struct desc_genl_family {
			using object_t = ::genl_family;
			using managed_t = genl_family_ref;
			static constexpr char const* name = "genl/family";
			static constexpr char const* cache_name = "genl/family";
		};
		struct desc_idiagnl_meminfo {
			using object_t = ::idiagnl_meminfo;
			using managed_t = idiagnl_meminfo_ref;
			static constexpr char const* name = "idiag/idiag_meminfo";
		};
		struct desc_idiagnl_vegasinfo {
			using object_t = ::idiagnl_vegasinfo;
			using managed_t = idiagnl_vegasinfo_ref;
			static constexpr char const* name = "idiag/idiag_vegasinfo";
		};
		struct desc_idiagnl_msg {
			using object_t = ::idiagnl_msg;
			using managed_t = idiagnl_msg_ref;
			static constexpr char const* name = "idiag/idiag_msg";
		};
		struct desc_idiagnl_req {
			using object_t = ::idiagnl_req;
			using managed_t = idiagnl_req_ref;
			static constexpr char const* name = "idiag/idiag_req";
		};
		struct desc_nfnl_ct {
			using object_t = ::nfnl_ct;
			using managed_t = nfnl_ct_ref;
			static constexpr char const* name = "netfilter/ct";
			static constexpr char const* cache_name = "netfilter/ct";
		};
		struct desc_nfnl_exp {
			using object_t = ::nfnl_exp;
			using managed_t = nfnl_exp_ref;
			static constexpr char const* name = "netfilter/exp";
			static constexpr char const* cache_name = "netfilter/exp";
		};
		struct desc_nfnl_log {
			using object_t = ::nfnl_log;
			using managed_t = nfnl_log_ref;
			static constexpr char const* name = "netfilter/log";
			static constexpr char const* cache_name = "netfilter/log";
		};
		struct desc_nfnl_log_msg {
			using object_t = ::nfnl_log_msg;
			using managed_t = nfnl_log_msg_ref;
			static constexpr char const* name = "netfilter/log_msg";
			static constexpr char const* cache_name = "netfilter/log_msg";
		};
		struct desc_nfnl_queue {
			using object_t = ::nfnl_queue;
			using managed_t = nfnl_queue_ref;
			static constexpr char const* name = "netfilter/queue";
			static constexpr char const* cache_name = "netfilter/queue";
		};
		struct desc_nfnl_queue_msg {
			using object_t = ::nfnl_queue_msg;
			using managed_t = nfnl_queue_msg_ref;
			static constexpr char const* name = "netfilter/queuemsg";
			static constexpr char const* cache_name = "netfilter/queue_msg";
		};
		struct desc_rtnl_act {
			using object_t = ::rtnl_act;
			using managed_t = rtnl_act_ref;
			static constexpr char const* name = "route/act";
			static constexpr char const* cache_name = "route/act";
		};
		struct desc_rtnl_class {
			using object_t = ::rtnl_class;
			using managed_t = rtnl_class_ref;
			static constexpr char const* name = "route/class";
			static constexpr char const* cache_name = "route/class";
		};
		struct desc_rtnl_cls {
			using object_t = ::rtnl_cls;
			using managed_t = rtnl_cls_ref;
			static constexpr char const* name = "route/cls";
			static constexpr char const* cache_name = "route/cls";
		};
		struct desc_rtnl_neigh {
			using object_t = ::rtnl_neigh;
			using managed_t = rtnl_neigh_ref;
			static constexpr char const* name = "route/neigh";
			static constexpr char const* cache_name = "route/neigh";
		};
		struct desc_rtnl_neightbl {
			using object_t = ::rtnl_neightbl;
			using managed_t = rtnl_neightbl_ref;
			static constexpr char const* name = "route/neightbl";
			static constexpr char const* cache_name = "route/neightbl";
		};
		struct desc_rtnl_qdisc {
			using object_t = ::rtnl_qdisc;
			using managed_t = rtnl_qdisc_ref;
			static constexpr char const* name = "route/qdisc";
			static constexpr char const* cache_name = "route/qdisc";
		};
		struct desc_rtnl_rule {
			using object_t = ::rtnl_rule;
			using managed_t = rtnl_rule_ref;
			static constexpr char const* name = "route/rule";
			static constexpr char const* cache_name = "route/rule";
		};
		struct desc_xfrmnl_ae {
			using object_t = ::xfrmnl_ae;
			using managed_t = xfrmnl_ae_ref;
			static constexpr char const* name = "xfrm/ae";
		};
		struct desc_xfrmnl_sa {
			using object_t = ::xfrmnl_sa;
			using managed_t = xfrmnl_sa_ref;
			static constexpr char const* name = "xfrm/sa";
			static constexpr char const* cache_name = "xfrm/sa";
		};
		struct desc_xfrmnl_sp {
			using object_t = ::xfrmnl_sp;
			using managed_t = xfrmnl_sp_ref;
			static constexpr char const* name = "xfrm/sp";
			static constexpr char const* cache_name = "xfrm/sp";
		};

		extern template class object_ref<desc_flnl_request>;
		extern template class object_ref<desc_flnl_result>;
		extern template class object_ref<desc_genl_family>;
		extern template class object_ref<desc_idiagnl_meminfo>;
		extern template class object_ref<desc_idiagnl_vegasinfo>;
		extern template class object_ref<desc_idiagnl_msg>;
		extern template class object_ref<desc_idiagnl_req>;
		extern template class object_ref<desc_nfnl_ct>;
		extern template class object_ref<desc_nfnl_exp>;
		extern template class object_ref<desc_nfnl_log>;
		extern template class object_ref<desc_nfnl_log_msg>;
		extern template class object_ref<desc_nfnl_queue>;
		extern template class object_ref<desc_nfnl_queue_msg>;
		extern template class object_ref<desc_rtnl_act>;
		extern template class object_ref<desc_rtnl_class>;
		extern template class object_ref<desc_rtnl_cls>;
		extern template class object_ref<desc_rtnl_neigh>;
		extern template class object_ref<desc_rtnl_neightbl>;
		extern template class object_ref<desc_rtnl_qdisc>;
		extern template class object_ref<desc_rtnl_rule>;
		extern template class object_ref<desc_xfrmnl_ae>;
		extern template class object_ref<desc_xfrmnl_sa>;
		extern template class object_ref<desc_xfrmnl_sp>;
	}

	extern template class nl_cache_iterator<internal::desc_flnl_request>;
	extern template class nl_cache_iterator<internal::desc_flnl_result>;
	extern template class nl_cache_iterator<internal::desc_genl_family>;
	// extern template class nl_cache_iterator<internal::desc_idiagnl_meminfo>;
	// extern template class nl_cache_iterator<internal::desc_idiagnl_vegasinfo>;
	// extern template class nl_cache_iterator<internal::desc_idiagnl_msg>;
	// extern template class nl_cache_iterator<internal::desc_idiagnl_req>;
	extern template class nl_cache_iterator<internal::desc_nfnl_ct>;
	extern template class nl_cache_iterator<internal::desc_nfnl_exp>;
	extern template class nl_cache_iterator<internal::desc_nfnl_log>;
	extern template class nl_cache_iterator<internal::desc_nfnl_log_msg>;
	extern template class nl_cache_iterator<internal::desc_nfnl_queue>;
	extern template class nl_cache_iterator<internal::desc_nfnl_queue_msg>;
	extern template class nl_cache_iterator<internal::desc_rtnl_act>;
	extern template class nl_cache_iterator<internal::desc_rtnl_class>;
	extern template class nl_cache_iterator<internal::desc_rtnl_cls>;
	extern template class nl_cache_iterator<internal::desc_rtnl_neigh>;
	extern template class nl_cache_iterator<internal::desc_rtnl_neightbl>;
	extern template class nl_cache_iterator<internal::desc_rtnl_qdisc>;
	extern template class nl_cache_iterator<internal::desc_rtnl_rule>;
	// extern template class nl_cache_iterator<internal::desc_xfrmnl_ae>;
	extern template class nl_cache_iterator<internal::desc_xfrmnl_sa>;
	extern template class nl_cache_iterator<internal::desc_xfrmnl_sp>;

	extern template class nl_cache_ref<internal::desc_flnl_request>;
	extern template class nl_cache_ref<internal::desc_flnl_result>;
	extern template class nl_cache_ref<internal::desc_genl_family>;
	// extern template class nl_cache_ref<internal::desc_idiagnl_meminfo>;
	// extern template class nl_cache_ref<internal::desc_idiagnl_vegasinfo>;
	// extern template class nl_cache_ref<internal::desc_idiagnl_msg>;
	// extern template class nl_cache_ref<internal::desc_idiagnl_req>;
	extern template class nl_cache_ref<internal::desc_nfnl_ct>;
	extern template class nl_cache_ref<internal::desc_nfnl_exp>;
	extern template class nl_cache_ref<internal::desc_nfnl_log>;
	extern template class nl_cache_ref<internal::desc_nfnl_log_msg>;
	extern template class nl_cache_ref<internal::desc_nfnl_queue>;
	extern template class nl_cache_ref<internal::desc_nfnl_queue_msg>;
	extern template class nl_cache_ref<internal::desc_rtnl_act>;
	extern template class nl_cache_ref<internal::desc_rtnl_class>;
	extern template class nl_cache_ref<internal::desc_rtnl_cls>;
	extern template class nl_cache_ref<internal::desc_rtnl_neigh>;
	extern template class nl_cache_ref<internal::desc_rtnl_neightbl>;
	extern template class nl_cache_ref<internal::desc_rtnl_qdisc>;
	extern template class nl_cache_ref<internal::desc_rtnl_rule>;
	// extern template class nl_cache_ref<internal::desc_xfrmnl_ae>;
	extern template class nl_cache_ref<internal::desc_xfrmnl_sa>;
	extern template class nl_cache_ref<internal::desc_xfrmnl_sp>;

	class flnl_request_ref : public internal::object_ref<internal::desc_flnl_request> {
	};
	class flnl_result_ref : public internal::object_ref<internal::desc_flnl_result> {
	};
	class genl_family_ref : public internal::object_ref<internal::desc_genl_family> {
	};
	class idiagnl_meminfo_ref : public internal::object_ref<internal::desc_idiagnl_meminfo> {
	};
	class idiagnl_vegasinfo_ref : public internal::object_ref<internal::desc_idiagnl_vegasinfo> {
	};
	class idiagnl_msg_ref : public internal::object_ref<internal::desc_idiagnl_msg> {
	};
	class idiagnl_req_ref : public internal::object_ref<internal::desc_idiagnl_req> {
	};
	class nfnl_ct_ref : public internal::object_ref<internal::desc_nfnl_ct> {
	};
	class nfnl_exp_ref : public internal::object_ref<internal::desc_nfnl_exp> {
	};
	class nfnl_log_ref : public internal::object_ref<internal::desc_nfnl_log> {
	};
	class nfnl_log_msg_ref : public internal::object_ref<internal::desc_nfnl_log_msg> {
	};
	class nfnl_queue_ref : public internal::object_ref<internal::desc_nfnl_queue> {
	};
	class nfnl_queue_msg_ref : public internal::object_ref<internal::desc_nfnl_queue_msg> {
	};
	class rtnl_act_ref : public internal::object_ref<internal::desc_rtnl_act> {
	};
	class rtnl_class_ref : public internal::object_ref<internal::desc_rtnl_class> {
	};
	class rtnl_cls_ref : public internal::object_ref<internal::desc_rtnl_cls> {
	};
	class rtnl_neigh_ref : public internal::object_ref<internal::desc_rtnl_neigh> {
	};
	class rtnl_neightbl_ref : public internal::object_ref<internal::desc_rtnl_neightbl> {
	};
	class rtnl_qdisc_ref : public internal::object_ref<internal::desc_rtnl_qdisc> {
	};
	class rtnl_rule_ref : public internal::object_ref<internal::desc_rtnl_rule> {
	};
	class xfrmnl_ae_ref : public internal::object_ref<internal::desc_xfrmnl_ae> {
	};
	class xfrmnl_sa_ref : public internal::object_ref<internal::desc_xfrmnl_sa> {
	};
	class xfrmnl_sp_ref : public internal::object_ref<internal::desc_xfrmnl_sp> {
	};

	using flnl_request_cache_ref = nl_cache_ref<internal::desc_flnl_request>;
	using flnl_result_cache_ref = nl_cache_ref<internal::desc_flnl_result>;
	using genl_family_cache_ref = nl_cache_ref<internal::desc_genl_family>;
	// using idiagnl_meminfo_cache_ref = nl_cache_ref<internal::desc_idiagnl_meminfo>;
	// using idiagnl_vegasinfo_cache_ref = nl_cache_ref<internal::desc_idiagnl_vegasinfo>;
	// using idiagnl_msg_cache_ref = nl_cache_ref<internal::desc_idiagnl_msg>;
	// using idiagnl_req_cache_ref = nl_cache_ref<internal::desc_idiagnl_req>;
	using nfnl_ct_cache_ref = nl_cache_ref<internal::desc_nfnl_ct>;
	using nfnl_exp_cache_ref = nl_cache_ref<internal::desc_nfnl_exp>;
	using nfnl_log_cache_ref = nl_cache_ref<internal::desc_nfnl_log>;
	using nfnl_log_msg_cache_ref = nl_cache_ref<internal::desc_nfnl_log_msg>;
	using nfnl_queue_cache_ref = nl_cache_ref<internal::desc_nfnl_queue>;
	using nfnl_queue_msg_cache_ref = nl_cache_ref<internal::desc_nfnl_queue_msg>;
	using rtnl_act_cache_ref = nl_cache_ref<internal::desc_rtnl_act>;
	using rtnl_class_cache_ref = nl_cache_ref<internal::desc_rtnl_class>;
	using rtnl_cls_cache_ref = nl_cache_ref<internal::desc_rtnl_cls>;
	using rtnl_neigh_cache_ref = nl_cache_ref<internal::desc_rtnl_neigh>;
	using rtnl_neightbl_cache_ref = nl_cache_ref<internal::desc_rtnl_neightbl>;
	using rtnl_qdisc_cache_ref = nl_cache_ref<internal::desc_rtnl_qdisc>;
	using rtnl_rule_cache_ref = nl_cache_ref<internal::desc_rtnl_rule>;
	// using xfrmnl_ae_cache_ref = nl_cache_ref<internal::desc_xfrmnl_ae>;
	using xfrmnl_sa_cache_ref = nl_cache_ref<internal::desc_xfrmnl_sa>;
	using xfrmnl_sp_cache_ref = nl_cache_ref<internal::desc_xfrmnl_sp>;
}

#endif /* _NUT_NETLINK_OTHERS_H */
