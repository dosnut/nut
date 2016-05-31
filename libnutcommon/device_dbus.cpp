#include "common.h"
#include "enum.h"

/* DBus serializiation helpers for device.h types */

namespace libnutcommon {
	QDBusArgument& operator<<(QDBusArgument& argument, OptionalQDBusObjectPath const& optionalObjPath) {
		// send as string; can't send empty QDBusObjectPath
		argument.beginStructure();
		argument << static_cast<QString>(optionalObjPath.path());
		argument.endStructure();
		return argument;
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, OptionalQDBusObjectPath& optionalObjPath) {
		argument.beginStructure();
		QString tmp;
		argument >> tmp;
		optionalObjPath = OptionalQDBusObjectPath{tmp};
		argument.endStructure();
		return argument;
	}

	QDBusArgument& operator<<(QDBusArgument& argument, DeviceState devstate) {
		return dbusSerializeEnum(argument, devstate);
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, DeviceState& devstate) {
		return dbusUnserializeEnum(argument, devstate);
	}

	QDBusArgument& operator<<(QDBusArgument& argument, DeviceType type) {
		return dbusSerializeEnum(argument, type);
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, DeviceType& type) {
		return dbusUnserializeEnum(argument, type);
	}

	QDBusArgument& operator<<(QDBusArgument& argument, InterfaceState state) {
		return dbusSerializeEnum(argument, state);
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, InterfaceState& state) {
		return dbusUnserializeEnum(argument, state);
	}

	QDBusArgument& operator<<(QDBusArgument& argument, DeviceProperties const& devprop) {
		argument.beginStructure();
		argument
			<< devprop.name
			<< devprop.type
			<< devprop.activeEnvironment
			<< devprop.state
			<< devprop.essid
			<< devprop.macAddress;
		argument.endStructure();
		return argument;
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, DeviceProperties& devprop) {
		argument.beginStructure();
		argument
			>> devprop.name
			>> devprop.type
			>> devprop.activeEnvironment
			>> devprop.state
			>> devprop.essid
			>> devprop.macAddress;
		argument.endStructure();
		return argument;
	}

	QDBusArgument& operator<<(QDBusArgument& argument, EnvironmentProperties const& envprop) {
		argument.beginStructure();
		argument
			<< envprop.name
			<< envprop.id
			<< envprop.active
			<< envprop.selectResult
			<< envprop.selectResults;
		argument.endStructure();
		return argument;
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, EnvironmentProperties& envprop) {
		argument.beginStructure();
		argument
			>> envprop.name
			>> envprop.id
			>> envprop.active
			>> envprop.selectResult
			>> envprop.selectResults;
		return argument;
	}

	QDBusArgument& operator<<(QDBusArgument& argument, InterfaceProperties const& ifprop) {
		argument.beginStructure();
		argument
			<< ifprop.state
			<< ifprop.ip
			<< ifprop.netmask
			<< ifprop.gateway
			<< ifprop.dnsServers
			<< ifprop.gatewayMetric
			<< ifprop.needUserSetup;
		argument.endStructure();
		return argument;
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, InterfaceProperties& ifprop) {
		argument.beginStructure();
		argument
			>> ifprop.state
			>> ifprop.ip
			>> ifprop.netmask
			>> ifprop.gateway
			>> ifprop.dnsServers
			>> ifprop.gatewayMetric
			>> ifprop.needUserSetup;
		argument.endStructure();
		return argument;
	}

	QDBusArgument& operator<<(QDBusArgument& argument, IPv4UserConfig const& data) {
		argument.beginStructure();
		argument
			<< data.ip
			<< data.netmask
			<< data.gateway
			<< data.dnsServers;
		argument.endStructure();
		return argument;
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, IPv4UserConfig& data) {
		argument.beginStructure();
		argument
			>> data.ip
			>> data.netmask
			>> data.gateway
			>> data.dnsServers;
		argument.endStructure();
		return argument;
	}
}
