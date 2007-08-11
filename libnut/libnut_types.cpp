#include "libnut_types.h"

namespace libnut {

QDBusArgument &operator<< (QDBusArgument &argument, const libnut_SelectConfig & selconf) {
    argument.beginStructure();
    argument << selconf.type << selconf.useMAC << selconf.essid << selconf.arpIP.toIPv4Address();
    argument.endStructure();
    return argument;
}
const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_SelectConfig &selconf) {
    quint32 hostaddress;
    argument.beginStructure();
    argument >> selconf.type >> selconf.useMAC >> selconf.essid >> hostaddress;
    selconf.arpIP = QHostAddress::QHostAddress(hostaddress);
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
    endStructure();
    return argument;
}
const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_wlanNetworkProperties &wlanprop) {
    argument.beginStructure();
    argument >> wlanprop.scanresult >> wlanprop.password >> wlanprop.proto >> wlanprop.key_mgmt;
    endStructure();
    return argument;
}

QDBusArgument &operator<< (QDBusArgument &argument, const libnut_EnvironmentProperties &envprop) {
    argument.beginStructure();
    argument << envprop.active << envprop.name;
    argument << envprop.currentSelection.type << envprop.currentSelection.useMAC;
    argument << envprop.currentSelection.arpIP.toIPv4Address() << envprop.currentSelection.essid;
    argument.endStructure();
    return argument;
};
const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_EnvironmentProperties &envprop) {
    quint32 hostaddress;
    argument.beginStructure();
    argument >> envprop.active >> envprop.name;
    argument >> envprop.currentSelection.type >> envprop.currentSelection.useMAC;
    argument >> hostaddress;
    envprop.currentSelection.arpIP = QHostAddress::QHostAddress(hostaddress);
    argument >> envprop.currentSelection.essid;
    argument.endStructure();
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
}
int metatype_id_libnut_DeviceProperties = qRegisterMetaType<libnut::libnut_DeviceProperties>("libnut_DeviceProperties");
int metatype_id_libnut_SelectConfig = qRegisterMetaType<libnut::libnut_SelectConfig>("libnut_SelectConfig");
int metatype_id_libnut_EnvironmentProperties = qRegisterMetaType<libnut::libnut_EnvironmentProperties>("libnut_EnvironmentProperties");
int metatype_id_libnut_InterfaceProperties = qRegisterMetaType<libnut::libnut_InterfaceProperties>("libnut_InterfaceProperties");
int id = qMetaTypeId<libnut::libnut_SelectConfig>();
