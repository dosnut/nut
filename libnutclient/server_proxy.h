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


class CDevice;

/*
* Proxy class for interface DBus.DeviceManager
*/
class DBusDeviceManagerInterface: public QDBusAbstractInterface {
	Q_OBJECT
public:
	static inline const char *staticInterfaceName()
	{ return NUT_DBUS_URL ".DeviceManager"; }

	enum { GET_DEVICE_LIST };

public:
	DBusDeviceManagerInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

	~DBusDeviceManagerInterface();

public slots: // METHODS
	void getDeviceList();
// 	QDBusReply<bool> createBridge(QList<QDBusObjectPath> devicePaths);
// 	QDBusReply<bool> destroyBridge(QDBusObjectPath devicePath);
// 	QDBusReply<bool> addToBridge(QDBusObjectPath bridge, QList<QDBusObjectPath> devicePaths);

private slots:
	void dbret_errorOccured(QDBusError error);

	void dbret_getDeviceList(QList<QDBusObjectPath> devices);
	
signals: // SIGNALS
	//Return functions
	void gotDeviceList(QList<QDBusObjectPath> devices);
// 	void createdBridge(bool worked);
// 	void destroyedBrdige(bool worked);
	//DBUS-SIGNALS
	void deviceAdded(const QDBusObjectPath &objectpath);
	void deviceRemoved(const QDBusObjectPath &objectpath);

	void errorOccured(QDBusError error/*, int method */); //TODO:Finer errors, needz zem with the call
	void queueErrorOccured(QString method);
};

/*
* Proxy class for interface DBus.Device
*/
class DBusDeviceInterface: public QDBusAbstractInterface {
	Q_OBJECT
public:
	static inline const char *staticInterfaceName()
	{ return NUT_DBUS_URL ".Device"; }
	CDevice * m_device;

public:
	DBusDeviceInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

	~DBusDeviceInterface();

public Q_SLOTS: // METHODS
	void disable();
	void enable();
	void setEnvironment(QDBusObjectPath envpath);
	void setEnvironment(qint32 env);
	void getEnvironments();
	void getProperties();
	void getEssid();
	void getConfig();
	void getActiveEnvironment();

private slots:
	void dbret_dummy(QDBusMessage) {};
	void dbret_getEnvironments(QList<QDBusObjectPath> envs) { emit gotEnvironments(envs); }
	void dbret_getProperties(libnutcommon::DeviceProperties props) { emit gotProperties(props); }
	void dbret_getEssid(QString essid) { emit gotEssid(essid); }
	void dbret_getConfig(libnutcommon::DeviceConfig config) { emit gotConfig(config); }
	void dbret_getActiveEnvironment(QString activeEnv) { emit gotActiveEnvironment(activeEnv); }
	void dbret_errorOccured(QDBusError error) { emit errorOccured(error); }

Q_SIGNALS: // SIGNALS
	//Return function signals
	void gotEnvironments(QList<QDBusObjectPath> envs);
	void gotProperties(libnutcommon::DeviceProperties props);
	void gotEssid(QString essid);
	void gotConfig(libnutcommon::DeviceConfig config);
	void gotActiveEnvironment(QString activeEnv);

	void errorOccured(QDBusError error);
	void queueErrorOccured(QString method);

	//DBUS Signals
	void environmentChangedActive(const QString &newenv);
	void stateChanged(int newState, int oldState);
	void newWirelssNetworkFound();
	
};


/*
* Proxy class for interface DBus.Environment
*/
class DBusEnvironmentInterface: public QDBusAbstractInterface {
	Q_OBJECT
private:
	CDevice * m_device;
public:
	static inline const char *staticInterfaceName()
	{ return NUT_DBUS_URL ".Environment"; }

public:
	DBusEnvironmentInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

	~DBusEnvironmentInterface();

public Q_SLOTS: // METHODS
	void getInterfaces();
	void getProperties();
	void getConfig();
	void getSelectResult();
	void getSelectResults();

private slots:
	void dbret_getInterfaces(QList<QDBusObjectPath> interfaces) { emit gotInterfaces(interfaces); }
	void dbret_getProperties(libnutcommon::EnvironmentProperties properties) { emit gotProperties(properties); }
	void dbret_getConfig(libnutcommon::EnvironmentConfig config) { emit gotConfig(config); }
	void dbret_getSelectResult(libnutcommon::SelectResult selectResult) { emit gotSelectResult(selectResult); }
	void dbret_getSelectResults(QVector<libnutcommon::SelectResult> selectResults) { emit gotSelectResults(selectResults); }
	void dbret_errorOccured(QDBusError error) { emit errorOccured(error,""); }

Q_SIGNALS: // SIGNALS
	
	//dbus return functions
	void gotInterfaces(QList<QDBusObjectPath>);
	void gotProperties(libnutcommon::EnvironmentProperties properties);
	void gotConfig(libnutcommon::EnvironmentConfig config);
	void gotSelectResult(libnutcommon::SelectResult selectResult);
	void gotSelectResults(QVector<libnutcommon::SelectResult> selectResults);

	void errorOccured(QDBusError error, QString method);
	void queueErrorOccured(QString method);

	//dbus signals
	void stateChanged(bool state);
	void selectResultsChanged(libnutcommon::SelectResult result, QVector<libnutcommon::SelectResult> results);
};


/*
* Proxy class for interface DBus.Interface
*/
class DBusInterfaceInterface_IPv4: public QDBusAbstractInterface {
	Q_OBJECT
private:
	CDevice * m_device;
public:
	static inline const char *staticInterfaceName()
	{ return NUT_DBUS_URL ".Interface_IPv4"; }

public:
	DBusInterfaceInterface_IPv4(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

	~DBusInterfaceInterface_IPv4();

public Q_SLOTS: // METHODS
	void activate();
	void deactivate();
	void getProperties();
	void getConfig();
	void setDynamic();
	void needUserSetup(); //returns bool
	void setUserConfig(libnutcommon::IPv4UserConfig userConfig); //returns bool
	void getUserConfig();

private slots:
	void dbret_dummy(QDBusMessage) {};
	void dbret_getProperties(libnutcommon::InterfaceProperties properties) { emit gotProperties(properties); }
	void dbret_getConfig(libnutcommon::IPv4Config config) { emit gotConfig(config); }
	void dbret_getNeedUserSetup(bool need) { emit gotNeedUserSetup(need); }
	void dbret_getSetUserConfig(bool worked) { emit gotSetUserConfig(worked); }
	void dbret_getUserConfig(libnutcommon::IPv4UserConfig userConfig) { emit gotUserConfig(userConfig); }
	void dbret_errorOccured(QDBusError error) { emit errorOccured(error,""); }

Q_SIGNALS: // SIGNALS
	
	//dbus return functions
	void gotProperties(libnutcommon::InterfaceProperties properties);
	void gotConfig(libnutcommon::IPv4Config config);
	void gotNeedUserSetup(bool need);
	void gotSetUserConfig(bool worked);
	void gotUserConfig(libnutcommon::IPv4UserConfig config);

	void errorOccured(QDBusError error, QString method);
	void queueErrorOccured(QString method);

	//dbus signals
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
