/*
 * enum to flags helper
 *
 */

#ifndef NUT_COMMON_FLAGS_H
#define NUT_COMMON_FLAGS_H

#pragma once

#include <type_traits>

namespace libnutcommon {
	template<typename Enum>
	class Flags {
	public:
		typedef typename std::underlying_type<Enum>::type int_type;
		typedef Enum enum_type;

		inline Flags() : v(0) { }
		inline Flags(Enum e) : v(i(e)) { }
		inline explicit Flags(int_type v) : v(v) { }

		inline operator int_type () const { return v; }
		inline int_type int_value() const { return v; }
		inline operator bool () const { return 0 != v; }
		inline bool operator !() const { return !v; }

		inline bool testFlag(Enum e) const { return 0 != (v & i(e)); }

		inline Flags& operator|=(Flags a) { v |= a.v; return *this; }
		inline Flags& operator&=(Flags a) { v &= a.v; return *this; }
		inline Flags& operator^=(Flags a) { v ^= a.v; return *this; }
		friend inline bool operator==(Flags a, Flags b) { return a.v == b.v; }
		friend inline bool operator!=(Flags a, Flags b) { return a.v != b.v; }

		friend inline Flags operator|(Flags a, Flags b) { return Flags(a.v | b.v); }
		friend inline Flags operator&(Flags a, Flags b) { return Flags(a.v & b.v); }
		friend inline Flags operator^(Flags a, Flags b) { return Flags(a.v ^ b.v); }
	private:
		static inline int_type i(Enum e) {
			return static_cast< int_type >(e);
		}

		int_type v;
	};

#define NUTCOMMON_DECLARE_FLAG_OPERATORS(Flags) \
	inline Flags operator|(Flags::enum_type a, Flags::enum_type b) { \
		return Flags(a) | Flags(b); \
	} \
	inline Flags operator&(Flags::enum_type a, Flags::enum_type b) { \
		return Flags(a) & Flags(b); \
	} \
	inline Flags operator^(Flags::enum_type a, Flags::enum_type b) { \
		return Flags(a) ^ Flags(b); \
	}
}

#endif
