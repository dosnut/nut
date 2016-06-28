#include "netlink.h"

#include "netlink_addr.h"
#include "netlink_cache.h"
#include "netlink_generic.h"
#include "netlink_msg.h"
#include "netlink_rtnl_addr.h"
#include "netlink_rtnl_link.h"
#include "netlink_rtnl_route.h"
#include "netlink_sock.h"

#include "netlink_templates_impl.h"

extern "C" {
#include <netlink/msg.h>
}

namespace netlink {
	namespace {
		class error_category final : public std::error_category {
			char const* name() const noexcept override {
				return "netlink error";
			}

			std::string message(int code) const noexcept override {
				return std::string{::nl_geterror(code)};
			}
		};
	}

	std::error_category& get_error_category() noexcept {
		static error_category cat;
		return cat;
	}

	std::error_code make_error_code(errc e) noexcept {
		return std::error_code(
			static_cast<int>(e),
			get_error_category());
	}

	std::error_code make_netlink_error_code(int code) noexcept {
		return std::error_code(
			code < 0 ? -code : code,
			get_error_category());
	}

	namespace internal {
		void desc_nl_addr::acquire(nl_addr* addr) noexcept {
			::nl_addr_get(addr);
		}

		void desc_nl_addr::release(nl_addr* addr) noexcept {
			::nl_addr_put(addr);
		}

		nl_addr* desc_nl_addr::clone(nl_addr* addr) noexcept {
			return ::nl_addr_clone(addr);
		}

		void desc_nl_cache::acquire(nl_cache* cache) noexcept {
			::nl_cache_get(cache);
		}

		void desc_nl_cache::release(nl_cache* cache) noexcept {
			::nl_cache_put(cache);
		}

		nl_cache* desc_nl_cache::clone(nl_cache* cache) noexcept {
			return ::nl_cache_clone(cache);
		}

		void desc_nl_cb::acquire(nl_cb* cb) noexcept {
			::nl_cb_get(cb);
		}

		void desc_nl_cb::release(nl_cb* cb) noexcept {
			::nl_cb_put(cb);
		}

		nl_cb* desc_nl_cb::clone(nl_cb* cb) noexcept {
			return ::nl_cb_clone(cb);
		}

		void desc_nl_msg::acquire(nl_msg* msg) noexcept {
			return ::nlmsg_get(msg);
		}

		void desc_nl_msg::release(nl_msg* msg) noexcept {
			return ::nlmsg_free(msg);
		}

		template class generic_ref_no_clone<desc_nl_addr>;
		template class generic_ref_no_clone<desc_nl_cache>;
		template class generic_ref_no_clone<desc_nl_cb>;
		template class generic_ref_no_clone<desc_nl_msg>;

		template class object_ref<desc_rtnl_addr>;
		template class object_ref<desc_rtnl_link>;
		template class object_ref<desc_rtnl_route>;
	}

	QByteArray from_nl_data(nl_data const* data) noexcept {
		return QByteArray(reinterpret_cast<char const*>(::nl_data_get(data)), ::nl_data_get_size(data));
	}
}
