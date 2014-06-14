/*
 * common enum helpers
 *
 */

#ifndef NUT_COMMON_ENUM_H
#define NUT_COMMON_ENUM_H

#include <QDBusArgument>
#include <type_traits>

namespace libnutcommon {
	template<typename Enum, typename T>
	T enumArrayEntry(Enum value, std::initializer_list<T> const& arr) {
		auto ndx = static_cast< typename std::underlying_type<Enum>::type >(value);
		if (ndx < 0 || ndx >= arr.size()) return { };
		return begin(arr)[ndx];
	}

	/* helpers to serialize/unserialize enum value for QDBus */
	template<typename Enum>
	void dbusArgWriteEnum(QDBusArgument& argument, Enum value) {
		argument << static_cast< typename std::underlying_type<Enum>::type >(value);
	}

	template<typename Enum>
	void dbusArgReadEnum(QDBusArgument const& argument, Enum &value) {
		typename std::underlying_type<Enum>::type tmp;
		argument >> tmp;
		value = static_cast<Enum>(tmp);
	}

	template<typename Enum>
	QDBusArgument& dbusSerializeEnum(QDBusArgument& argument, Enum value) {
		argument.beginStructure();
		dbusArgWriteEnum(argument, value);
		argument.endStructure();
		return argument;
	}

	template<typename Enum>
	QDBusArgument const& dbusUnserializeEnum(QDBusArgument const& argument, Enum &value) {
		argument.beginStructure();
		dbusArgReadEnum(argument, value);
		argument.endStructure();
		return argument;
	}

	/* helpers to serialize/unserialize libnutcommon::Flags<enum> value for QDBus;
	 * required `Flags` interface:
	 * - Flags::int_type Flags::int_value()
	 * - Flags::Flags(Flags::int_type v)
	 */
	template<typename Flags>
	void dbusArgWriteFlags(QDBusArgument& argument, Flags value) {
		argument << value.int_value();
	}

	template<typename Flags>
	void dbusArgReadFlags(QDBusArgument const& argument, Flags &value) {
		typename Flags::int_type tmp;
		argument >> tmp;
		value = Flags(tmp);
	}

	template<typename Flags>
	QDBusArgument& dbusSerializeFlags(QDBusArgument& argument, Flags value) {
		argument.beginStructure();
		dbusArgWriteFlags(argument, value);
		argument.endStructure();
		return argument;
	}

	template<typename Flags>
	QDBusArgument const& dbusUnserializeFlags(QDBusArgument const& argument, Flags &value) {
		argument.beginStructure();
		dbusArgReadFlags(argument, value);
		argument.endStructure();
		return argument;
	}
}

#endif
