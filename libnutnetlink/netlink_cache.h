#ifndef _NUT_NETLINK_CACHE_H
#define _NUT_NETLINK_CACHE_H

#pragma once

#include "netlink.h"

extern "C" {
#include <netlink/cache.h>
}

#include <QString>
#include <QFlags>

#include <iterator>

namespace netlink {
	enum class nl_cache_flag : unsigned int {
		iter = NL_CACHE_AF_ITER,
	};
	using nl_cache_flags = QFlags<nl_cache_flag>;
	Q_DECLARE_OPERATORS_FOR_FLAGS(nl_cache_flags)

	class nl_cache_untyped_ref : protected internal::generic_ref<internal::desc_nl_cache> {
	private:
		using parent_t = internal::generic_ref<internal::desc_nl_cache>;
	public:
		using parent_t::get;
		using parent_t::reset;

		int nitems() const noexcept;
		void clear() const noexcept;

		bool is_empty() const noexcept;

		/* usually used to specify address family.
		 *
		 * if nl_cache_flag::iter is set and the cache supports
		 * multiple families refill will iterate over all families
		 * using arg1 to request them all.
		 */
		void set_arg1(int arg) const noexcept;

		void set_arg2(int arg) const noexcept;

		void set_flags(nl_cache_flags flags) const noexcept;

		std::error_code refill(nl_socket_ptr const& sock) const noexcept;
	};

	template<typename Descriptor>
	class nl_cache_ref;

	template<typename Descriptor>
	class nl_cache_iterator : std::iterator<
			std::bidirectional_iterator_tag,
			typename Descriptor::managed_t,
			typename Descriptor::managed_t,
			typename Descriptor::managed_t> {
	public:
		static_assert(Descriptor::cache_name, "object not allowed in caches");
		explicit constexpr nl_cache_iterator() noexcept {
		}

		using element_raw_t = typename Descriptor::object_t;
		using element_t = typename Descriptor::managed_t;

		element_t operator*() const noexcept;

		element_t operator->() const noexcept;

		nl_cache_iterator& operator++() noexcept;

		nl_cache_iterator operator++(int) noexcept;

		nl_cache_iterator& operator--() noexcept;

		nl_cache_iterator operator--(int) noexcept;

		friend bool operator==(nl_cache_iterator const& a, nl_cache_iterator const& b) noexcept {
			return a.m_current == b.m_current && a.m_at_end == b.m_at_end;
		}

		friend bool operator!=(nl_cache_iterator const& a, nl_cache_iterator const& b) noexcept {
			return !(a == b);
		}

	private:
		friend class nl_cache_ref<Descriptor>;

		nl_object* m_current{nullptr};
		bool m_at_end{true};
	};

	template<typename Descriptor>
	class nl_cache_ref : public nl_cache_untyped_ref {
	public:
		static_assert(Descriptor::cache_name, "object not allowed in caches");
		using element_raw_t = typename Descriptor::object_t;
		using element_t = typename Descriptor::managed_t;
		using iterator = nl_cache_iterator<Descriptor>;

		static nl_cache_ref alloc(std::error_code& ec) noexcept;
		static nl_cache_ref alloc();

		int nitems_filter(element_t const& filter) const noexcept;

		nl_cache_ref clone() const noexcept;
		nl_cache_ref subset(element_t const& filter) const noexcept;

		element_t search(element_t const& needle) const noexcept;
		element_t find(element_t const& filter) const noexcept;

		iterator begin() const noexcept;
		iterator end() const noexcept;
	};

	extern template class nl_cache_iterator<internal::desc_rtnl_addr>;
	extern template class nl_cache_iterator<internal::desc_rtnl_link>;
	extern template class nl_cache_iterator<internal::desc_rtnl_route>;

	extern template class nl_cache_ref<internal::desc_rtnl_addr>;
	extern template class nl_cache_ref<internal::desc_rtnl_link>;
	extern template class nl_cache_ref<internal::desc_rtnl_route>;

	using rtnl_addr_cache_ref = nl_cache_ref<internal::desc_rtnl_addr>;
	using rtnl_link_cache_ref = nl_cache_ref<internal::desc_rtnl_link>;
	using rtnl_route_cache_ref = nl_cache_ref<internal::desc_rtnl_route>;
}

#endif /* _NUT_NETLINK_CACHE_H */
