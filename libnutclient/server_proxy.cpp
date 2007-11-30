#include "server_proxy.h"
#include "libnutclient/client.h"

namespace libnutclient {
/*
 * Implementation of interface class DBusDeviceInterface
 */

DBusDeviceInterface::DBusDeviceInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
	m_device = static_cast<CDevice*>(parent);
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
	m_device = static_cast<CDevice*>(parent->parent());
}

DBusEnvironmentInterface::~DBusEnvironmentInterface()
{
}





/*
 * Implementation of interface class DBusInterfaceInterface_IPv4
 */

DBusInterfaceInterface_IPv4::DBusInterfaceInterface_IPv4(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
	m_device = static_cast<CDevice*>((parent->parent())->parent());
}

DBusInterfaceInterface_IPv4::~DBusInterfaceInterface_IPv4()
{
}
}

//Methods

