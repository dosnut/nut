#include "netlink_rtnl_addr.h"

#include "netlink_addr.h"
#include "netlink_rtnl_link.h"

extern "C" {
#include <netlink/route/addr.h>
}

namespace netlink {
	QString toString(rtnl_addr_flags flags) {
		char buf[512];
		return QString::fromUtf8(rtnl_addr_flags2str(static_cast<int>(flags), buf, sizeof(buf)));
	}

	rtnl_addr_ref rtnl_addr_ref::alloc() {
		return rtnl_addr_ref::take_own(::rtnl_addr_alloc());
	}

	std::error_code rtnl_addr_ref::add(nl_socket_ptr const& sock, new_flags flags) const noexcept {
		int err = ::rtnl_addr_add(sock.get(), get(), int(flags));
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	std::error_code rtnl_addr_ref::remove(nl_socket_ptr const& sock, int flags) const noexcept {
		int err = ::rtnl_addr_delete(sock.get(), get(), int(flags));
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	std::error_code rtnl_addr_ref::set_label(QString const& label) const {
		int err = ::rtnl_addr_set_label(get(), label.toUtf8().data());
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	QString rtnl_addr_ref::get_label() const {
		return QString::fromUtf8(::rtnl_addr_get_label(get()));
	}

	void rtnl_addr_ref::set_ifindex(int ifIndex) const noexcept {
		::rtnl_addr_set_ifindex(get(), ifIndex);
	}
	int rtnl_addr_ref::get_ifindex() const noexcept {
		return ::rtnl_addr_get_ifindex(get());
	}

	void rtnl_addr_ref::set_link(rtnl_link_ref const& link) const noexcept {
		::rtnl_addr_set_link(get(), link.get());
	}
	rtnl_link_ref rtnl_addr_ref::get_link() const noexcept {
		return rtnl_link_ref::take_inc_ref(::rtnl_addr_get_link(get()));
	}

	void rtnl_addr_ref::set_family(int family) const noexcept {
		::rtnl_addr_set_family(get(), family);
	}
	int rtnl_addr_ref::get_family() const noexcept {
		return ::rtnl_addr_get_family(get());
	}

	void rtnl_addr_ref::set_prefixlen(int prefixlen) const noexcept {
		::rtnl_addr_set_prefixlen(get(), prefixlen);
	}
	int rtnl_addr_ref::get_prefixlen() const noexcept {
		return ::rtnl_addr_get_prefixlen(get());
	}

	void rtnl_addr_ref::set_scope(int scope) const noexcept {
		::rtnl_addr_set_scope(get(), scope);
	}
	int rtnl_addr_ref::get_scope() const noexcept {
		return ::rtnl_addr_get_scope(get());
	}

	void rtnl_addr_ref::set_flags(rtnl_addr_flags flags) const noexcept {
		::rtnl_addr_set_flags(get(), static_cast<unsigned int>(flags));
	}
	void rtnl_addr_ref::unset_flags(rtnl_addr_flags flags) const noexcept {
		::rtnl_addr_unset_flags(get(), static_cast<unsigned int>(flags));
	}
	rtnl_addr_flags rtnl_addr_ref::get_flags() const noexcept {
		return rtnl_addr_flags(::rtnl_addr_get_flags(get()));
	}

	std::error_code rtnl_addr_ref::set_local(nl_addr_ref const& local) const noexcept {
		int err = ::rtnl_addr_set_local(get(), local.get());
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	nl_addr_ref rtnl_addr_ref::get_local() const noexcept {
		return nl_addr_ref::take_inc_ref(::rtnl_addr_get_local(get()));
	}

	std::error_code rtnl_addr_ref::set_peer(nl_addr_ref const& peer) const noexcept {
		int err = ::rtnl_addr_set_peer(get(), peer.get());
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	nl_addr_ref rtnl_addr_ref::get_peer() const noexcept {
		return nl_addr_ref::take_inc_ref(::rtnl_addr_get_peer(get()));
	}

	std::error_code rtnl_addr_ref::set_broadcast(nl_addr_ref const& broadcast) const noexcept {
		int err = ::rtnl_addr_set_broadcast(get(), broadcast.get());
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	nl_addr_ref rtnl_addr_ref::get_broadcast() const noexcept {
		return nl_addr_ref::take_inc_ref(::rtnl_addr_get_broadcast(get()));
	}

	std::error_code rtnl_addr_ref::set_multicast(nl_addr_ref const& multicast) const noexcept {
		int err = ::rtnl_addr_set_multicast(get(), multicast.get());
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	nl_addr_ref rtnl_addr_ref::get_multicast() const noexcept {
		return nl_addr_ref::take_inc_ref(::rtnl_addr_get_multicast(get()));
	}

	std::error_code rtnl_addr_ref::set_anycast(nl_addr_ref const& anycast) const noexcept {
		int err = ::rtnl_addr_set_anycast(get(), anycast.get());
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	nl_addr_ref rtnl_addr_ref::get_anycast() const noexcept {
		return nl_addr_ref::take_inc_ref(::rtnl_addr_get_anycast(get()));
	}

	void rtnl_addr_ref::set_valid_lifetime(uint32_t valid_lifetime) const noexcept {
		::rtnl_addr_set_valid_lifetime(get(), valid_lifetime);
	}
	uint32_t rtnl_addr_ref::get_valid_lifetime() const noexcept {
		return ::rtnl_addr_get_valid_lifetime(get());
	}

	void rtnl_addr_ref::set_preferred_lifetime(uint32_t preferred_lifetime) const noexcept {
		::rtnl_addr_set_preferred_lifetime(get(), preferred_lifetime);
	}
	uint32_t rtnl_addr_ref::get_preferred_lifetime() const noexcept {
		return ::rtnl_addr_get_preferred_lifetime(get());
	}

	uint32_t rtnl_addr_ref::get_create_time() const noexcept {
		return ::rtnl_addr_get_create_time(get());
	}
	uint32_t rtnl_addr_ref::get_last_update_time() const noexcept {
		return ::rtnl_addr_get_last_update_time(get());
	}
}
