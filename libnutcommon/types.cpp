#include "types.h"
#include "config.h"

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
}

QDBusArgument &operator<< (QDBusArgument &argument, const QHostAddress &data) {
	argument.beginStructure();
	argument << data.toString();
	argument.endStructure();
	return argument;
}
const QDBusArgument &operator>> (const QDBusArgument &argument, QHostAddress &data) {
	argument.beginStructure();
	QString addr;
	argument >> addr;
	data = QHostAddress(addr);
	argument.endStructure();
	return argument;
}

namespace libnutcommon {
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
	};
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


	void init() {
		static int done = 0;
		if (done) return;
		done = 1;
		
		qRegisterMetaType<QHostAddress>("QHostAddress");
		qRegisterMetaType<QList<QHostAddress> >("QList<QHostAddress>");
		qDBusRegisterMetaType<QHostAddress>();
		qDBusRegisterMetaType<QList<QHostAddress> >();
		
		qRegisterMetaType<DeviceProperties>("DeviceProperties");
		qRegisterMetaType<EnvironmentProperties>("EnvironmentProperties");
		qRegisterMetaType<InterfaceProperties>("InterfaceProperties");
		
		qDBusRegisterMetaType<DeviceProperties>();
		qDBusRegisterMetaType<EnvironmentProperties>();
		qDBusRegisterMetaType<InterfaceProperties>();
		
		qRegisterMetaType<Config>("libnutcommon::Config");
		qRegisterMetaType<DeviceConfig>("libnutcommon::DeviceConfig");
		qRegisterMetaType<SelectResult>("libnutcommon::SelectResult");
		qRegisterMetaType< QVector< SelectResult > >("QVector<libnutcommon::SelectRule>");
		qRegisterMetaType<SelectRule>("libnutcommon::SelectRule");
		qRegisterMetaType<SelectConfig>("libnutcommon::SelectConfig");
		qRegisterMetaType<EnvironmentConfig>("libnutcommon::EnvironmentConfig");
		qRegisterMetaType<IPv4Config>("libnutcommon::IPv4Config");
		qRegisterMetaType<IPv4UserConfig>("libnutcommon::IPv4UserConfig");
		qRegisterMetaType< QVector< quint32 > >("QVector< quint32 >");
		qRegisterMetaType< QVector< QVector< quint32 > > >("QVector< QVector< quint32 > >");
	
		qDBusRegisterMetaType<Config>();
		qDBusRegisterMetaType<DeviceConfig>();
		qDBusRegisterMetaType<SelectResult>();
		qDBusRegisterMetaType< QVector< SelectResult > >();
		qDBusRegisterMetaType<SelectRule>();
		qDBusRegisterMetaType<SelectConfig>();
		qDBusRegisterMetaType<EnvironmentConfig>();
		qDBusRegisterMetaType<IPv4Config>();
		qDBusRegisterMetaType<IPv4UserConfig>();
		qDBusRegisterMetaType< QVector< quint32 > >();
		qDBusRegisterMetaType< QVector< QVector< quint32 > > >();
	}
}
