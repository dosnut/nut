#include "types.h"

namespace libnut {
	QDBusArgument &operator<< (QDBusArgument &argument, const libnut_SelectConfig & selconf) {
		argument.beginStructure();
		argument << selconf.selected << selconf.flags << selconf.useMac << selconf.macAddress.toString() << selconf.arpIP.toIPv4Address() << selconf.essid;
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_SelectConfig &selconf) {
		argument.beginStructure();
		QString mac;
		quint32 ip;
		argument >> selconf.selected >> selconf.flags >> selconf.useMac;
		argument >> mac;
		selconf.macAddress = nut::MacAddress(mac);
		argument >> ip;
		selconf.arpIP = QHostAddress(ip);
		argument >> selconf.essid;
		argument.endStructure();
		return argument;
	}
	
	QDBusArgument &operator<< (QDBusArgument &argument, const DeviceState &devstate) {
		argument << (int) devstate;
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, DeviceState &devstate) {
		int state;
		argument >> state;
		devstate = (DeviceState) state;
		return argument;
	}
	QDBusArgument &operator<< (QDBusArgument &argument, const DeviceType &devtype) {
		argument << (int) devtype;
	}
    const QDBusArgument &operator>> (const QDBusArgument &argument, DeviceState &devtype) {
		int type;
		argument >> type;
		devtype = (DeviceType) type;
		return argument;
    }

	QDBusArgument &operator<< (QDBusArgument &argument, const libnut_DeviceProperties & devprop) {
		argument.beginStructure();
		argument << devprop.name << devprop.activeEnvironment << devprop.state << devprop.type;
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_DeviceProperties &devprop) {
		argument.beginStructure();
		argument >> devprop.name >> devprop.activeEnvironment >> devprop.state >> devprop.type;
		argument.endStructure();
		return argument;
	}
	
	QDBusArgument &operator<< (QDBusArgument &argument, const libnut_wlanScanresult &scanres) {
		argument.beginStructure();
		argument << scanres.essid << scanres.channel << scanres.bssid << scanres.flags << scanres.signallevel << scanres.encryption;
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_wlanScanresult &scanres) {
		argument.beginStructure();
		argument >> scanres.essid >> scanres.channel >> scanres.bssid >> scanres.flags >> scanres.signallevel >> scanres.encryption;
		argument.endStructure();
		return argument;
	}
	
	QDBusArgument &operator<< (QDBusArgument &argument, const libnut_wlanNetworkProperties &wlanprop) {
		argument.beginStructure();
		argument << wlanprop.scanresult << wlanprop.password << wlanprop.proto << wlanprop.key_mgmt;
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_wlanNetworkProperties &wlanprop) {
		argument.beginStructure();
		argument >> wlanprop.scanresult >> wlanprop.password >> wlanprop.proto >> wlanprop.key_mgmt;
		argument.endStructure();
		return argument;
	}
	
	QDBusArgument &operator<< (QDBusArgument &argument, const libnut_EnvironmentProperties &envprop) {
		argument.beginStructure();
		argument << envprop.name;
		argument.endStructure();
		return argument;
	};
	const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_EnvironmentProperties &envprop) {
		argument.beginStructure();
		argument >> envprop.name;
		return argument;
	}
	QDBusArgument &operator<< (QDBusArgument &argument, const libnut_InterfaceProperties &ifprop) {
		argument.beginStructure();
		argument << ifprop.active << ifprop.userDefineable << ifprop.isStatic;
		argument << ifprop.ip.toIPv4Address() << ifprop.netmask.toIPv4Address() << ifprop.gateway.toIPv4Address();
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_InterfaceProperties &ifprop) {
		quint32 hostaddress;
		argument.beginStructure();
		argument >> ifprop.active >> ifprop.userDefineable >> ifprop.isStatic;
		argument >> hostaddress;
		ifprop.ip = QHostAddress::QHostAddress(hostaddress);
		argument >> hostaddress;
		ifprop.netmask = QHostAddress::QHostAddress(hostaddress);
		argument >> hostaddress;
		ifprop.gateway = QHostAddress::QHostAddress(hostaddress);
		argument.endStructure();
		return argument;
	}
	
	static void init() {
		static int done = 0;
		if (done) return;
		done = 1;
		qRegisterMetaType<libnut_DeviceProperties>("libnut_DeviceProperties");
		qRegisterMetaType<DeviceState>("DeviceState");
		qRegisterMetaType<libnut_SelectConfig>("libnut_SelectConfig");
		qRegisterMetaType<libnut_EnvironmentProperties>("libnut_EnvironmentProperties");
		qRegisterMetaType<libnut_InterfaceProperties>("libnut_InterfaceProperties");
//		qRegisterMetaType<QList<libnut_SelectConfig> >("libnut_SelectConfigList");
		qRegisterMetaType<libnut_wlanScanresult>("libnut_wlanScanresult");
//		qRegisterMetaType<QList<libnut_wlanScanresult> >("libnut_wlanScanresultList");
		qRegisterMetaType<libnut_wlanNetworkProperties>("libnut_wlanNetworkProperties");
	
		qDBusRegisterMetaType<libnut_DeviceProperties>();
		qDBusRegisterMetaType<DeviceState>();
		qDBusRegisterMetaType<libnut_SelectConfig>();
		qDBusRegisterMetaType<libnut_EnvironmentProperties>();
		qDBusRegisterMetaType<libnut_InterfaceProperties>();
//		qDBusRegisterMetaType<QList<libnut_SelectConfig> >();
		qDBusRegisterMetaType<libnut_wlanScanresult>();
//		qDBusRegisterMetaType<QList<libnut_wlanScanresult> >();
		qDBusRegisterMetaType<libnut_wlanNetworkProperties>();
	}
}

namespace common {
	void init() { libnut::init(); }
}

