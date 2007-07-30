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
    uchar selconf_type;
    argument.beginStructure();
    argument >> selconf_type;
    selconf.type = (libnut_SelectType) selconf_type;
    argument >> selconf.useMAC >> selconf.essid >> hostaddress;
    selconf.arpIP = QHostAddress::QHostAddress(hostaddress);
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<< (QDBusArgument &argument, const libnut_DeviceProperties & devprop) {
    argument.beginStructure();
    argument << devprop.name << devprop.activeEnvironment << devprop.enabled;
    argument.endStructure();
    return argument;
}
const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_DeviceProperties &devprop) {
    argument.beginStructure();
    argument >> devprop.name >> devprop.activeEnvironment >> devprop.enabled;
    argument.endStructure();
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
    uchar selconf_type;
    quint32 hostaddress;
    argument.beginStructure();
    argument >> envprop.active >> envprop.name;
    argument >> selconf_type;
    envprop.currentSelection.type = (libnut_SelectType) selconf_type;
    argument >> envprop.currentSelection.useMAC;
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
};

Q_DECLARE_METATYPE(libnut::libnut_SelectConfig)
Q_DECLARE_METATYPE(libnut::libnut_DeviceProperties)
Q_DECLARE_METATYPE(libnut::libnut_EnvironmentProperties)
Q_DECLARE_METATYPE(libnut::libnut_InterfaceProperties)
