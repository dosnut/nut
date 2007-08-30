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
    
    QDBusArgument &operator<< (QDBusArgument &argument, const libnut_DeviceProperties & devprop) {
        argument.beginStructure();
        argument << devprop.name << devprop.activeEnvironment << devprop.enabled << devprop.type;
        argument.endStructure();
        return argument;
    }
    const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_DeviceProperties &devprop) {
        argument.beginStructure();
        argument >> devprop.name >> devprop.activeEnvironment >> devprop.enabled >> devprop.type;
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
        argument << envprop.active << envprop.name;
        argument.endStructure();
        return argument;
    };
    const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_EnvironmentProperties &envprop) {
        argument.beginStructure();
        argument >> envprop.active >> envprop.name;
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

	class libnut_types_init {
		public:
			libnut_types_init() {
				qRegisterMetaType<libnut::libnut_DeviceProperties>("libnut::libnut_DeviceProperties");
				qRegisterMetaType<libnut::libnut_SelectConfig>("libnut::libnut_SelectConfig");
				qRegisterMetaType<libnut::libnut_EnvironmentProperties>("libnut::libnut_EnvironmentProperties");
				qRegisterMetaType<libnut::libnut_InterfaceProperties>("libnut::libnut_InterfaceProperties");
				qRegisterMetaType<QList<libnut::libnut_SelectConfig> >("libnut::libnut_SelectConfigList");
				qRegisterMetaType<libnut::libnut_wlanScanresult>("libnut::libnut_wlanScanresult");
				qRegisterMetaType<QList<libnut::libnut_wlanScanresult> >("libnut::libnut_wlanScanresultList");
				qRegisterMetaType<libnut::libnut_wlanNetworkProperties>("libnut::libnut_wlanNetworkProperties");
			
				qDBusRegisterMetaType<libnut::libnut_DeviceProperties>();
				qDBusRegisterMetaType<libnut::libnut_SelectConfig>();
				qDBusRegisterMetaType<libnut::libnut_EnvironmentProperties>();
				qDBusRegisterMetaType<libnut::libnut_InterfaceProperties>();
				qDBusRegisterMetaType<QList<libnut::libnut_SelectConfig> >();
				qDBusRegisterMetaType<libnut::libnut_wlanScanresult>();
				qDBusRegisterMetaType<QList<libnut::libnut_wlanScanresult> >();
				qDBusRegisterMetaType<libnut::libnut_wlanNetworkProperties>();
			}
	};
	static libnut_types_init lt_init;
}

