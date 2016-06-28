#include "netlink_cache.h"
#include "netlink_cache_impl.h"

#include "netlink_generic.h"
#include "netlink_rtnl_addr.h"
#include "netlink_rtnl_link.h"
#include "netlink_rtnl_route.h"

namespace netlink {
	int nl_cache_untyped_ref::nitems() const noexcept {
		return ::nl_cache_nitems(get());
	}
	void nl_cache_untyped_ref::clear() const noexcept {
		::nl_cache_clear(get());
	}

	bool nl_cache_untyped_ref::is_empty() const noexcept {
		return ::nl_cache_is_empty(get());
	}

	void nl_cache_untyped_ref::set_arg1(int arg) const noexcept {
		::nl_cache_set_arg1(get(), arg);
	}
	void nl_cache_untyped_ref::set_arg2(int arg) const noexcept {
		::nl_cache_set_arg2(get(), arg);
	}
	void nl_cache_untyped_ref::set_flags(netlink::nl_cache_flags flags) const noexcept {
		::nl_cache_set_flags(get(), (unsigned int)(flags));
	}

	std::error_code nl_cache_untyped_ref::refill(nl_socket_ptr const& sock) const noexcept {
		int err = ::nl_cache_refill(sock.get(), get());
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	template class nl_cache_iterator<internal::desc_rtnl_addr>;
	template class nl_cache_iterator<internal::desc_rtnl_link>;
	template class nl_cache_iterator<internal::desc_rtnl_route>;

	template class nl_cache_ref<internal::desc_rtnl_addr>;
	template class nl_cache_ref<internal::desc_rtnl_link>;
	template class nl_cache_ref<internal::desc_rtnl_route>;
}
