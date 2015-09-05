#ifndef NUT_CMD_COMMANDS_H
#define NUT_CMD_COMMANDS_H

#include "cnut_types.h"

namespace cnut {
	//"Public functions"

	using libnutcommon::OptionalQDBusObjectPath;

	//Get functions
	QStringList listDeviceNames(QDBusConnection& connection);
	QStringList listEnvironmentNames(QDBusConnection& connection, QDBusObjectPath const& devPath, bool withIndex = false);
	QStringList listInterfaceIndexes(QDBusConnection& connection, QDBusObjectPath const& envPath);
	QString getDeviceType(QDBusConnection& connection, QDBusObjectPath const& devPath);
	QString getDeviceState(QDBusConnection& connection, QDBusObjectPath const& devPath);
	QString getActiveEnvironment(QDBusConnection& connection, QDBusObjectPath const& devPath);
	QString getEnvironmentActive(QDBusConnection& connection, QDBusObjectPath const& envPath);
	QString getEnvironmentSelectable(QDBusConnection& connection, QDBusObjectPath const& envPath);
	QString getInterfaceState(QDBusConnection& connection, QDBusObjectPath const& ifPath);
	QStringList getInterfaceProperties(QDBusConnection& connection, QDBusObjectPath const& ifPath);

	//Set functions
	void setEnvironment(QDBusConnection& connection, QDBusObjectPath const& devPath, qint32 index);
	void setEnvironment(QDBusConnection& connection, QDBusObjectPath const& devPath, QDBusObjectPath const& envPath);
	void enableDevice(QDBusConnection& connection, QDBusObjectPath const& devPath);
	void disableDevice(QDBusConnection& connection, QDBusObjectPath const& devPath);

	//helper functions
	OptionalQDBusObjectPath getDevicePathByName(QDBusConnection& connection, QString const& name);
	OptionalQDBusObjectPath getEnvironmentPathByName(QDBusConnection& connection, QDBusObjectPath const& devPath, QString const& envName);
	OptionalQDBusObjectPath getEnvironmentPathByIndex(QDBusConnection& connection, QDBusObjectPath const& devPath, qint32 index);
	OptionalQDBusObjectPath getInterfacePathByIndex(QDBusConnection& connection, QDBusObjectPath const& envPath, qint32 index);
}

#endif
