#include "libnut_server_proxy.h"
/*
 * Implementation of interface class NUT_DBUS_URLDeviceInterface
 */

NUT_DBUS_URLDeviceInterface::NUT_DBUS_URLDeviceInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

NUT_DBUS_URLDeviceInterface::~NUT_DBUS_URLDeviceInterface()
{
}

/*
 * Implementation of interface class NUT_DBUS_URLDeviceManagerInterface
 */

NUT_DBUS_URLDeviceManagerInterface::NUT_DBUS_URLDeviceManagerInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

NUT_DBUS_URLDeviceManagerInterface::~NUT_DBUS_URLDeviceManagerInterface()
{
}

/*
 * Implementation of interface class NUT_DBUS_URLEnvironmentInterface
 */

NUT_DBUS_URLEnvironmentInterface::NUT_DBUS_URLEnvironmentInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

NUT_DBUS_URLEnvironmentInterface::~NUT_DBUS_URLEnvironmentInterface()
{
}

/*
 * Implementation of interface class NUT_DBUS_URLInterfaceInterface
 */

NUT_DBUS_URLInterfaceInterface::NUT_DBUS_URLInterfaceInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

NUT_DBUS_URLInterfaceInterface::~NUT_DBUS_URLInterfaceInterface()
{
}
