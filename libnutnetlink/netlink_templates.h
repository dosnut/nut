#ifndef _NUT_NETLINK_TEMPLATES_H
#define _NUT_NETLINK_TEMPLATES_H

#pragma once

#include <QString>

extern "C" {
struct nl_msg;
struct nl_object;
}

namespace netlink {
	namespace internal {
		template<typename Descriptor>
		class generic_ref_no_clone {
		public:
			using type = typename Descriptor::type;
			using managed_t = typename Descriptor::managed_t;

		protected:
			explicit constexpr generic_ref_no_clone() noexcept;
			generic_ref_no_clone(generic_ref_no_clone const& other) noexcept;
			generic_ref_no_clone(generic_ref_no_clone&& other) noexcept;
			/* not virtual */
			~generic_ref_no_clone() noexcept;
			generic_ref_no_clone& operator=(generic_ref_no_clone const& other) noexcept;
			generic_ref_no_clone& operator=(generic_ref_no_clone&& other) noexcept;

		public:
			type* get() const noexcept;

			/* implicit */ operator type*() const noexcept;
			explicit operator bool() const noexcept;

			void reset() noexcept;

			/* take ownership of pointer */
			void set_own(type* ptr) noexcept;
			static managed_t take_own(type* ptr) noexcept;
			/* ptr is managed by caller, increment refcount */
			void set_inc_ref(type* ptr) noexcept;
			static managed_t take_inc_ref(type* ptr) noexcept;

			/* some api functions want pointers to pointers to store new objects in */
			type** reset_and_get_ptr_ref() noexcept;

			managed_t clone() const noexcept;

		protected:
			type* m_ptr{nullptr};
		};

		template<typename Descriptor>
		constexpr generic_ref_no_clone<Descriptor>::generic_ref_no_clone() noexcept {
		}

		template<typename Descriptor>
		class generic_ref : public generic_ref_no_clone<Descriptor> {
		public:
			using managed_t = typename generic_ref_no_clone<Descriptor>::managed_t;
			managed_t clone() const noexcept {
				return managed_t::take_own(Descriptor::clone(this->get()));
			}
		};

		template<typename Descriptor>
		class object_ref {
		public:
			using object_t = typename Descriptor::object_t;
			using managed_t = typename Descriptor::managed_t;

		protected:
			explicit constexpr object_ref() noexcept;
			object_ref(object_ref const& other) noexcept;
			object_ref(object_ref&& other) noexcept;
			/* not virtual */
			~object_ref() noexcept;
			object_ref& operator=(object_ref const& other) noexcept;
			object_ref& operator=(object_ref&& other) noexcept;

		public:
			static object_t* safe_cast(::nl_object* ptr) noexcept;
			static managed_t parse_msg(::nl_msg* msg, std::error_code& ec) noexcept;
			static managed_t parse_msg(::nl_msg* msg);

			object_t* get() const noexcept;
			::nl_object* get_object() const noexcept;

			/* implicit */ operator object_t*() const noexcept;
			/* implicit */ operator ::nl_object*() const noexcept;
			explicit operator bool() const noexcept;

			void reset() noexcept;

			/* take ownership of pointer */
			void set_own(object_t* ptr) noexcept;
			static managed_t take_own(object_t* ptr) noexcept;
			/* ptr is managed by caller, increment refcount */
			void set_inc_ref(object_t* ptr) noexcept;
			static managed_t take_inc_ref(object_t* ptr) noexcept;

			/* some api functions want pointers to pointers to store new objects in */
			object_t** reset_and_get_ptr_ref() noexcept;

			managed_t clone() const noexcept;

			QString toString() const noexcept;

		protected:
			object_t* m_ptr{nullptr};
		};

		template<typename Descriptor>
		constexpr object_ref<Descriptor>::object_ref() noexcept {
		}
	}
}

#endif /* _NUT_NETLINK_TEMPLATES_H */
