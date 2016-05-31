#include "common.h"
#include "enum.h"

/* DBus serializiation helpers for config types */

namespace libnutcommon {
	QDBusArgument& operator<<(QDBusArgument& argument, DeviceConfig const& data) {
		argument.beginStructure();
		argument
			<< data.noAutoStart
			<< data.wpaConfigFile
			<< data.wpaDriver
			<< data.gatewayMetric;
		argument.beginArray(qMetaTypeId<EnvironmentConfig>());
		for(auto const& ec: data.environments) {
			argument << *ec;
		}
		argument.endArray();
		argument.endStructure();
		return argument;
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, DeviceConfig& data) {
		argument.beginStructure();
		argument
			>> data.noAutoStart
			>> data.wpaConfigFile
			>> data.wpaDriver
			>> data.gatewayMetric;
		argument.beginArray();
		while (!argument.atEnd()) {
			auto ec = std::make_shared<EnvironmentConfig>();
			argument >> *ec;
			data.environments.push_back(std::move(ec));
		}
		argument.endArray();
		argument.endStructure();
		return argument;
	}

	QDBusArgument& operator<<(QDBusArgument& argument, SelectResult selectResult) {
		return dbusSerializeEnum(argument, selectResult);
	}
	const QDBusArgument& operator>>(const QDBusArgument& argument, SelectResult& selectResult) {
		return dbusUnserializeEnum(argument, selectResult);
	}

	QDBusArgument& operator<<(QDBusArgument& argument, SelectType selectType) {
		return dbusSerializeEnum(argument, selectType);
	}
	const QDBusArgument& operator>>(const QDBusArgument& argument, SelectType& selectType) {
		return dbusUnserializeEnum(argument, selectType);
	}

	QDBusArgument& operator<<(QDBusArgument& argument, SelectRule const& data) {
		argument.beginStructure();
		argument
			<< data.invert
			<< data.selType
			<< data.block
			<< data.essid
			<< data.ipAddr
			<< data.macAddr;
		argument.endStructure();
		return argument;
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, SelectRule& data) {
		argument.beginStructure();
		argument
			>> data.invert
			>> data.selType
			>> data.block
			>> data.essid
			>> data.ipAddr
			>> data.macAddr;
		argument.endStructure();
		return argument;
	}

	QDBusArgument& operator<<(QDBusArgument& argument, SelectConfig const& data) {
		argument.beginStructure();
		argument
			<< data.filters
			<< data.blocks;
		argument.endStructure();
		return argument;
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, SelectConfig& data) {
		argument.beginStructure();
		argument
			>> data.filters
			>> data.blocks;
		argument.endStructure();
		return argument;
	}

	QDBusArgument& operator<<(QDBusArgument& argument, EnvironmentConfig const& data) {
		argument.beginStructure();
		argument
			<< data.name
			<< data.select;
		argument.beginArray( qMetaTypeId<IPv4Config>() );
		for(auto const& ic: data.ipv4Interfaces) {
			argument << *ic;
		}
		argument.endArray();
		argument.endStructure();
		return argument;
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, EnvironmentConfig& data) {
		argument.beginStructure();
		argument
			>> data.name
			>> data.select;
		argument.beginArray();
		while (!argument.atEnd()) {
			auto ic = std::make_shared<IPv4Config>();
			argument >> *ic;
			data.ipv4Interfaces.push_back(std::move(ic));
		}
		argument.endArray();
		argument.endStructure();
		return argument;
	}

	QDBusArgument& operator<<(QDBusArgument& argument, IPv4ConfigFlags flags) {
		return dbusSerializeFlags(argument, flags);
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, IPv4ConfigFlags& flags) {
		return dbusUnserializeFlags(argument, flags);
	}

	QDBusArgument& operator<<(QDBusArgument& argument, IPv4Config const& data) {
		argument.beginStructure();
		argument
			<< data.flags
			<< data.static_ip
			<< data.static_netmask
			<< data.static_gateway
			<< data.static_dnsservers
			<< data.gatewayMetric;
		argument.endStructure();
		return argument;
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, IPv4Config& data) {
		argument.beginStructure();
		argument
			>> data.flags
			>> data.static_ip
			>> data.static_netmask
			>> data.static_gateway
			>> data.static_dnsservers
			>> data.gatewayMetric;
		argument.endStructure();
		return argument;
	}
}
