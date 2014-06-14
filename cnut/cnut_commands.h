#ifndef NUT_CMD_COMMANDS_H
#define NUT_CMD_COMMANDS_H
#include "server_proxy.h"
#include "cnut_types.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusObjectPath>
#include <QObject>
#include <iostream>
namespace cnut {


	void print(QStringList list);
	void print(QString str);

	//"Public functions"

	//Get functions
	QStringList listDeviceNames(QDBusConnection * connection);
	QStringList listEnvironmentNames(QDBusConnection * connection, QString const& devPath);
	QStringList listEnvironmentNamesWithIndex(QDBusConnection * connection, QString const& devPath);
	QStringList listInterfaceIndexes(QDBusConnection * connection, QString const& envPath);
	QString getDeviceName(QDBusConnection * connection, QString const& devPath);
	QString getDeviceType(QDBusConnection * connection, QString const& devPath);
	QString getDeviceState(QDBusConnection * connection, QString const& devPath);
	QString getActiveEnvironment(QDBusConnection * connection, QString const& devPath);
	QString getEnvironmentName(QDBusConnection * connection, QString const& envPath);
	QString getEnvironmentState(QDBusConnection * connection, QString const& envPath);
	QString getEnvironmentSelectable(QDBusConnection * connection, QString const& envPath);
	QString getInterfaceState(QDBusConnection * connection, QString const& ifPath);
	QString getInterfaceType(QDBusConnection * connection, QString const& ifPath);
	QStringList getInterfaceProperties(QDBusConnection * connection, QString const& ifPath);

	//Set functions
	bool setEnvironment(QDBusConnection * connection, QString const& devPath, int index);
	void setEnvironment(QDBusConnection * connection, QString const& devPath, QString const& envPath);
	void enableDevice(QDBusConnection * connection, QString const& devPath);
	void disableDevice(QDBusConnection * connection, QString const& devPath);

	//"Private functions"
	QString getDevicePathByName(QDBusConnection * connection, QString const&  name);
	QString getEnvironmentPathByName(QDBusConnection * connection, QString const& devPath, QString const& envName);
	QString getEnvironmentPathByIndex(QDBusConnection * connection, QString const& devPath, qint32 index);
	QString getInterfacePathByIndex(QDBusConnection * connection, QString const& envPath, qint32 index);
	libnutcommon::DeviceProperties getDeviceProperties(QDBusConnection * connection, QString const& devPath);
	libnutcommon::EnvironmentProperties getEnvironmentProperties(QDBusConnection * connection, QString const& env);
	libnutcommon::InterfaceProperties getRawInterfaceProperties(QDBusConnection * connection, QString const& iface);
	void checkAccessRights(QDBusError error);
}
#endif
