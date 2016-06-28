#include "netlink_rtnl_route.h"

#include "netlink_addr.h"
#include "netlink_rtnl_addr.h"
#include "netlink_rtnl_link.h"

extern "C" {
#include <netlink/route/route.h>
}

namespace netlink {
#if 0
	extern int	rtnl_route_parse(struct nlmsghdr *, struct rtnl_route **);

	extern int	rtnl_route_add(struct nl_sock *, struct rtnl_route *, int);
	extern int	rtnl_route_delete(struct nl_sock *, struct rtnl_route *, int);
#endif

	QString toString(nexthop_flags flags) {
		char buf[512];
		return QString::fromUtf8(::rtnl_route_nh_flags2str((unsigned int)(flags), buf, sizeof(buf)));
	}

	QString toString(route_proto proto) {
		char buf[64];
		return QString::fromUtf8(::rtnl_route_proto2str(uint8_t(proto), buf, sizeof(buf)));
	}

	QString toString(route_metric metric) {
		char buf[64];
		return QString::fromUtf8(::rtnl_route_metric2str(int(metric), buf, sizeof(buf)));
	}

	rtnl_nexthop rtnl_nexthop_ptr::clone() const noexcept {
		rtnl_nexthop nh;
		nh.m_managed_ptr.reset(::rtnl_route_nh_clone(m_nexthop));
		nh.m_nexthop = nh.m_managed_ptr.get();
		return nh;
	}

	::rtnl_nexthop const* rtnl_nexthop_ptr::get() const noexcept {
		return m_nexthop;
	}

	::rtnl_nexthop* rtnl_nexthop_ptr::get() noexcept {
		return m_nexthop;
	}

	void rtnl_nexthop_ptr::set_weight(uint8_t weight) noexcept {
		::rtnl_route_nh_set_weight(m_nexthop, weight);
	}

	uint8_t rtnl_nexthop_ptr::get_weight() const noexcept {
		return ::rtnl_route_nh_get_weight(m_nexthop);
	}

	void rtnl_nexthop_ptr::set_ifindex(int ifindex) noexcept {
		::rtnl_route_nh_set_ifindex(m_nexthop, ifindex);
	}

	int rtnl_nexthop_ptr::get_ifindex() const noexcept {
		return ::rtnl_route_nh_get_ifindex(m_nexthop);
	}

	void rtnl_nexthop_ptr::set_gateway(const nl_addr_ref& gateway) noexcept {
		::rtnl_route_nh_set_gateway(m_nexthop, gateway.get());
	}

	nl_addr_ref rtnl_nexthop_ptr::get_gateway() const noexcept {
		return nl_addr_ref::take_inc_ref(::rtnl_route_nh_get_gateway(m_nexthop));
	}

	void rtnl_nexthop_ptr::set_flags(nexthop_flags flags) noexcept {
		::rtnl_route_nh_set_flags(m_nexthop, (unsigned int)(flags));
	}

	void rtnl_nexthop_ptr::unset_flags(nexthop_flags flags) noexcept {
		::rtnl_route_nh_unset_flags(m_nexthop, (unsigned int)(flags));
	}

	nexthop_flags rtnl_nexthop_ptr::get_flags() const noexcept {
		return nexthop_flags(::rtnl_route_nh_get_flags(m_nexthop));
	}

	void rtnl_nexthop_ptr::set_realms(uint32_t realms) noexcept {
		::rtnl_route_nh_set_realms(m_nexthop, realms);
	}

	uint32_t rtnl_nexthop_ptr::get_realms() const noexcept {
		return ::rtnl_route_nh_get_realms(m_nexthop);
	}

	void rtnl_nexthop::deleter_t::operator()(::rtnl_nexthop* nh) noexcept {
		::rtnl_route_nh_free(nh);
	}

	rtnl_nexthop rtnl_nexthop::alloc() noexcept {
		rtnl_nexthop nh;
		nh.m_managed_ptr.reset(::rtnl_route_nh_alloc());
		nh.m_nexthop = nh.m_managed_ptr.get();
		return nh;
	}

	rtnl_nexthop rtnl_nexthop::take_own(::rtnl_nexthop* nh) noexcept {
		rtnl_nexthop res;
		res.m_managed_ptr.reset(nh);
		res.m_nexthop = nh;
		return res;
	}

	::rtnl_nexthop* rtnl_nexthop::release() noexcept {
		return m_managed_ptr.release();
	}

	rtnl_route_ref rtnl_route_ref::alloc() noexcept {
		return rtnl_route_ref::take_own(::rtnl_route_alloc());
	}

	std::error_code rtnl_route_ref::add(nl_socket_ptr const& sock, new_flags flags) const noexcept {
		int err = ::rtnl_route_add(sock.get(), get(), int(flags));
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	std::error_code rtnl_route_ref::remove(nl_socket_ptr const& sock, int flags) const noexcept {
		int err = ::rtnl_route_delete(sock.get(), get(), int(flags));
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}

	void rtnl_route_ref::set_table(uint32_t table) const noexcept {
		::rtnl_route_set_table(get(), table);
	}
	uint32_t rtnl_route_ref::get_table() const noexcept {
		return ::rtnl_route_get_table(get());
	}

	void rtnl_route_ref::set_scope(uint8_t scope) const noexcept {
		::rtnl_route_set_scope(get(), scope);
	}
	uint8_t rtnl_route_ref::get_scope() const noexcept {
		return ::rtnl_route_get_scope(get());
	}

	void rtnl_route_ref::set_tos(uint8_t tos) const noexcept {
		::rtnl_route_set_tos(get(), tos);
	}
	uint8_t rtnl_route_ref::get_tos() const noexcept {
		return ::rtnl_route_get_tos(get());
	}

	void rtnl_route_ref::set_protocol(uint8_t protocol) const noexcept {
		::rtnl_route_set_protocol(get(), protocol);
	}
	uint8_t rtnl_route_ref::get_protocol() const noexcept {
		return ::rtnl_route_get_protocol(get());
	}

	void rtnl_route_ref::set_priority(uint32_t priority) const noexcept {
		::rtnl_route_set_priority(get(), priority);
	}
	uint32_t rtnl_route_ref::get_priority() const noexcept {
		return ::rtnl_route_get_priority(get());
	}

	std::error_code rtnl_route_ref::set_family(uint8_t family) const noexcept {
		int err = ::rtnl_route_set_family(get(), family);
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	uint8_t rtnl_route_ref::get_family() const noexcept {
		return ::rtnl_route_get_family(get());
	}

	std::error_code rtnl_route_ref::set_type(uint8_t type) const noexcept {
		int err = ::rtnl_route_set_type(get(), type);
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	uint8_t rtnl_route_ref::get_type() const noexcept {
		return ::rtnl_route_get_type(get());
	}

	void rtnl_route_ref::set_flags(uint32_t flags) const noexcept {
		::rtnl_route_set_flags(get(), flags);
	}
	void rtnl_route_ref::unset_flags(uint32_t flags) const noexcept {
		::rtnl_route_unset_flags(get(), flags);
	}
	uint32_t rtnl_route_ref::get_flags() const noexcept {
		return ::rtnl_route_get_flags(get());
	}

	std::error_code rtnl_route_ref::set_metric(route_metric metric, unsigned int value) const noexcept {
		int err = ::rtnl_route_set_metric(get(), int(metric), value);
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	std::error_code rtnl_route_ref::unset_metric(route_metric metric) const noexcept {
		int err = ::rtnl_route_unset_metric(get(), int(metric));
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	unsigned int rtnl_route_ref::get_metric(route_metric metric, std::error_code& ec) const noexcept {
		unsigned int value{0};
		int err = ::rtnl_route_get_metric(get(), int(metric), &value);
		if (0 > err) {
			ec = make_netlink_error_code(err);
		}
		return value;
	}

	std::error_code rtnl_route_ref::set_dst(nl_addr_ref const& dst) const noexcept {
		int err = ::rtnl_route_set_dst(get(), dst.get());
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	nl_addr_ref rtnl_route_ref::get_dst() const noexcept {
		return nl_addr_ref::take_inc_ref(::rtnl_route_get_dst(get()));
	}

	std::error_code rtnl_route_ref::set_src(nl_addr_ref const& src) const noexcept {
		int err = ::rtnl_route_set_src(get(), src.get());
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	nl_addr_ref rtnl_route_ref::get_src() const noexcept {
		return nl_addr_ref::take_inc_ref(::rtnl_route_get_src(get()));
	}

	std::error_code rtnl_route_ref::set_pref_src(nl_addr_ref const& pref_src) const noexcept {
		int err = ::rtnl_route_set_pref_src(get(), pref_src.get());
		return 0 > err ? make_netlink_error_code(err) : std::error_code{};
	}
	nl_addr_ref rtnl_route_ref::get_pref_src() const noexcept {
		return nl_addr_ref::take_inc_ref(::rtnl_route_get_pref_src(get()));
	}

	void rtnl_route_ref::set_iif(int iif) const noexcept {
		::rtnl_route_set_iif(get(), iif);
	}
	int rtnl_route_ref::get_iif() const noexcept {
		return ::rtnl_route_get_iif(get());
	}

	int rtnl_route_ref::guess_scope() const noexcept {
		return ::rtnl_route_guess_scope(get());
	}

	void rtnl_route_ref::add_nexthop(rtnl_nexthop nh) const noexcept {
		::rtnl_route_add_nexthop(get(), nh.release());
	}
	rtnl_nexthop rtnl_route_ref::remove_nexthop(rtnl_nexthop_ptr const& nh) const noexcept {
		::rtnl_nexthop* nh_ptr = const_cast<::rtnl_nexthop*>(nh.get());
		::rtnl_route_remove_nexthop(get(), nh_ptr);
		// now somebody else needs to manage the nexthop
		return rtnl_nexthop::take_own(nh_ptr);
	}
	int rtnl_route_ref::get_nnexthops() const noexcept {
		return ::rtnl_route_get_nnexthops(get());
	}
	rtnl_nexthop_ptr rtnl_route_ref::nexthop_n(int ndx) const noexcept {
		return rtnl_nexthop_ptr(::rtnl_route_nexthop_n(get(), ndx));
	}

	static void route_foreach_nexthop_helper(::rtnl_nexthop* nh, void* arg) {
		auto const cb = reinterpret_cast<std::function<void(rtnl_nexthop_ptr const&)> const*>(arg);
		(*cb)(rtnl_nexthop_ptr(nh));
	}

	void rtnl_route_ref::foreach_nexthop(std::function<void(rtnl_nexthop_ptr const&)> cb) const {
		::rtnl_route_foreach_nexthop(
			get(),
			route_foreach_nexthop_helper,
			const_cast<void*>(reinterpret_cast<void const*>(&cb)));
	}
}
