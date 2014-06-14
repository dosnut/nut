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
		emit queueErrorOccured("getDeviceList");
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
		emit queueErrorOccured("disable");
}

void DBusDeviceInterface::enable() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("enable"), argumentList, this, SLOT(dbret_dummy(QDBusMessage)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("enable");
}

void DBusDeviceInterface::setEnvironment(QDBusObjectPath envpath) {
	QList<QVariant> argumentList;
	argumentList << qVariantFromValue(envpath);
	bool worked = callWithCallback(QLatin1String("setEnvironment"), argumentList, this, SLOT(dbret_dummy(QDBusMessage)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("setEnvironment");
}

void DBusDeviceInterface::setEnvironment(qint32 env) {
	QList<QVariant> argumentList;
	argumentList << qVariantFromValue(env);
	bool worked = callWithCallback(QLatin1String("setEnvironment"), argumentList, this, SLOT(dbret_dummy(QDBusMessage)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("setEnvironment(int)");
}

void DBusDeviceInterface::getEnvironments() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("getEnvironments"), argumentList, this, SLOT(dbret_getEnvironments(QList<QDBusObjectPath>)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("getEnvironments");
}

void DBusDeviceInterface::getProperties() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("getProperties"), argumentList, this, SLOT(dbret_getProperties(libnutcommon::DeviceProperties)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("getProperties");
}
void DBusDeviceInterface::getEssid() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("getEssid"), argumentList, this, SLOT(dbret_getEssid(QString)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("getEssid");
}

void DBusDeviceInterface::getConfig() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("getConfig"), argumentList, this, SLOT(dbret_getConfig(libnutcommon::DeviceConfig)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("getConfig");
}


void DBusDeviceInterface::getActiveEnvironment() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("getActiveEnvironment"), argumentList, this, SLOT(dbret_getActiveEnvironment(libnutcommon::OptionalQDBusObjectPath)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("getActiveEnvironment");
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

void DBusEnvironmentInterface::getInterfaces() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("getInterfaces"), argumentList, this, SLOT(dbret_getInterfaces(QList<QDBusObjectPath>)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("getInterfaces");
}

void DBusEnvironmentInterface::getProperties() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("getProperties"), argumentList, this, SLOT(dbret_getProperties(libnutcommon::EnvironmentProperties)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("getProperties");
}

void DBusEnvironmentInterface::getConfig() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("getConfig"), argumentList, this, SLOT(dbret_getConfig(libnutcommon::EnvironmentConfig)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("getConfig");
}

void DBusEnvironmentInterface::getSelectResult() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("getSelectResult"), argumentList, this, SLOT(dbret_getSelectResult(libnutcommon::SelectResult)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("getSelectResult");
}

void DBusEnvironmentInterface::getSelectResults() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("getSelectResults"), argumentList, this, SLOT(dbret_getSelectResults(QVector<libnutcommon::SelectResult>)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("getSelectResults");
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

void DBusInterfaceInterface_IPv4::activate() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("activate"), argumentList, this, SLOT(dbret_dummy(QDBusMessage)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("activate");
}

void DBusInterfaceInterface_IPv4::deactivate() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("deactivate"), argumentList, this, SLOT(dbret_dummy(QDBusMessage)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("deactivate");
}

void DBusInterfaceInterface_IPv4::getProperties() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("getProperties"), argumentList, this, SLOT(dbret_getProperties(libnutcommon::InterfaceProperties)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("getProperties");
}


void DBusInterfaceInterface_IPv4::getConfig() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("getConfig"), argumentList, this, SLOT(dbret_getConfig(libnutcommon::IPv4Config)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("getConfig");
}
void DBusInterfaceInterface_IPv4::setDynamic() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("setDynamic"), argumentList, this, SLOT(dbret_dummy(QDBusMessage)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("setDynamic");
}

void DBusInterfaceInterface_IPv4::needUserSetup() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("needUserSetup"), argumentList, this, SLOT(dbret_getNeedUserSetup(bool)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("needUserSetup");
}

void DBusInterfaceInterface_IPv4::setUserConfig(libnutcommon::IPv4UserConfig userConfig) {
	QList<QVariant> argumentList;
	argumentList << qVariantFromValue(userConfig);
	bool worked = callWithCallback(QLatin1String("setUserConfig"), argumentList, this, SLOT(dbret_getSetUserConfig(bool)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("setUserConfig");
}

void DBusInterfaceInterface_IPv4::getUserConfig() {
	QList<QVariant> argumentList;
	bool worked = callWithCallback(QLatin1String("getUserConfig"), argumentList, this, SLOT(dbret_getUserConfig(libnutcommon::IPv4UserConfig)), SLOT(dbret_errorOccured(QDBusError)));
	if (!worked)
		emit queueErrorOccured("getUserConfig");
}


} //end namespace
