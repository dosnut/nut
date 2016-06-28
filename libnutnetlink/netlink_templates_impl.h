#ifndef _NUT_NETLINK_TEMPLATES_IMPL_H
#define _NUT_NETLINK_TEMPLATES_IMPL_H

#pragma once

#include "netlink_templates.h"

namespace netlink {
	namespace internal {
		template<typename Descriptor>
		generic_ref_no_clone<Descriptor>::generic_ref_no_clone(generic_ref_no_clone const& other) noexcept
		: m_ptr(other.m_ptr) {
			if (*this) Descriptor::acquire(m_ptr);
		}

		template<typename Descriptor>
		generic_ref_no_clone<Descriptor>::generic_ref_no_clone(generic_ref_no_clone&& other) noexcept
		: m_ptr(other.m_ptr) {
			other.m_ptr = nullptr;
		}

		template<typename Descriptor>
		generic_ref_no_clone<Descriptor>::~generic_ref_no_clone() noexcept {
			reset();
		}

		template<typename Descriptor>
		generic_ref_no_clone<Descriptor>& generic_ref_no_clone<Descriptor>::operator=(generic_ref_no_clone const& other) noexcept {
			set_inc_ref(other.get());
			return *this;
		}

		template<typename Descriptor>
		generic_ref_no_clone<Descriptor>& generic_ref_no_clone<Descriptor>::operator=(generic_ref_no_clone&& other) noexcept {
			if (this != &other) {
				reset();
				m_ptr = other.m_ptr;
				other.m_ptr = nullptr;
			}
			return *this;
		}

		template<typename Descriptor>
		typename generic_ref_no_clone<Descriptor>::type* generic_ref_no_clone<Descriptor>::get() const noexcept {
			return m_ptr;
		}

		template<typename Descriptor>
		generic_ref_no_clone<Descriptor>::operator type*() const noexcept {
			return get();
		}

		template<typename Descriptor>
		generic_ref_no_clone<Descriptor>::operator bool() const noexcept {
			return m_ptr;
		}

		template<typename Descriptor>
		void generic_ref_no_clone<Descriptor>::reset() noexcept {
			if (*this) {
				Descriptor::release(m_ptr);
				m_ptr = nullptr;
			}
		}

		template<typename Descriptor>
		void generic_ref_no_clone<Descriptor>::set_own(type* ptr) noexcept {
			if (ptr == m_ptr) return;
			reset();
			m_ptr = ptr;
			/* own ref count from caller now */
		}

		template<typename Descriptor>
		typename generic_ref_no_clone<Descriptor>::managed_t generic_ref_no_clone<Descriptor>::take_own(type* ptr) noexcept {
			managed_t ref;
			ref.set_own(ptr);
			return ref;
		}

		template<typename Descriptor>
		void generic_ref_no_clone<Descriptor>::set_inc_ref(type* ptr) noexcept {
			if (ptr == m_ptr) return;
			reset();
			if (ptr) {
				m_ptr = ptr;
				Descriptor::acquire(m_ptr);
			}
		}

		template<typename Descriptor>
		typename generic_ref_no_clone<Descriptor>::managed_t generic_ref_no_clone<Descriptor>::take_inc_ref(type* ptr) noexcept {
			managed_t ref;
			ref.set_inc_ref(ptr);
			return ref;
		}

		template<typename Descriptor>
		typename generic_ref_no_clone<Descriptor>::type** generic_ref_no_clone<Descriptor>::reset_and_get_ptr_ref() noexcept {
			reset();
			return &m_ptr;
		}

		template<typename Descriptor>
		object_ref<Descriptor>::object_ref(object_ref const& other) noexcept
		: m_ptr(other.m_ptr) {
			if (*this) ::nl_object_get(get_object());
		}

		template<typename Descriptor>
		object_ref<Descriptor>::object_ref(object_ref&& other) noexcept
		: m_ptr(other.m_ptr) {
			other.m_ptr = nullptr;
		}

		template<typename Descriptor>
		object_ref<Descriptor>::~object_ref() noexcept {
			reset();
		}

		template<typename Descriptor>
		object_ref<Descriptor>& object_ref<Descriptor>::operator=(object_ref const& other) noexcept {
			set_inc_ref(other.get());
			return *this;
		}

		template<typename Descriptor>
		object_ref<Descriptor>& object_ref<Descriptor>::operator=(object_ref&& other) noexcept {
			if (this != &other) {
				reset();
				m_ptr = other.m_ptr;
				other.m_ptr = nullptr;
			}
			return *this;
		}

		template<typename Descriptor>
		typename object_ref<Descriptor>::object_t* object_ref<Descriptor>::safe_cast(nl_object* ptr) noexcept {
			if (ptr && 0 == strcmp(nl_object_get_type(ptr), Descriptor::name)) {
				return reinterpret_cast<object_t*>(ptr);
			}
			return nullptr;
		}

		namespace {
			template<typename Descriptor>
			struct parse_msg_ctx {
				object_ref<Descriptor> *res;
				std::error_code* ec;
				static void parse_cb(::nl_object* obj, void *arg) {
					if (!obj) return;
					auto self = reinterpret_cast<parse_msg_ctx*>(arg);
					auto ptr = object_ref<Descriptor>::safe_cast(obj);
					if (ptr) {
						*self->ec = std::error_code{};
						self->res->set_inc_ref(ptr);
					} else {
						*self->ec = make_error_code(errc::obj_mismatch);
					}
				}
			};
		}

		template<typename Descriptor>
		typename object_ref<Descriptor>::managed_t object_ref<Descriptor>::parse_msg(::nl_msg* msg, std::error_code& ec) noexcept {
			using ctx_t = parse_msg_ctx<Descriptor>;
			managed_t res;
			ctx_t ctx;
			ctx.res = &res;
			ctx.ec = &ec;
			ec = make_error_code(errc::obj_notfound);
			int err = ::nl_msg_parse(msg, &ctx_t::parse_cb, reinterpret_cast<void*>(&ctx));
			if (0 > err) ec = make_netlink_error_code(err);
			return res;
		}

		template<typename Descriptor>
		typename object_ref<Descriptor>::managed_t object_ref<Descriptor>::parse_msg(::nl_msg* msg) {
			std::error_code ec;
			auto res = parse_msg(msg, ec);
			if (ec) throw ec;
			return res;
		}

		template<typename Descriptor>
		typename object_ref<Descriptor>::object_t* object_ref<Descriptor>::get() const noexcept {
			return m_ptr;
		}

		template<typename Descriptor>
		::nl_object* object_ref<Descriptor>::get_object() const noexcept {
			return reinterpret_cast<::nl_object*>(m_ptr);
		}

		template<typename Descriptor>
		object_ref<Descriptor>::operator object_t*() const noexcept {
			return get();
		}

		template<typename Descriptor>
		object_ref<Descriptor>::operator ::nl_object*() const noexcept {
			return get_object();
		}

		template<typename Descriptor>
		object_ref<Descriptor>::operator bool() const noexcept {
			return m_ptr;
		}

		template<typename Descriptor>
		void object_ref<Descriptor>::reset() noexcept {
			if (*this) {
				::nl_object_put(get_object());
				m_ptr = nullptr;
			}
		}

		template<typename Descriptor>
		void object_ref<Descriptor>::set_own(object_t* ptr) noexcept {
			if (ptr == m_ptr) return;
			reset();
			m_ptr = ptr;
			/* own ref count from caller now */
		}

		template<typename Descriptor>
		typename object_ref<Descriptor>::managed_t object_ref<Descriptor>::take_own(object_t* ptr) noexcept {
			managed_t ref;
			ref.set_own(ptr);
			return ref;
		}

		template<typename Descriptor>
		void object_ref<Descriptor>::set_inc_ref(object_t* ptr) noexcept {
			if (ptr == m_ptr) return;
			reset();
			if (ptr) {
				m_ptr = ptr;
				::nl_object_get(get_object());
			}
		}

		template<typename Descriptor>
		typename object_ref<Descriptor>::managed_t object_ref<Descriptor>::take_inc_ref(object_t* ptr) noexcept {
			managed_t ref;
			ref.set_inc_ref(ptr);
			return ref;
		}

		template<typename Descriptor>
		typename object_ref<Descriptor>::object_t** object_ref<Descriptor>::reset_and_get_ptr_ref() noexcept {
			reset();
			return &m_ptr;
		}

		template<typename Descriptor>
		typename object_ref<Descriptor>::managed_t object_ref<Descriptor>::clone() const noexcept {
			managed_t res;
			res.set_own(reinterpret_cast<object_t*>(nl_object_clone(get_object())));
			return res;
		}

		template<typename Descriptor>
		QString object_ref<Descriptor>::toString() const noexcept {
			char buf[1024];
			::nl_object_dump_buf(get_object(), buf, sizeof(buf));
			size_t len = strnlen(buf, sizeof(buf));
			while (len > 0 && isspace(buf[len])) --len;
			return QString::fromUtf8(buf, len);
		}
	}
}

#endif /* _NUT_NETLINK_TEMPLATES_IMPL_H */
