#ifndef NUT_CMD_COMMANDS_H
#define NUT_CMD_COMMANDS_H
#include "../libnutclient/server_proxy.h"
#include "nut_cmd_types.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusObjectPath>
#include <QObject>
namespace nut_cmd {

	//"Public functions"

	//Get functions
	QStringList listDeviceNames(QDBusConnection * connection);
	QStringList listEnvironmentNames(QDBusConnection * connection, QString &devPath);
	QString getDeviceName(QDBusConnection * connection, QString &devPath);
	QString getDeviceType(QDBusConnection * connection, QString &devPath);
	QString getDeviceState(QDBusConnection * connection, QString &devPath);
	QString getActiveEnvironment(QDBusConnection * connection, QString &devPath);
	QString getEnvironmentName(QDBusConnection * connection, QString &envPath);
	QString getEnvironmentState(QDBusConnection * connection, QString &envPath);
	
	//Set functions
	bool setEnvironment(QDBusConnection * connection, QString &devPath, int index);
	void setEnvironment(QDBusConnection * connection, QString &devPath, QString &envPath);
	void enableDevice(QDBusConnection * connection, QString &devPath);
	void disableDevice(QDBusConnection * connection, QString &devPath);

	//"Private functions"
	QString getDevicePathByName(QDBusConnection * connection, QString & name);
	QString getEnvironmentPathByName(QDBusConnection * connection, QString &devPath, QString &envName);
	DeviceProperties getDeviceProperties(QDBusConnection * connection, QDBusObjectPath &dev);
	libnutcommon::EnvironmentProperties getEnvironmentProperties(QDBusConnection * connection, QString &env);
	checkAccessRights(QDBusError error);
}
#endif