#ifndef LIBNUTCLIENT_SERVER_PROXY_H
#define LIBNUTCLIENT_SERVER_PROXY_H

#include <QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>
#include <QMetaType>
#include "libnutcommon/common.h"

namespace nut_cmd {
/*
* Proxy class for interface DBus.Device
*/

class DBusDeviceInterface: public QDBusAbstractInterface {
	Q_OBJECT
public:
	static inline const char *staticInterfaceName()
	{ return NUT_DBUS_URL ".Device"; }

public:
	DBusDeviceInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

	~DBusDeviceInterface();

public Q_SLOTS: // METHODS
	inline QDBusReply<void> disable();
	inline QDBusReply<void> enable();
	inline QDBusReply<QList<QDBusObjectPath> > getEnvironments();
	inline QDBusReply<libnutcommon::DeviceProperties> getProperties();
	inline QDBusReply<QString> getEssid();
	inline QDBusReply<void> setEnvironment(QDBusObjectPath envpath);
	inline QDBusReply<void> setEnvironment(qint32 env);
};

/*
* Proxy class for interface DBus.DeviceManager
*/
class DBusDeviceManagerInterface: public QDBusAbstractInterface {
	Q_OBJECT
public:
	static inline const char *staticInterfaceName()
	{ return NUT_DBUS_URL ".DeviceManager"; }

public:
	DBusDeviceManagerInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

	~DBusDeviceManagerInterface();

public Q_SLOTS: // METHODS
	inline QDBusReply<QList<QDBusObjectPath> > getDeviceList();
};

/*
* Proxy class for interface DBus.Environment
*/
class DBusEnvironmentInterface: public QDBusAbstractInterface {
	Q_OBJECT
public:
	static inline const char *staticInterfaceName()
	{ return NUT_DBUS_URL ".Environment"; }

public:
	DBusEnvironmentInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

	~DBusEnvironmentInterface();

public Q_SLOTS: // METHODS
	inline QDBusReply<QList<QDBusObjectPath> > getInterfaces();
	inline QDBusReply<libnutcommon::EnvironmentProperties> getProperties();
};

/*
* Proxy class for interface DBus.Interface
*/
class DBusInterfaceInterface_IPv4: public QDBusAbstractInterface {
	Q_OBJECT
public:
	static inline const char *staticInterfaceName()
	{ return NUT_DBUS_URL ".Interface_IPv4"; }

public:
	DBusInterfaceInterface_IPv4(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

	~DBusInterfaceInterface_IPv4();

public Q_SLOTS: // METHODS
	inline QDBusReply<libnutcommon::InterfaceProperties> getProperties();
};


//DeviceInterface
inline QDBusReply<void> DBusDeviceInterface::disable() {
	QList<QVariant> argumentList;
	return callWithArgumentList(QDBus::NoBlock, QLatin1String("disable"), argumentList);
}
inline QDBusReply<void> DBusDeviceInterface::enable() {
	QList<QVariant> argumentList;
	return callWithArgumentList(QDBus::NoBlock, QLatin1String("enable"), argumentList);
}
inline QDBusReply<QList<QDBusObjectPath> > DBusDeviceInterface::getEnvironments() {
	QList<QVariant> argumentList;
	return callWithArgumentList(QDBus::Block, QLatin1String("getEnvironments"), argumentList);
}
inline QDBusReply<libnutcommon::DeviceProperties> DBusDeviceInterface::getProperties() {
	QList<QVariant> argumentList;
	return callWithArgumentList(QDBus::Block, QLatin1String("getProperties"), argumentList);
}
inline QDBusReply<QString> DBusDeviceInterface::getEssid() {
	QList<QVariant> argumentList;
	return callWithArgumentList(QDBus::Block, QLatin1String("getEssid"), argumentList);
}
inline QDBusReply<void> DBusDeviceInterface::setEnvironment(QDBusObjectPath envpath) {
	QList<QVariant> argumentList;
	argumentList << qVariantFromValue(envpath);
	return callWithArgumentList(QDBus::NoBlock, QLatin1String("setEnvironment"), argumentList);
}
inline QDBusReply<void> DBusDeviceInterface::setEnvironment(qint32 env) {
	QList<QVariant> argumentList;
	argumentList << qVariantFromValue(env);
	return callWithArgumentList(QDBus::NoBlock, QLatin1String("setEnvironment"), argumentList);
}

//DeviceManagerInterface
inline QDBusReply<QList<QDBusObjectPath> > DBusDeviceManagerInterface::getDeviceList() {
	QList<QVariant> argumentList;
	return callWithArgumentList(QDBus::Block, QLatin1String("getDeviceList"), argumentList);
}

//EnvironmentInterface
inline QDBusReply<QList<QDBusObjectPath> > DBusEnvironmentInterface::getInterfaces() {
	QList<QVariant> argumentList;
	return callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getInterfaces"), argumentList);
}
inline QDBusReply<libnutcommon::EnvironmentProperties> DBusEnvironmentInterface::getProperties() {
	QList<QVariant> argumentList;
	return callWithArgumentList(QDBus::Block, QLatin1String("getProperties"), argumentList);
}

//InterfaceInterface
inline QDBusReply<libnutcommon::InterfaceProperties> DBusInterfaceInterface_IPv4::getProperties() {
	QList<QVariant> argumentList;
	return callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getProperties"), argumentList);
}

}
#endif
