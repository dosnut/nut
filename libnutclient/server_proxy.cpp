#include "server_proxy.h"
#include "libnutclient/client.h"

namespace libnutclient {

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
void DBusDeviceManagerInterface::getDeviceList() {
	QList<QVariant> devListArgs;
	bool devListCall = callWithCallback(QLatin1String("getDeviceList"), devListArgs, this, SLOT(dbret_getDeviceList(QList< QDBusObjectPath >)), SLOT(dbret_errorOccured(QDBusError)) );
	if (!devListCall)
		emit queueErrorOccured();
}

void DBusDeviceManagerInterface::dbret_errorOccured(QDBusError error) {
	emit errorOccured(error);
}
void DBusDeviceManagerInterface::dbret_getDeviceList(QList<QDBusObjectPath> devices) {
	emit gotDeviceList(devices);
}



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

