#include "netlink_others.h"

#include "netlink_templates_impl.h"
#include "netlink_cache_impl.h"

namespace netlink {
	namespace internal {
		template class object_ref<desc_flnl_request>;
		template class object_ref<desc_flnl_result>;
		template class object_ref<desc_genl_family>;
		template class object_ref<desc_idiagnl_meminfo>;
		template class object_ref<desc_idiagnl_vegasinfo>;
		template class object_ref<desc_idiagnl_msg>;
		template class object_ref<desc_idiagnl_req>;
		template class object_ref<desc_nfnl_ct>;
		template class object_ref<desc_nfnl_exp>;
		template class object_ref<desc_nfnl_log>;
		template class object_ref<desc_nfnl_log_msg>;
		template class object_ref<desc_nfnl_queue>;
		template class object_ref<desc_nfnl_queue_msg>;
		template class object_ref<desc_rtnl_act>;
		template class object_ref<desc_rtnl_class>;
		template class object_ref<desc_rtnl_cls>;
		template class object_ref<desc_rtnl_neigh>;
		template class object_ref<desc_rtnl_neightbl>;
		template class object_ref<desc_rtnl_qdisc>;
		template class object_ref<desc_rtnl_rule>;
		template class object_ref<desc_xfrmnl_ae>;
		template class object_ref<desc_xfrmnl_sa>;
		template class object_ref<desc_xfrmnl_sp>;
	}

	template class nl_cache_iterator<internal::desc_flnl_request>;
	template class nl_cache_iterator<internal::desc_flnl_result>;
	template class nl_cache_iterator<internal::desc_genl_family>;
	// template class nl_cache_iterator<internal::desc_idiagnl_meminfo>;
	// template class nl_cache_iterator<internal::desc_idiagnl_vegasinfo>;
	// template class nl_cache_iterator<internal::desc_idiagnl_msg>;
	// template class nl_cache_iterator<internal::desc_idiagnl_req>;
	template class nl_cache_iterator<internal::desc_nfnl_ct>;
	template class nl_cache_iterator<internal::desc_nfnl_exp>;
	template class nl_cache_iterator<internal::desc_nfnl_log>;
	template class nl_cache_iterator<internal::desc_nfnl_log_msg>;
	template class nl_cache_iterator<internal::desc_nfnl_queue>;
	template class nl_cache_iterator<internal::desc_nfnl_queue_msg>;
	template class nl_cache_iterator<internal::desc_rtnl_act>;
	template class nl_cache_iterator<internal::desc_rtnl_class>;
	template class nl_cache_iterator<internal::desc_rtnl_cls>;
	template class nl_cache_iterator<internal::desc_rtnl_neigh>;
	template class nl_cache_iterator<internal::desc_rtnl_neightbl>;
	template class nl_cache_iterator<internal::desc_rtnl_qdisc>;
	template class nl_cache_iterator<internal::desc_rtnl_rule>;
	// template class nl_cache_iterator<internal::desc_xfrmnl_ae>;
	template class nl_cache_iterator<internal::desc_xfrmnl_sa>;
	template class nl_cache_iterator<internal::desc_xfrmnl_sp>;

	template class nl_cache_ref<internal::desc_flnl_request>;
	template class nl_cache_ref<internal::desc_flnl_result>;
	template class nl_cache_ref<internal::desc_genl_family>;
	// template class nl_cache_ref<internal::desc_idiagnl_meminfo>;
	// template class nl_cache_ref<internal::desc_idiagnl_vegasinfo>;
	// template class nl_cache_ref<internal::desc_idiagnl_msg>;
	// template class nl_cache_ref<internal::desc_idiagnl_req>;
	template class nl_cache_ref<internal::desc_nfnl_ct>;
	template class nl_cache_ref<internal::desc_nfnl_exp>;
	template class nl_cache_ref<internal::desc_nfnl_log>;
	template class nl_cache_ref<internal::desc_nfnl_log_msg>;
	template class nl_cache_ref<internal::desc_nfnl_queue>;
	template class nl_cache_ref<internal::desc_nfnl_queue_msg>;
	template class nl_cache_ref<internal::desc_rtnl_act>;
	template class nl_cache_ref<internal::desc_rtnl_class>;
	template class nl_cache_ref<internal::desc_rtnl_cls>;
	template class nl_cache_ref<internal::desc_rtnl_neigh>;
	template class nl_cache_ref<internal::desc_rtnl_neightbl>;
	template class nl_cache_ref<internal::desc_rtnl_qdisc>;
	template class nl_cache_ref<internal::desc_rtnl_rule>;
	// template class nl_cache_ref<internal::desc_xfrmnl_ae>;
	template class nl_cache_ref<internal::desc_xfrmnl_sa>;
	template class nl_cache_ref<internal::desc_xfrmnl_sp>;
}
