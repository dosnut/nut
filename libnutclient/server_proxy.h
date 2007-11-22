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
class DBusDeviceInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return NUT_DBUS_URL ".Device"; }

public:
    DBusDeviceInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~DBusDeviceInterface();

public Q_SLOTS: // METHODS
    inline QDBusReply<void> disable()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::NoBlock, QLatin1String("disable"), argumentList);
    }

    inline QDBusReply<void> enable()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::NoBlock, QLatin1String("enable"), argumentList);
    }

    inline QDBusReply<QList<QDBusObjectPath> > getEnvironments()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("getEnvironments"), argumentList);
    }
    inline QDBusReply<libnutcommon::DeviceProperties> getProperties() {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("getProperties"), argumentList);
    }
    inline QDBusReply<QString> getEssid() {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("getEssid"), argumentList);
    }
	
	inline QDBusReply<libnutcommon::DeviceConfig> getConfig()
	{
		QList<QVariant> argumentList;
		return callWithArgumentList(QDBus::Block, QLatin1String("getConfig"), argumentList);
	}
    inline QDBusReply<void> setEnvironment(QDBusObjectPath envpath) {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(envpath);
        return callWithArgumentList(QDBus::NoBlock, QLatin1String("setEnvironment"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void environmentChangedActive(const QString &newenv);
    void environmentAdded(const QDBusObjectPath &path);
    void environmentRemoved(const QDBusObjectPath &path);
    void stateChanged(int newState, int oldState);
};

/*
 * Proxy class for interface DBus.DeviceManager
 */
class DBusDeviceManagerInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return NUT_DBUS_URL ".DeviceManager"; }

public:
    DBusDeviceManagerInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~DBusDeviceManagerInterface();

public Q_SLOTS: // METHODS
    inline QDBusReply<QList<QDBusObjectPath> > getDeviceList()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("getDeviceList"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void deviceAdded(const QDBusObjectPath &objectpath);
    void deviceRemoved(const QDBusObjectPath &objectpath);
};

/*
 * Proxy class for interface DBus.Environment
 */
class DBusEnvironmentInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return NUT_DBUS_URL ".Environment"; }

public:
    DBusEnvironmentInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~DBusEnvironmentInterface();

public Q_SLOTS: // METHODS
    inline QDBusReply<QList<QDBusObjectPath> > getInterfaces()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("getInterfaces"), argumentList);
    }

    inline QDBusReply<libnutcommon::EnvironmentProperties> getProperties()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("getProperties"), argumentList);
    }
	inline QDBusReply<libnutcommon::EnvironmentConfig> getConfig()
	{
		QList<QVariant> argumentList;
		return callWithArgumentList(QDBus::Block, QLatin1String("getConfig"), argumentList);
	}
	inline QDBusReply<libnutcommon::SelectResult> getSelectResult()
	{
		QList<QVariant> argumentList;
		return callWithArgumentList(QDBus::Block, QLatin1String("getSelectResult"), argumentList);
	}
	inline QDBusReply<QVector<libnutcommon::SelectResult> > getSelectResults()
	{
		QList<QVariant> argumentList;
		return callWithArgumentList(QDBus::Block, QLatin1String("getSelectResults"), argumentList);
	}

Q_SIGNALS: // SIGNALS
    void stateChanged(bool state);
	void selectResultsChanged(libnutcommon::SelectResult result, QVector<libnutcommon::SelectResult> results);
};

/*
 * Proxy class for interface DBus.Interface
 */
class DBusInterfaceInterface_IPv4: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return NUT_DBUS_URL ".Interface_IPv4"; }

public:
    DBusInterfaceInterface_IPv4(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~DBusInterfaceInterface_IPv4();

public Q_SLOTS: // METHODS
    inline QDBusReply<void> activate()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::NoBlock, QLatin1String("activate"), argumentList);
    }

    inline QDBusReply<void> deactivate()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::NoBlock, QLatin1String("deactivate"), argumentList);
    }

    inline QDBusReply<libnutcommon::InterfaceProperties> getProperties()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("getProperties"), argumentList);
    }
	inline QDBusReply<libnutcommon::IPv4Config> getConfig()
	{
		QList<QVariant> argumentList;
		return callWithArgumentList(QDBus::Block, QLatin1String("getConfig"), argumentList); 
	}
    inline QDBusReply<void> setDynamic()
    {
		QList<QVariant> argumentList;
		return callWithArgumentList(QDBus::NoBlock, QLatin1String("setDynamic"), argumentList);
    }

	inline QDBusReply<bool> needUserSetup() {
		QList<QVariant> argumentList;
		return callWithArgumentList(QDBus::Block, QLatin1String("needUserSetup"), argumentList);
	}
	inline QDBusReply<bool> setUserConfig(libnutcommon::IPv4UserConfig userConfig) {
		QList<QVariant> argumentList;
		argumentList << qVariantFromValue(userConfig);
		return callWithArgumentList(QDBus::Block, QLatin1String("setUserConfig"), argumentList);
	}
	inline QDBusReply<libnutcommon::IPv4UserConfig> getUserConfig() {
		QList<QVariant> argumentList;
		return callWithArgumentList(QDBus::Block, QLatin1String("getUserConfig"), argumentList);
	}

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
#endif
