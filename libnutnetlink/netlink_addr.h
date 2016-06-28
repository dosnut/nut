#ifndef _NUT_NETLINK_ADDR_H
#define _NUT_NETLINK_ADDR_H

#pragma once

#include "netlink.h"

#include <QString>
#include <QHostAddress>

namespace netlink {
	class nl_addr_ref : public internal::generic_ref<internal::desc_nl_addr> {
	public:
		static nl_addr_ref alloc_zero(size_t max_addr_len) noexcept;
		static nl_addr_ref create(int family, void const* buf, size_t size) noexcept;
		static nl_addr_ref create(QHostAddress const& addr) noexcept;
		static nl_addr_ref create(QPair<QHostAddress, int> const& network) noexcept;
		static nl_addr_ref parse(char const* addr, int family_hint, std::error_code& ec) noexcept;

		void set_family(int family) const noexcept;
		int get_family() const noexcept;

		void set_binary_addr(void const* buf, size_t size) const noexcept;
		void* get_binary_addr() const noexcept;
		size_t get_len() const noexcept;

		void set_prefixlen(unsigned int prefixlen) const noexcept;
		unsigned int get_prefixlen() const noexcept;

		QString toString() const;

		bool iszero() const noexcept;
		int guess_family() const noexcept;

		/* without prefix len */
		friend int cmp(nl_addr_ref const& a, nl_addr_ref const& b) noexcept;
		/* with prefix len */
		friend int cmp_prefix(nl_addr_ref const& a, nl_addr_ref const& b) noexcept;
	};

}

#endif /* _NUT_NETLINK_ADDR_H */
