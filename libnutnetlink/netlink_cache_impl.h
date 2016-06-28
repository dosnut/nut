#ifndef _NUT_NETLINK_CACHE_IMPL_H
#define _NUT_NETLINK_CACHE_IMPL_H

#pragma once

#include "netlink_cache.h"

namespace netlink {
	template<typename Descriptor>
	typename nl_cache_iterator<Descriptor>::element_t nl_cache_iterator<Descriptor>::operator*() const noexcept {
		return element_t::take_inc_ref(reinterpret_cast<element_raw_t*>(m_current));
	}

	template<typename Descriptor>
	typename nl_cache_iterator<Descriptor>::element_t nl_cache_iterator<Descriptor>::operator->() const noexcept {
		return operator *();
	}

	template<typename Descriptor>
	nl_cache_iterator<Descriptor>& nl_cache_iterator<Descriptor>::operator++() noexcept { // pre increment
		auto next = ::nl_cache_get_next(m_current);
		if (next) {
			m_current = next;
		} else {
			m_at_end = true;
		}
		return *this;
	}

	template<typename Descriptor>
	nl_cache_iterator<Descriptor> nl_cache_iterator<Descriptor>::operator++(int) noexcept { // post increment
		nl_cache_iterator copy(*this);
		operator++();
		return copy;
	}

	template<typename Descriptor>
	nl_cache_iterator<Descriptor>& nl_cache_iterator<Descriptor>::operator--() noexcept { // pre decrement
		if (m_at_end) {
			m_at_end = false;
		} else {
			m_current = ::nl_cache_get_prev(m_current);
		}
		return *this;
	}

	template<typename Descriptor>
	nl_cache_iterator<Descriptor> nl_cache_iterator<Descriptor>::operator--(int) noexcept { // post decrement
		nl_cache_iterator copy(*this);
		operator--();
		return copy;
	}

	template<typename Descriptor>
	nl_cache_ref<Descriptor> nl_cache_ref<Descriptor>::alloc(std::error_code& ec) noexcept {
		nl_cache_ref res;
		int err = ::nl_cache_alloc_name(Descriptor::cache_name, res.reset_and_get_ptr_ref());
		if (0 > err) ec = make_netlink_error_code(err);
		return res;
	}

	template<typename Descriptor>
	nl_cache_ref<Descriptor> nl_cache_ref<Descriptor>::alloc() {
		std::error_code ec;
		auto res = alloc(ec);
		if (ec) throw ec;
		return res;
	}

	template<typename Descriptor>
	int nl_cache_ref<Descriptor>::nitems_filter(element_t const& filter) const noexcept {
		return ::nl_cache_nitems_filter(get(), filter.get_object());
	}

	template<typename Descriptor>
	nl_cache_ref<Descriptor> nl_cache_ref<Descriptor>::clone() const noexcept {
		nl_cache_ref res;
		static_cast<nl_cache_untyped_ref&>(res) = nl_cache_untyped_ref::clone();
		return res;
	}

	template<typename Descriptor>
	nl_cache_ref<Descriptor> nl_cache_ref<Descriptor>::subset(element_t const& filter) const noexcept {
		nl_cache_ref res;
		res.set_own(::nl_cache_subset(get(), filter.get_object()));
		return res;
	}

	template<typename Descriptor>
	typename nl_cache_ref<Descriptor>::element_t nl_cache_ref<Descriptor>::search(element_t const& needle) const noexcept {
		::nl_object* o = ::nl_cache_search(get(), needle.get_object());
		return o ? element_t::take_own(reinterpret_cast<element_raw_t*>(o)) : element_t{};
	}

	template<typename Descriptor>
	typename nl_cache_ref<Descriptor>::element_t nl_cache_ref<Descriptor>::find(element_t const& filter) const noexcept {
		::nl_object* o = ::nl_cache_find(get(), filter.get_object());
		return o ? element_t::take_own(reinterpret_cast<element_raw_t*>(o)) : element_t{};
	}

	template<typename Descriptor>
	typename nl_cache_ref<Descriptor>::iterator nl_cache_ref<Descriptor>::begin() const noexcept {
		iterator i;
		i.m_current = ::nl_cache_get_first(get());
		i.m_at_end = false;
		return i;
	}

	template<typename Descriptor>
	typename nl_cache_ref<Descriptor>::iterator nl_cache_ref<Descriptor>::end() const noexcept {
		iterator i;
		i.m_current = ::nl_cache_get_last(get());
		i.m_at_end = true;
		return i;
	}
}

#endif /* _NUT_NETLINK_CACHE_IMPL_H */
