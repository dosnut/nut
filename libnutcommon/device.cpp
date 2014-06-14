#include "common.h"
#include "enum.h"

namespace libnutcommon {
	QDBusArgument& operator<<(QDBusArgument& argument, const OptionalQDBusObjectPath& optionalObjPath) {
		// send as string; can't send empty QDBusObjectPath
		argument.beginStructure();
		argument << static_cast<QString>(optionalObjPath.path());
		argument.endStructure();
		return argument;
	}

	const QDBusArgument& operator>>(const QDBusArgument& argument, OptionalQDBusObjectPath& optionalObjPath) {
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
	const QDBusArgument& operator>>(const QDBusArgument& argument, DeviceState& devstate) {
		return dbusUnserializeEnum(argument, devstate);
	}

	QString toString(DeviceType type) {
		return enumArrayEntry(type, { "eth", "air", "ppp" });
	}
	QDBusArgument& operator<<(QDBusArgument& argument, DeviceType type) {
		return dbusSerializeEnum(argument, type);
	}
	const QDBusArgument& operator>>(const QDBusArgument& argument, DeviceType& type) {
		return dbusUnserializeEnum(argument, type);
	}

	QString toString(InterfaceState state) {
		return enumArrayEntry(state, { "off", "static", "dhcp", "zeroconf", "waitforconfig" });
	}
	QDBusArgument& operator<<(QDBusArgument& argument, InterfaceState state) {
		return dbusSerializeEnum(argument, state);
	}
	const QDBusArgument& operator>>(const QDBusArgument& argument, InterfaceState& state) {
		return dbusUnserializeEnum(argument, state);
	}


	QDBusArgument &operator<<(QDBusArgument &argument, const DeviceProperties & devprop) {
		argument.beginStructure();
		argument
			<< devprop.name
			<< devprop.activeEnvironment
			<< devprop.state
			<< devprop.type;
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>>(const QDBusArgument &argument, DeviceProperties &devprop) {
		argument.beginStructure();
		argument
			>> devprop.name
			>> devprop.activeEnvironment
			>> devprop.state
			>> devprop.type;
		argument.endStructure();
		return argument;
	}

	QDBusArgument &operator<<(QDBusArgument &argument, const EnvironmentProperties &envprop) {
		argument.beginStructure();
		argument
			<< envprop.name
			<< envprop.active;
		argument.endStructure();
		return argument;
	}

	const QDBusArgument &operator>>(const QDBusArgument &argument, EnvironmentProperties &envprop) {
		argument.beginStructure();
		argument
			>> envprop.name
			>> envprop.active;
		return argument;
	}

	QDBusArgument &operator<<(QDBusArgument &argument, const InterfaceProperties &ifprop) {
		argument.beginStructure();
		argument
			<< ifprop.ifState
			<< ifprop.ip
			<< ifprop.netmask
			<< ifprop.gateway
			<< ifprop.dns;
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>>(const QDBusArgument &argument, InterfaceProperties &ifprop) {
		argument.beginStructure();
		argument
			>> ifprop.ifState
			>> ifprop.ip
			>> ifprop.netmask
			>> ifprop.gateway
			>> ifprop.dns;
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

		qDBusRegisterMetaType<OptionalQDBusObjectPath>();
		qDBusRegisterMetaType<DeviceState>();
		qDBusRegisterMetaType<DeviceType>();
		qDBusRegisterMetaType<InterfaceState>();
		qDBusRegisterMetaType<DeviceProperties>();
		qDBusRegisterMetaType<EnvironmentProperties>();
		qDBusRegisterMetaType<InterfaceProperties>();
	}
}
