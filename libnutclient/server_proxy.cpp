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


void DBusDeviceInterface::disable() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("disable"), argumentList, this, SLOT(dbret_dummy(QDBusMessage)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured();
}

void DBusDeviceInterface::enable() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("enable"), argumentList, this, SLOT(dbret_dummy(QDBusMessage)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured();
}

void DBusDeviceInterface::setEnvironment(QDBusObjectPath envpath) {
	QList<QVariant> argumentList;
	argumentList << qVariantFromValue(envpath);
	bool worked = callWithCallback(QLatin1String("setEnvironment"), argumentList, this, SLOT(dbret_dummy(QDBusMessage)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured();
}

void DBusDeviceInterface::setEnvironment(qint32 env) {
	QList<QVariant> argumentList;
	argumentList << qVariantFromValue(env);
	bool worked = callWithCallback(QLatin1String("setEnvironment"), argumentList, this, SLOT(dbret_dummy(QDBusMessage)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured();
}

void DBusDeviceInterface::getEnvironments() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("getEnvironment"), argumentList, this, SLOT(dbret_getEnvironments(QList<QDBusObjectPath>)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured();
}

void DBusDeviceInterface::getProperties() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("getProperties"), argumentList, this, SLOT(dbret_getProperties(libnutcommon::DeviceProperties props)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured();
}
void DBusDeviceInterface::getEssid() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("getEssid"), argumentList, this, SLOT(dbret_getEssid(QString)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured();
}

void DBusDeviceInterface::getConfig() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("getConfig"), argumentList, this, SLOT(dbret_getConfig(libnutcommon::DeviceConfig)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured();
}


void DBusDeviceInterface::getActiveEnvironment() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("getActiveEnvironment"), argumentList, this, SLOT(dbret_getActiveEnvironment(QString), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured();
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

