#ifndef _NUT_NETLINK_RTNL_ADDR_H
#define _NUT_NETLINK_RTNL_ADDR_H

#pragma once

#include "netlink_sock.h"

#include <QString>

namespace netlink {
	enum class rtnl_addr_flag : unsigned int {
		secondary = IFA_F_SECONDARY,
		temporary = IFA_F_TEMPORARY,
		nodad = IFA_F_NODAD,
		optimistic = IFA_F_OPTIMISTIC,
		dadfailed = IFA_F_DADFAILED,
		homeaddress = IFA_F_HOMEADDRESS,
		deprecated = IFA_F_DEPRECATED,
		tentative = IFA_F_TENTATIVE,
		permanent = IFA_F_PERMANENT,
		managetempaddr = IFA_F_MANAGETEMPADDR,
		noprefixroute = IFA_F_NOPREFIXROUTE,
		mcautojoin = IFA_F_MCAUTOJOIN,
		stable_privacy = IFA_F_STABLE_PRIVACY,
	};
	using rtnl_addr_flags = QFlags<rtnl_addr_flag>;
	Q_DECLARE_OPERATORS_FOR_FLAGS(rtnl_addr_flags)
	QString toString(rtnl_addr_flags flags);

	class rtnl_addr_ref : public internal::object_ref<internal::desc_rtnl_addr> {
	public:
		static rtnl_addr_ref alloc();

		std::error_code add(nl_socket_ptr const& sock, new_flags flags = new_flags{}) const noexcept;
		std::error_code remove(nl_socket_ptr const& sock, int flags = 0) const noexcept;

		std::error_code set_label(QString const& label) const;
		QString get_label() const;

		void set_ifindex(int ifIndex) const noexcept;
		int get_ifindex() const noexcept;

		void set_link(rtnl_link_ref const& link) const noexcept;
		rtnl_link_ref get_link() const noexcept;

		void set_family(int family) const noexcept;
		int get_family() const noexcept;

		void set_prefixlen(int prefixlen) const noexcept;
		int get_prefixlen() const noexcept;

		void set_scope(int scope) const noexcept;
		int get_scope() const noexcept;

		void set_flags(rtnl_addr_flags flags) const noexcept;
		void unset_flags(rtnl_addr_flags flags) const noexcept;
		rtnl_addr_flags get_flags() const noexcept;

		std::error_code set_local(nl_addr_ref const& local) const noexcept;
		nl_addr_ref get_local() const noexcept;

		std::error_code set_peer(nl_addr_ref const& peer) const noexcept;
		nl_addr_ref get_peer() const noexcept;

		std::error_code set_broadcast(nl_addr_ref const& broadcast) const noexcept;
		nl_addr_ref get_broadcast() const noexcept;

		std::error_code set_multicast(nl_addr_ref const& multicast) const noexcept;
		nl_addr_ref get_multicast() const noexcept;

		std::error_code set_anycast(nl_addr_ref const& anycast) const noexcept;
		nl_addr_ref get_anycast() const noexcept;

		void set_valid_lifetime(uint32_t valid_lifetime) const noexcept;
		uint32_t get_valid_lifetime() const noexcept;

		void set_preferred_lifetime(uint32_t preferred_lifetime) const noexcept;
		uint32_t get_preferred_lifetime() const noexcept;

		uint32_t get_create_time() const noexcept;
		uint32_t get_last_update_time() const noexcept;
	};
}

#endif /* _NUT_NETLINK_RTNL_ADDR_H */
