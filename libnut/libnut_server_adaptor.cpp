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

bool DeviceAdaptor::disable()
{
    // handle method call NUT_DBUS_URL.Device.disable
    bool out0;
    QMetaObject::invokeMethod(parent(), "disable", Q_RETURN_ARG(bool, out0));
    return out0;
}

bool DeviceAdaptor::enable()
{
    // handle method call NUT_DBUS_URL.Device.enable
    bool out0;
    QMetaObject::invokeMethod(parent(), "enable", Q_RETURN_ARG(bool, out0));
    return out0;
}

QList<QDBusObjectPath> DeviceAdaptor::getEnvironments()
{
    // handle method call NUT_DBUS_URL.Device.getEnvironments
    QList<QDBusObjectPath> out0;
    QMetaObject::invokeMethod(parent(), "getEnvironments", Q_RETURN_ARG(QList<QDBusObjectPath>, out0));
    return out0;
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
libnut_SelectConfig EnvironmentAdaptor::getCurrentSelection() {
    libnut_SelectConfig out0;
    QMetaObject::invokeMethod(parent(), "getCurrentSelection", Q_RETURN_ARG(libnut_SelectConfig, out0));
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
}
