#include "libnut_server_adaptor.h"
namespace libnut {
/*
 * Implementation of adaptor class DeviceAdaptor
 */

DeviceAdaptor::DeviceAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

DeviceAdaptor::~DeviceAdaptor()
{
    // destructor
}

void DeviceAdaptor::disable()
{
    // handle method call NUT_DBUS_URL.Device.disable
    QMetaObject::invokeMethod(parent(), "disable");
}

void DeviceAdaptor::enable()
{
    // handle method call NUT_DBUS_URL.Device.enable
    QMetaObject::invokeMethod(parent(), "enable");
}

QList<QDBusObjectPath> DeviceAdaptor::getEnvironments()
{
    // handle method call NUT_DBUS_URL.Device.getEnvironments
    QList<QDBusObjectPath> out0;
    QMetaObject::invokeMethod(parent(), "getEnvironments", Q_RETURN_ARG(QList<QDBusObjectPath>, out0));
    return out0;
}
QList<libnut_wlanScanresult> DeviceAdaptor::getwlanScan() {
    QList<libnut_wlanScanresult> out0;
    QMetaObject::invokeMethod(parent(), "getwlanScan", Q_RETURN_ARG(QList<libnut_wlanScanresult>, out0));
    return out0;
}
void DeviceAdaptor::addwlanEnvironment(libnut_wlanNetworkProperties netprops) {
    QMetaObject::invokeMethod(parent(), "addwlanEnvironment", Q_ARG(libnut_wlanNetworkProperties, netprops));
}
void DeviceAdaptor::addEnvironment(libnut_EnvironmentProperties envprops) {
    QMetaObject::invokeMethod(parent(), "addEnvironment", Q_ARG(libnut_EnvironmentProperties, envprops));
}
libnut_DeviceProperties DeviceAdaptor::getProperties() {
    libnut_DeviceProperties out0;
    QMetaObject::invokeMethod(parent(), "getProperties", Q_RETURN_ARG(libnut_DeviceProperties, out0));
    return out0;
}
void DeviceAdaptor::setEnvironment(QDBusObjectPath envpath) {
    QMetaObject::invokeMethod(parent(), "setEnvironment", Q_ARG(QDBusObjectPath, envpath));
}
/*
 * Implementation of adaptor class DeviceManagerAdaptor
 */

DeviceManagerAdaptor::DeviceManagerAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

DeviceManagerAdaptor::~DeviceManagerAdaptor()
{
    // destructor
}

QList<QDBusObjectPath> DeviceManagerAdaptor::getDeviceList()
{
    // handle method call NUT_DBUS_URL.DeviceManager.getDeviceList
    QList<QDBusObjectPath> out0;
    QMetaObject::invokeMethod(parent(), "getDeviceList", Q_RETURN_ARG(QList<QDBusObjectPath>, out0));
    return out0;
}

/*
 * Implementation of adaptor class EnvironmentAdaptor
 */

EnvironmentAdaptor::EnvironmentAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

EnvironmentAdaptor::~EnvironmentAdaptor()
{
    // destructor
}
QList<libnut_SelectConfig> EnvironmentAdaptor::getSelectConfig() {
    QList<libnut_SelectConfig> out0;
    QMetaObject::invokeMethod(parent(), "getSelectConfig", Q_RETURN_ARG(QList<libnut_SelectConfig>, out0));
    return out0;
}
libnut_EnvironmentProperties EnvironmentAdaptor::getProperties() {
    libnut_EnvironmentProperties out0;
    QMetaObject::invokeMethod(parent(), "getProperties", Q_RETURN_ARG(libnut_EnvironmentProperties, out0));
    return out0;
}


QList<QDBusObjectPath> EnvironmentAdaptor::getInterfaces()
{
    // handle method call NUT_DBUS_URL.Environment.getInterfaces
    QList<QDBusObjectPath> out0;
    QMetaObject::invokeMethod(parent(), "getInterfaces", Q_RETURN_ARG(QList<QDBusObjectPath>, out0));
    return out0;
}
void EnvironmentAdaptor::addInterface(libnut_InterfaceProperties prop) {
    QMetaObject::invokeMethod(parent(), "addInterface", Q_ARG(libnut_InterfaceProperties, prop));
}
void EnvironmentAdaptor::removeInterface(QDBusObjectPath path) {
    QMetaObject::invokeMethod(parent(), "removeInterface", Q_ARG(QDBusObjectPath, path));
}

/*
 * Implementation of adaptor class InterfaceAdaptor
 */

InterfaceAdaptor::InterfaceAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

InterfaceAdaptor::~InterfaceAdaptor()
{
    // destructor
}

void InterfaceAdaptor::activate()
{
    // handle method call NUT_DBUS_URL.Interface.activate
    QMetaObject::invokeMethod(parent(), "activate");
}

void InterfaceAdaptor::deactivate()
{
    // handle method call NUT_DBUS_URL.Interface.deactivate
    QMetaObject::invokeMethod(parent(), "deactivate");
}

void InterfaceAdaptor::setGateway(uint Gateway)
{
    // handle method call NUT_DBUS_URL.Interface.setGateway
    QMetaObject::invokeMethod(parent(), "setGateway", Q_ARG(uint, Gateway));
}

void InterfaceAdaptor::setIP(uint HostAddress)
{
    // handle method call NUT_DBUS_URL.Interface.setGateway
    QMetaObject::invokeMethod(parent(), "setIP", Q_ARG(uint, HostAddress));
}

void InterfaceAdaptor::setNetmask(uint Netmask)
{
    // handle method call NUT_DBUS_URL.Interface.setNetmask
    QMetaObject::invokeMethod(parent(), "setNetmask", Q_ARG(uint, Netmask));
}

void InterfaceAdaptor::setDynamic()
{
    // handle method call NUT_DBUS_URL.Interface.setNetmask
    QMetaObject::invokeMethod(parent(), "setDynamic");
}
}
