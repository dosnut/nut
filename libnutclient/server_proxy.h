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
	CDevice * m_device;
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




//Methods

inline QDBusReply<QList<QDBusObjectPath> > DBusEnvironmentInterface::getInterfaces() {
	QList<QVariant> argumentList;
	QDBusMessage msg;
	if ( m_device->incrementLock() ) {
		msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getInterfaces"), argumentList);
		m_device->decrementLock();
	}
	else {
		msg.createErrorReply(QDBusError::AccessDenied, "Server removed Device");
	}
	return msg;
}

inline QDBusReply<libnutcommon::EnvironmentProperties> DBusEnvironmentInterface::getProperties() {
	QList<QVariant> argumentList;
	QDBusMessage msg;
	if ( m_device->incrementLock() ) {
		msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getProperties"), argumentList);
		m_device->decrementLock();
	}
	else {
		msg.createErrorReply(QDBusError::AccessDenied, "Server removed Device");
	}
	return msg;
}
inline QDBusReply<libnutcommon::EnvironmentConfig> DBusEnvironmentInterface::getConfig() {
	QList<QVariant> argumentList;
	QDBusMessage msg;
	if ( m_device->incrementLock() ) {
		msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getConfig"), argumentList);
		m_device->decrementLock();
	}
	else {
		msg.createErrorReply(QDBusError::AccessDenied, "Server removed Device");
	}
	return msg;
}
inline QDBusReply<libnutcommon::SelectResult> DBusEnvironmentInterface::getSelectResult() {
	QList<QVariant> argumentList;
	QDBusMessage msg;
	if ( m_device->incrementLock() ) {
		msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getSelectResult"), argumentList);
		m_device->decrementLock();
	}
	else {
		msg.createErrorReply(QDBusError::AccessDenied, "Server removed Device");
	}
	return msg;
}
inline QDBusReply<QVector<libnutcommon::SelectResult> > DBusEnvironmentInterface::getSelectResults() {
	QList<QVariant> argumentList;
	QDBusMessage msg;
	if ( m_device->incrementLock() ) {
		msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getSelectResults"), argumentList);
		m_device->decrementLock();
	}
	else {
		msg.createErrorReply(QDBusError::AccessDenied, "Server removed Device");
	}
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
	QDBusMessage msg;
	if ( m_device->incrementLock() ) {
		msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getProperties"), argumentList);
		m_device->decrementLock();
	}
	else {
		msg.createErrorReply(QDBusError::AccessDenied, "Server removed Device");
	}
	return msg;
}
inline QDBusReply<libnutcommon::IPv4Config> DBusInterfaceInterface_IPv4::getConfig() {
	QList<QVariant> argumentList;
	QDBusMessage msg;
	if ( m_device->incrementLock() ) {
		msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getConfig"), argumentList); 
		m_device->decrementLock();
	}
	else {
		msg.createErrorReply(QDBusError::AccessDenied, "Server removed Device");
	}
	return msg;
}
inline QDBusReply<void> DBusInterfaceInterface_IPv4::setDynamic() {
	QList<QVariant> argumentList;
	return callWithArgumentList(QDBus::NoBlock, QLatin1String("setDynamic"), argumentList);
}

inline QDBusReply<bool> DBusInterfaceInterface_IPv4::needUserSetup() {
	QList<QVariant> argumentList;
	QDBusMessage msg;
	if ( m_device->incrementLock() ) {
		msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("needUserSetup"), argumentList);
		m_device->decrementLock();
	}
	else {
		msg.createErrorReply(QDBusError::AccessDenied, "Server removed Device");
	}
	return msg;
}
inline QDBusReply<bool> DBusInterfaceInterface_IPv4::setUserConfig(libnutcommon::IPv4UserConfig userConfig) {
	QList<QVariant> argumentList;
	QDBusMessage msg;
	argumentList << qVariantFromValue(userConfig);
	if ( m_device->incrementLock() ) {
		msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("setUserConfig"), argumentList);
		m_device->decrementLock();
	}
	else {
		msg.createErrorReply(QDBusError::AccessDenied, "Server removed Device");
	}
	return msg;
}
inline QDBusReply<libnutcommon::IPv4UserConfig> DBusInterfaceInterface_IPv4::getUserConfig() {
	QList<QVariant> argumentList;
	QDBusMessage msg;
	if ( m_device->incrementLock() ) {
		msg = callWithArgumentList(QDBus::BlockWithGui, QLatin1String("getUserConfig"), argumentList);
		m_device->decrementLock();
	}
	else {
		msg.createErrorReply(QDBusError::AccessDenied, "Server removed Device");
	}
	return msg;
}





}
#endif
