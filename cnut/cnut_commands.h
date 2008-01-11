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
	QStringList listEnvironmentNames(QDBusConnection * connection, QString &devPath);
	QStringList listEnvironmentNamesWithIndex(QDBusConnection * connection, QString &devPath);
	QStringList listInterfaceIndexes(QDBusConnection * connection, QString &envPath);
	QString getDeviceName(QDBusConnection * connection, QString &devPath);
	QString getDeviceType(QDBusConnection * connection, QString &devPath);
	QString getDeviceState(QDBusConnection * connection, QString &devPath);
	QString getActiveEnvironment(QDBusConnection * connection, QString &devPath);
	QString getEnvironmentName(QDBusConnection * connection, QString &envPath);
	QString getEnvironmentState(QDBusConnection * connection, QString &envPath);
	QString getEnvironmentSelectable(QDBusConnection * connection, QString &envPath);
	QString getInterfaceState(QDBusConnection * connection, QString &ifPath);
	QString getInterfaceType(QDBusConnection * connection, QString &ifPath);
	QStringList getInterfaceProperties(QDBusConnection * connection, QString &ifPath);
	
	//Set functions
	bool setEnvironment(QDBusConnection * connection, QString &devPath, int index);
	void setEnvironment(QDBusConnection * connection, QString &devPath, QString &envPath);
	void enableDevice(QDBusConnection * connection, QString &devPath);
	void disableDevice(QDBusConnection * connection, QString &devPath);

	//"Private functions"
	QString getDevicePathByName(QDBusConnection * connection, QString & name);
	QString getEnvironmentPathByName(QDBusConnection * connection, QString &devPath, QString &envName);
	QString getEnvironmentPathByIndex(QDBusConnection * connection, QString &devPath, qint32 index);
	QString getInterfacePathByIndex(QDBusConnection * connection, QString &envPath, qint32 index);
	libnutcommon::DeviceProperties getDeviceProperties(QDBusConnection * connection, QString &devPath);
	libnutcommon::EnvironmentProperties getEnvironmentProperties(QDBusConnection * connection, QString &env);
	libnutcommon::InterfaceProperties getRawInterfaceProperties(QDBusConnection * connection, QString &iface);
	void checkAccessRights(QDBusError error);
}
#endif
