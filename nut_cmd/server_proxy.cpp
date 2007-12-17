#include "server_proxy.h"

namespace nut_cmd {
/*
 * Implementation of interface class DBusDeviceInterface
 */

DBusDeviceInterface::DBusDeviceInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

DBusDeviceInterface::~DBusDeviceInterface()
{
}



/*
 * Implementation of interface class DBusDeviceManagerInterface
 */

DBusDeviceManagerInterface::DBusDeviceManagerInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

DBusDeviceManagerInterface::~DBusDeviceManagerInterface()
{
}

//Methods






/*
 * Implementation of interface class DBusEnvironmentInterface
 */

DBusEnvironmentInterface::DBusEnvironmentInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

DBusEnvironmentInterface::~DBusEnvironmentInterface()
{
}
}

//Methods

