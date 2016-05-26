#include "common.h"
#include "enum.h"

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

	QString toString(DeviceState state) {
		return enumArrayEntry(state, { "deactivated", "activated", "carrier", "unconfigured", "up" });
	}
	QDBusArgument& operator<<(QDBusArgument& argument, DeviceState devstate) {
		return dbusSerializeEnum(argument, devstate);
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, DeviceState& devstate) {
		return dbusUnserializeEnum(argument, devstate);
	}

	QString toString(DeviceType type) {
		return enumArrayEntry(type, { "eth", "air", "ppp" });
	}
	QDBusArgument& operator<<(QDBusArgument& argument, DeviceType type) {
		return dbusSerializeEnum(argument, type);
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, DeviceType& type) {
		return dbusUnserializeEnum(argument, type);
	}

	QString toString(InterfaceState state) {
		return enumArrayEntry(state, { "off", "static", "dhcp", "zeroconf", "waitforconfig" });
	}
	QDBusArgument& operator<<(QDBusArgument& argument, InterfaceState state) {
		return dbusSerializeEnum(argument, state);
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, InterfaceState& state) {
		return dbusUnserializeEnum(argument, state);
	}

	bool operator==(DeviceProperties const& a, DeviceProperties const& b) {
		return a.name              == b.name
			&& a.type              == b.type
			&& a.activeEnvironment == b.activeEnvironment
			&& a.state             == b.state
			&& a.essid             == b.essid
			&& a.macAddress        == b.macAddress;
	}
	bool operator!=(DeviceProperties const& a, DeviceProperties const& b) {
		return !(a == b);
	}

	QDBusArgument& operator<<(QDBusArgument& argument, const DeviceProperties& devprop) {
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

	bool operator==(EnvironmentProperties const& a, EnvironmentProperties const& b) {
		return a.name          == b.name
			&& a.id            == b.id
			&& a.active        == b.active
			&& a.selectResult  == b.selectResult
			&& a.selectResults == b.selectResults;
	}
	bool operator!=(EnvironmentProperties const& a, EnvironmentProperties const& b) {
		return !(a == b);
	}

	QDBusArgument& operator<<(QDBusArgument& argument, const EnvironmentProperties& envprop) {
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

	bool operator==(InterfaceProperties const& a, InterfaceProperties const& b) {
		return a.state         == b.state
			&& a.ip            == b.ip
			&& a.netmask       == b.netmask
			&& a.gateway       == b.gateway
			&& a.dnsServers    == b.dnsServers
			&& a.gatewayMetric == b.gatewayMetric
			&& a.needUserSetup == b.needUserSetup;
	}
	bool operator!=(InterfaceProperties const& a, InterfaceProperties const& b) {
		return !(a == b);
	}

	QDBusArgument& operator<<(QDBusArgument& argument, const InterfaceProperties& ifprop) {
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

	bool operator==(IPv4UserConfig const& a, IPv4UserConfig const& b) {
		return a.ip         == b.ip
			&& a.netmask    == b.netmask
			&& a.gateway    == b.gateway
			&& a.dnsServers == b.dnsServers;
	}
	bool operator!=(IPv4UserConfig const& a, IPv4UserConfig const& b) {
		return !(a == b);
	}

	QDBusArgument& operator<<(QDBusArgument& argument, const IPv4UserConfig& data) {
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

	// called by common.cpp: init()
	void device_init() {
		qRegisterMetaType<OptionalQDBusObjectPath>("libnutcommon::OptionalQDBusObjectPath");
		qRegisterMetaType<DeviceState>("libnutcommon::DeviceState");
		qRegisterMetaType<DeviceType>("libnutcommon::DeviceType");
		qRegisterMetaType<InterfaceState>("libnutcommon::InterfaceState");
		qRegisterMetaType<DeviceProperties>("libnutcommon::DeviceProperties");
		qRegisterMetaType<EnvironmentProperties>("libnutcommon::EnvironmentProperties");
		qRegisterMetaType<InterfaceProperties>("libnutcommon::InterfaceProperties");
		qRegisterMetaType<IPv4UserConfig>("libnutcommon::IPv4UserConfig");

		qDBusRegisterMetaType<OptionalQDBusObjectPath>();
		qDBusRegisterMetaType<DeviceState>();
		qDBusRegisterMetaType<DeviceType>();
		qDBusRegisterMetaType<InterfaceState>();
		qDBusRegisterMetaType<DeviceProperties>();
		qDBusRegisterMetaType<EnvironmentProperties>();
		qDBusRegisterMetaType<InterfaceProperties>();
		qDBusRegisterMetaType<IPv4UserConfig>();
	}
}
