#include "common.h"

namespace libnutcommon {
	QString toString(enum DeviceState state) {
		const char* names[] = { "deactivated", "activated", "carrier", "unconfigured", "up" };
		return names[(int) state];
	}
	QString toString(enum DeviceType type) {
		const char* names[] = { "eth", "air", "ppp" };
		return names[(int) type];
	}
	QString toString(enum InterfaceState state) {
		const char* names[] = { "off", "static", "dhcp", "zeroconf", "waitforconfig" };
		return names[(int) state];
	}
	
	QDBusArgument &operator<< (QDBusArgument &argument, const DeviceProperties & devprop) {
		argument.beginStructure();
		argument << devprop.name << devprop.activeEnvironment << (int) devprop.state << (int) devprop.type;
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, DeviceProperties &devprop) {
		int tmp;
		argument.beginStructure();
		argument >> devprop.name >> devprop.activeEnvironment >> tmp;
		devprop.state = (DeviceState) tmp;
		argument >> tmp;
		devprop.type = (DeviceType) tmp;
		argument.endStructure();
		return argument;
	}
	
	QDBusArgument &operator<< (QDBusArgument &argument, const EnvironmentProperties &envprop) {
		argument.beginStructure();
		argument << envprop.name;
		argument << envprop.active;
		argument.endStructure();
		return argument;
	}

	const QDBusArgument &operator>> (const QDBusArgument &argument, EnvironmentProperties &envprop) {
		argument.beginStructure();
		argument >> envprop.name;
		argument >> envprop.active;
		return argument;
	}
	
	QDBusArgument &operator<< (QDBusArgument &argument, const InterfaceProperties &ifprop) {
		argument.beginStructure();
		argument << ifprop.ifState;
		argument << ifprop.ip << ifprop.netmask << ifprop.gateway << ifprop.dns;
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, InterfaceProperties &ifprop) {
		argument.beginStructure();
		int tmp;
		argument >> tmp;
		ifprop.ifState = (InterfaceState) tmp;
		argument >> ifprop.ip >> ifprop.netmask >> ifprop.gateway >> ifprop.dns;
		argument.endStructure();
		return argument;
	}

	// called by common.cpp: init()
	void device_init() {
		qRegisterMetaType<DeviceProperties>("DeviceProperties");
		qRegisterMetaType<EnvironmentProperties>("EnvironmentProperties");
		qRegisterMetaType<InterfaceProperties>("InterfaceProperties");
		
		qDBusRegisterMetaType<DeviceProperties>();
		qDBusRegisterMetaType<EnvironmentProperties>();
		qDBusRegisterMetaType<InterfaceProperties>();
	}
}
