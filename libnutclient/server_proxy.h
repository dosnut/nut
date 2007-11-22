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

namespace libnutclient {
/*
* Proxy class for interface DBus.Device
*/
class CDevice;

class DBusDeviceInterface: public QDBusAbstractInterface {
	Q_OBJECT
public:
	static inline const char *staticInterfaceName()
	{ return NUT_DBUS_URL ".Device"; }
	CDevice * device;

public:
	DBusDeviceInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

	~DBusDeviceInterface();

public Q_SLOTS: // METHODS
	inline QDBusReply<void> disable();
	inline QDBusReply<void> enable();
	inline QDBusReply<QList<QDBusObjectPath> > getEnvironments();
	inline QDBusReply<libnutcommon::DeviceProperties> getProperties();
	inline QDBusReply<QString> getEssid();
	inline QDBusReply<libnutcommon::DeviceConfig> getConfig();
	inline QDBusReply<void> setEnvironment(QDBusObjectPath envpath);

Q_SIGNALS: // SIGNALS
	void environmentChangedActive(const QString &newenv);
	void environmentAdded(const QDBusObjectPath &path);
	void environmentRemoved(const QDBusObjectPath &path);
	void stateChanged(int newState, int oldState);
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

Q_SIGNALS: // SIGNALS
	void deviceAdded(const QDBusObjectPath &objectpath);
	void deviceRemoved(const QDBusObjectPath &objectpath);
};

/*
* Proxy class for interface DBus.Environment
*/
class DBusEnvironmentInterface: public QDBusAbstractInterface {
	Q_OBJECT
private:
	CDevice * device;
public:
	static inline const char *staticInterfaceName()
	{ return NUT_DBUS_URL ".Environment"; }

public:
	DBusEnvironmentInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

	~DBusEnvironmentInterface();

public Q_SLOTS: // METHODS
	inline QDBusReply<QList<QDBusObjectPath> > getInterfaces();
	inline QDBusReply<libnutcommon::EnvironmentProperties> getProperties();
	inline QDBusReply<libnutcommon::EnvironmentConfig> getConfig();
	inline QDBusReply<libnutcommon::SelectResult> getSelectResult();
	inline QDBusReply<QVector<libnutcommon::SelectResult> > getSelectResults();

Q_SIGNALS: // SIGNALS
	void stateChanged(bool state);
	void selectResultsChanged(libnutcommon::SelectResult result, QVector<libnutcommon::SelectResult> results);
};

/*
* Proxy class for interface DBus.Interface
*/
class DBusInterfaceInterface_IPv4: public QDBusAbstractInterface {
	Q_OBJECT
private:
	CDevice * device;
public:
	static inline const char *staticInterfaceName()
	{ return NUT_DBUS_URL ".Interface_IPv4"; }

public:
	DBusInterfaceInterface_IPv4(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

	~DBusInterfaceInterface_IPv4();

public Q_SLOTS: // METHODS
	inline QDBusReply<void> activate();
	inline QDBusReply<void> deactivate();
	inline QDBusReply<libnutcommon::InterfaceProperties> getProperties();
	inline QDBusReply<libnutcommon::IPv4Config> getConfig();
	inline QDBusReply<void> setDynamic();
	inline QDBusReply<bool> needUserSetup();
	inline QDBusReply<bool> setUserConfig(libnutcommon::IPv4UserConfig userConfig);
	inline QDBusReply<libnutcommon::IPv4UserConfig> getUserConfig();

Q_SIGNALS: // SIGNALS
	void stateChanged(libnutcommon::InterfaceProperties properties);
};
/*
namespace DBus {
typedef ::DBusDeviceInterface Device;
typedef DBus::DBusDeviceManagerInterface DeviceManager;
typedef DBus::DBusEnvironmentInterface Environment;
typedef DBus::DBusInterfaceInterface_IPv4 Interface;
}*/
}
#include "libnutclient/client.h"

namespace libnutclient {
//Methods
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
	device->incrementLock();
	QDBusMessage msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getEnvironments"), argumentList);
	device->decrementLock();
	return msg;
}
inline QDBusReply<libnutcommon::DeviceProperties> DBusDeviceInterface::getProperties() {
	QList<QVariant> argumentList;
	return callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getProperties"), argumentList);
}
inline QDBusReply<QString> DBusDeviceInterface::getEssid() {
	QList<QVariant> argumentList;
	device->incrementLock();
	QDBusMessage msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getEssid"), argumentList);
	device->decrementLock();
	return msg;
}

inline QDBusReply<libnutcommon::DeviceConfig> DBusDeviceInterface::getConfig() {
	QList<QVariant> argumentList;
	device->incrementLock();
	QDBusMessage msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getConfig"), argumentList);
	device->decrementLock();
	return msg;
}
inline QDBusReply<void> DBusDeviceInterface::setEnvironment(QDBusObjectPath envpath) {
	QList<QVariant> argumentList;
	argumentList << qVariantFromValue(envpath);
	return callWithArgumentList(QDBus::NoBlock, QLatin1String("setEnvironment"), argumentList);
}


inline QDBusReply<QList<QDBusObjectPath> > DBusDeviceManagerInterface::getDeviceList() {
	QList<QVariant> argumentList;
	return callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getDeviceList"), argumentList);
}



//Methods

inline QDBusReply<QList<QDBusObjectPath> > DBusEnvironmentInterface::getInterfaces() {
	QList<QVariant> argumentList;
	device->incrementLock();
	QDBusMessage msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getInterfaces"), argumentList);
	device->decrementLock();
	return msg;
}

inline QDBusReply<libnutcommon::EnvironmentProperties> DBusEnvironmentInterface::getProperties() {
	QList<QVariant> argumentList;
	device->incrementLock();
	QDBusMessage msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getProperties"), argumentList);
	device->decrementLock();
	return msg;
}
inline QDBusReply<libnutcommon::EnvironmentConfig> DBusEnvironmentInterface::getConfig() {
	QList<QVariant> argumentList;
	device->incrementLock();
	QDBusMessage msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getConfig"), argumentList);
	device->decrementLock();
	return msg;
}
inline QDBusReply<libnutcommon::SelectResult> DBusEnvironmentInterface::getSelectResult() {
	QList<QVariant> argumentList;
	device->incrementLock();
	QDBusMessage msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getSelectResult"), argumentList);
	device->decrementLock();
	return msg;
}
inline QDBusReply<QVector<libnutcommon::SelectResult> > DBusEnvironmentInterface::getSelectResults() {
	QList<QVariant> argumentList;
	device->incrementLock();
	QDBusMessage msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getSelectResults"), argumentList);
	device->decrementLock();
	return msg;
}


inline QDBusReply<void> DBusInterfaceInterface_IPv4::activate() {
	QList<QVariant> argumentList;
	return callWithArgumentList(QDBus::NoBlock, QLatin1String("activate"), argumentList);
}

inline QDBusReply<void> DBusInterfaceInterface_IPv4::deactivate() {
	QList<QVariant> argumentList;
	return callWithArgumentList(QDBus::NoBlock, QLatin1String("deactivate"), argumentList);
}

inline QDBusReply<libnutcommon::InterfaceProperties> DBusInterfaceInterface_IPv4::getProperties() {
	QList<QVariant> argumentList;
	device->incrementLock();
	QDBusMessage msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getProperties"), argumentList);
	device->decrementLock();
	return msg;
}
inline QDBusReply<libnutcommon::IPv4Config> DBusInterfaceInterface_IPv4::getConfig() {
	QList<QVariant> argumentList;
	device->incrementLock();
	QDBusMessage msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getConfig"), argumentList); 
	device->decrementLock();
	return msg;
}
inline QDBusReply<void> DBusInterfaceInterface_IPv4::setDynamic() {
	QList<QVariant> argumentList;
	return callWithArgumentList(QDBus::NoBlock, QLatin1String("setDynamic"), argumentList);
}

inline QDBusReply<bool> DBusInterfaceInterface_IPv4::needUserSetup() {
	QList<QVariant> argumentList;
	device->incrementLock();
	QDBusMessage msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("needUserSetup"), argumentList);
	device->decrementLock();
	return msg;
}
inline QDBusReply<bool> DBusInterfaceInterface_IPv4::setUserConfig(libnutcommon::IPv4UserConfig userConfig) {
	QList<QVariant> argumentList;
	argumentList << qVariantFromValue(userConfig);
	device->incrementLock();
	QDBusMessage msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("setUserConfig"), argumentList);
	device->decrementLock();
	return msg;
}
inline QDBusReply<libnutcommon::IPv4UserConfig> DBusInterfaceInterface_IPv4::getUserConfig() {
	QList<QVariant> argumentList;
	device->incrementLock();
	QDBusMessage msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getUserConfig"), argumentList);
	device->decrementLock();
	return msg;
}





}
#endif
