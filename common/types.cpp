#include "types.h"

namespace libnut {
	QDBusArgument &operator<< (QDBusArgument &argument, const SelectConfig & selconf) {
		argument.beginStructure();
		argument << selconf.selected << selconf.flags << selconf.useMac << selconf.macAddress.toString() << selconf.arpIP.toIPv4Address() << selconf.essid;
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, SelectConfig &selconf) {
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
	
	QDBusArgument &operator<< (QDBusArgument &argument, const WlanScanresult &scanres) {
		argument.beginStructure();
		argument << scanres.essid << scanres.channel << scanres.bssid << scanres.flags << scanres.signallevel << (int) scanres.encryption;
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, WlanScanresult &scanres) {
		int tmp;
		argument.beginStructure();
		argument >> scanres.essid >> scanres.channel >> scanres.bssid >> scanres.flags >> scanres.signallevel >> tmp;
		scanres.encryption = (WlanEncryptionType) tmp;
		argument.endStructure();
		return argument;
	}
	
	QDBusArgument &operator<< (QDBusArgument &argument, const WlanNetworkProperties &wlanprop) {
		argument.beginStructure();
		argument << wlanprop.scanresult << wlanprop.password << wlanprop.proto << wlanprop.key_mgmt;
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, WlanNetworkProperties &wlanprop) {
		argument.beginStructure();
		argument >> wlanprop.scanresult >> wlanprop.password >> wlanprop.proto >> wlanprop.key_mgmt;
		argument.endStructure();
		return argument;
	}
	
	QDBusArgument &operator<< (QDBusArgument &argument, const EnvironmentProperties &envprop) {
		argument.beginStructure();
		argument << envprop.name;
		argument.endStructure();
		return argument;
	};
	const QDBusArgument &operator>> (const QDBusArgument &argument, EnvironmentProperties &envprop) {
		argument.beginStructure();
		argument >> envprop.name;
		return argument;
	}
	QDBusArgument &operator<< (QDBusArgument &argument, const InterfaceProperties &ifprop) {
		argument.beginStructure();
		argument << ifprop.active << ifprop.userDefineable << ifprop.isStatic;
		argument << ifprop.ip.toIPv4Address() << ifprop.netmask.toIPv4Address() << ifprop.gateway.toIPv4Address();
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, InterfaceProperties &ifprop) {
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
		qRegisterMetaType<DeviceProperties>("DeviceProperties");
		qRegisterMetaType<SelectConfig>("SelectConfig");
		qRegisterMetaType<QList<SelectConfig> >("SelectConfigList");
		qRegisterMetaType<EnvironmentProperties>("EnvironmentProperties");
		qRegisterMetaType<InterfaceProperties>("InterfaceProperties");
//		qRegisterMetaType<QList<SelectConfig> >("SelectConfigList");
		qRegisterMetaType<WlanScanresult>("WlanScanresult");
		qRegisterMetaType<QList<libnut::WlanScanresult> >("WlanScanresultList");
//		qRegisterMetaType<QList<WlanScanresult> >("WlanScanresultList");
		qRegisterMetaType<WlanNetworkProperties>("WlanNetworkProperties");
	
		qDBusRegisterMetaType<DeviceProperties>();
		qDBusRegisterMetaType<SelectConfig>();
		qDBusRegisterMetaType<QList<SelectConfig> >();
		qDBusRegisterMetaType<EnvironmentProperties>();
		qDBusRegisterMetaType<InterfaceProperties>();
//		qDBusRegisterMetaType<QList<SelectConfig> >();
		qDBusRegisterMetaType<WlanScanresult>();
		qDBusRegisterMetaType<QList<libnut::WlanScanresult> >();
//		qDBusRegisterMetaType<QList<WlanScanresult> >();
		qDBusRegisterMetaType<WlanNetworkProperties>();
	}
}

namespace common {
	void init() { libnut::init(); }
}

