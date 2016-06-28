#ifndef _NUT_NETLINK_RTNL_ROUTE_H
#define _NUT_NETLINK_RTNL_ROUTE_H

#pragma once

#include "netlink_sock.h"

#include <QString>
#include <QFlags>

#include <iterator>
#include <memory>

namespace netlink {
	enum class nexthop_flag : unsigned int {
		dead = RTNH_F_DEAD, /* Nexthop is dead (used by multipath) */
		pervasive = RTNH_F_PERVASIVE, /* Do recursive gateway lookup */
		onlink = RTNH_F_ONLINK, /* Gateway is forced on link */
		offload = RTNH_F_OFFLOAD, /* offloaded route */
		linkdown = RTNH_F_LINKDOWN, /* carrier-down on nexthop */
	};
	using nexthop_flags = QFlags<nexthop_flag>;
	Q_DECLARE_OPERATORS_FOR_FLAGS(nexthop_flags)
	QString toString(nexthop_flags flags);

	enum class route_proto : uint8_t {
		unspec = RTPROT_UNSPEC,
		redirect = RTPROT_REDIRECT, /* Route installed by ICMP redirects; not used by current IPv4 */
		kernel = RTPROT_KERNEL,     /* Route installed by kernel */
		boot = RTPROT_BOOT,         /* Route installed during boot */
		static_ = RTPROT_STATIC,     /* Route installed by administrator */
	};
	QString toString(route_proto proto);

	enum class route_metric : int {
		unspec = RTAX_UNSPEC,
		lock = RTAX_LOCK,
		mtu = RTAX_MTU,
		window = RTAX_WINDOW,
		rtt = RTAX_RTT,
		rttvar = RTAX_RTTVAR,
		ssthresh = RTAX_SSTHRESH,
		cwnd = RTAX_CWND,
		advmss = RTAX_ADVMSS,
		reordering = RTAX_REORDERING,
		hoplimit = RTAX_HOPLIMIT,
		initcwnd = RTAX_INITCWND,
		features = RTAX_FEATURES,
		rto_min = RTAX_RTO_MIN,
		initrwnd = RTAX_INITRWND,
		quickack = RTAX_QUICKACK,
		cc_algo = RTAX_CC_ALGO,
	};
	QString toString(route_metric metric);

	class rtnl_nexthop;

	/* does not own the nexthop object */
	class rtnl_nexthop_ptr {
	public:
		explicit constexpr rtnl_nexthop_ptr() noexcept = default;
		explicit constexpr rtnl_nexthop_ptr(::rtnl_nexthop *nh) noexcept
		: m_nexthop(nh) {
		}
		~rtnl_nexthop_ptr() noexcept = default;
		rtnl_nexthop_ptr(rtnl_nexthop_ptr const&) noexcept = default;
		rtnl_nexthop_ptr(rtnl_nexthop_ptr&&) noexcept = default;
		rtnl_nexthop_ptr& operator=(rtnl_nexthop_ptr const&) noexcept = default;
		rtnl_nexthop_ptr& operator=(rtnl_nexthop_ptr&&) noexcept = default;

		static rtnl_nexthop_ptr alloc() noexcept;

		rtnl_nexthop clone() const noexcept;
		::rtnl_nexthop const* get() const noexcept;
		::rtnl_nexthop* get() noexcept;

		void set_weight(uint8_t weight) noexcept;
		uint8_t get_weight() const noexcept;

		void set_ifindex(int ifindex) noexcept;
		int get_ifindex() const noexcept;

		void set_gateway(nl_addr_ref const& gateway) noexcept;
		nl_addr_ref get_gateway() const noexcept;

		void set_flags(nexthop_flags flags) noexcept;
		void unset_flags(nexthop_flags flags) noexcept;
		nexthop_flags get_flags() const noexcept;

		void set_realms(uint32_t realms) noexcept;
		uint32_t get_realms() const noexcept;

	protected:
		::rtnl_nexthop* m_nexthop{nullptr};
	};

	class rtnl_nexthop : public rtnl_nexthop_ptr {
	private:
		struct deleter_t {
			void operator()(::rtnl_nexthop* nh) noexcept;
		};
		using ptr_t = std::unique_ptr<::rtnl_nexthop, deleter_t>;

	public:
		explicit constexpr rtnl_nexthop() noexcept = default;
		~rtnl_nexthop() noexcept = default;
		rtnl_nexthop(rtnl_nexthop&&) noexcept = default;
		rtnl_nexthop& operator=(rtnl_nexthop&&) noexcept = default;

		static rtnl_nexthop alloc() noexcept;
		static rtnl_nexthop take_own(::rtnl_nexthop* nh) noexcept;

		::rtnl_nexthop* release() noexcept;

	private:
		friend class rtnl_nexthop_ptr;
		ptr_t m_managed_ptr{nullptr};
	};

	class rtnl_route_ref : public internal::object_ref<internal::desc_rtnl_route> {
	public:
		static rtnl_route_ref alloc() noexcept;

		std::error_code add(nl_socket_ptr const& sock, new_flags flags = new_flags{}) const noexcept;
		std::error_code remove(nl_socket_ptr const& sock, int flags = 0) const noexcept;

		void set_table(uint32_t table) const noexcept;
		uint32_t get_table() const noexcept;

		void set_scope(uint8_t scope) const noexcept;
		uint8_t get_scope() const noexcept;

		void set_tos(uint8_t tos) const noexcept;
		uint8_t get_tos() const noexcept;

		void set_protocol(uint8_t protocol) const noexcept;
		uint8_t get_protocol() const noexcept;

		void set_priority(uint32_t priority) const noexcept;
		uint32_t get_priority() const noexcept;

		std::error_code  set_family(uint8_t family) const noexcept;
		uint8_t get_family() const noexcept;

		std::error_code set_type(uint8_t type) const noexcept;
		uint8_t get_type() const noexcept;

		void set_flags(uint32_t flags) const noexcept;
		void unset_flags(uint32_t) const noexcept;
		uint32_t get_flags() const noexcept;

		std::error_code set_metric(route_metric metric, unsigned int value) const noexcept;
		std::error_code unset_metric(route_metric metric) const noexcept;
		unsigned int get_metric(route_metric metric, std::error_code& ec) const noexcept;

		std::error_code set_dst(nl_addr_ref const& dst) const noexcept;
		nl_addr_ref get_dst() const noexcept;

		std::error_code set_src(nl_addr_ref const& src) const noexcept;
		nl_addr_ref get_src() const noexcept;

		std::error_code set_pref_src(nl_addr_ref const& pref_src) const noexcept;
		nl_addr_ref get_pref_src() const noexcept;

		void set_iif(int iif) const noexcept;
		int get_iif() const noexcept;

		int guess_scope() const noexcept;

		void add_nexthop(rtnl_nexthop nh) const noexcept;
		rtnl_nexthop remove_nexthop(rtnl_nexthop_ptr const& nh) const noexcept;
		int get_nnexthops() const noexcept;
		rtnl_nexthop_ptr nexthop_n(int ndx) const noexcept;
		void foreach_nexthop(std::function<void(rtnl_nexthop_ptr const&)> cb) const;
	};
}

#endif /* _NUT_NETLINK_RTNL_ROUTE_H */
