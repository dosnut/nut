#ifndef NUTSDBUS_H
#define NUTSDBUS_H

#include <QDBusConnection>
#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <QHostAddress>
#include <QTimerEvent>

#include "libnutcommon/common.h"

namespace nuts {
	class DBusDeviceManager;
	class DBusDevice;
	class DBusEnvironment;
	class DBusInterface;
	class DBusInterface_IPv4;
#ifdef IPv6
	class DBusInterface_IPv6;
#endif
}

#include "device.h"

namespace nuts {

	class DBusDeviceManager: public QDBusAbstractAdaptor {
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".DeviceManager")
	private:
		QDBusConnection m_dbusConnection;
		DeviceManager * m_devmgr;
		QHash<QString, DBusDevice *> m_dbusDevices;
		static const QDBusObjectPath m_dbusPath, m_dbusDevicesPath;
		int m_timerId;
		libnutcommon::CDBusMonitor m_dbusMonitor;
		bool registered;

		void initTree();
		void timerEvent(QTimerEvent *event);


		/** Registers DeviceManager+subtree with dbus **/
		void registerAll();
		/** Unregisters DeviceManager+subtree with dbus **/
		void unregisterAll();
		/**Sets a new dbus connection for all subtrees **/
		void setAllDBusConnection(QDBusConnection * connection);
	private slots:
		void devAdded(QString devName, Device *dev);
		void devRemoved(QString devName, Device *dev);

		friend int mainApp(int argc, char* argv[]);
		friend class DeviceManager;

		void stopDBus();
		void dbusStopped();
		void dbusStarted();
	public:
		DBusDeviceManager(DeviceManager *devmgr);
		virtual ~DBusDeviceManager();

	// DBus API: de.unistuttgart.nut /manager de.unistuttgart.nut.DeviceManager.*
	public slots:
		QList<QDBusObjectPath> getDeviceList();
		QStringList getDeviceNames();

#if 0
		// wishlist:
		bool createBridge(QString name);
		bool destroyBridge(QDBusObjectPath devicePath);
		bool destroyBridge(qint32 deviceId);
		bool addToBridge(QDBusObjectPath bridge, QList<QDBusObjectPath> devicePaths);
		bool addToBridge(qint32 bridgeId, QList<qint32> deviceIds);
		bool removeFromBridge(QDBusObjectPath bridge, QList<QDBusObjectPath> devicePaths);
		bool removeFromBridge(qint32 bridgeId, QList<qint32> deviceIds);
#endif

	signals:
		void deviceAdded(const QDBusObjectPath &objectpath);
		void deviceRemoved(const QDBusObjectPath &objectpath);
		void deviceAdded(const QString &devname);
		void deviceRemoved(const QString &devname);
	};

	class DBusDevice: public QDBusAbstractAdaptor {
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".Device")
	private:
		Device *m_device;
		QDBusConnection *m_dbusConnection;
		QList<DBusEnvironment* > m_dbusEnvironments;
		QDBusObjectPath m_dbusPath;
		int m_activeEnvironment;
		bool registered;

	private slots:
		void devStateChanged(libnutcommon::DeviceState newState, libnutcommon::DeviceState oldState);
		void devEnvironmentChanged(int newEnvironment);

	public:
		DBusDevice(Device *dev, QDBusConnection *connection, const QString &path);
		virtual ~DBusDevice();

		QDBusObjectPath getPath();
		void registerAll();
		void unregisterAll();
		void setAllDBusConnection(QDBusConnection * connection);

	// DBus API: de.unistuttgart.nut /devices/<devname> de.unistuttgart.nut.Device.*
	public slots:
		Q_NOREPLY void enable();
		Q_NOREPLY void disable();

		Q_NOREPLY void setEnvironment(const QDBusObjectPath &path);
		Q_NOREPLY void setEnvironment(qint32 env);

		libnutcommon::DeviceProperties getProperties();
		QList<QDBusObjectPath> getEnvironments();
		QList<qint32> getEnvironmentIds();
		libnutcommon::DeviceConfig getConfig();
		libnutcommon::DeviceState getState();
		libnutcommon::DeviceType getType();
		libnutcommon::OptionalQDBusObjectPath getActiveEnvironment();
		qint32 getActiveEnvironmentIndex();
		QString getEssid();

	signals:
		void stateChanged(libnutcommon::DeviceState newState, libnutcommon::DeviceState oldState);
		void environmentChangedActive(const QDBusObjectPath &objectpath);
		void environmentChangedActive(qint32 envId);
		void newWirelessNetworkFound();
	};

	class DBusEnvironment: public QDBusAbstractAdaptor {
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".Environment")
	private:
		Environment *m_environment;
		QDBusConnection *m_dbusConnection;
		QList<DBusInterface_IPv4*> m_dbusInterfacesIPv4;
#ifdef IPv6
		QList<DBusInterface_IPv6*> m_dbusInterfacesIPv6;
#endif
		QDBusObjectPath m_dbusPath;
		Device * m_device;
		bool registered;

	private slots:
		void selectResultReady();
	public:
		DBusEnvironment(Environment *env, QDBusConnection *connection, const QDBusObjectPath &path, Device* dev);
		virtual ~DBusEnvironment();
		inline Environment * getEnvironment() const { return m_environment; }

		QDBusObjectPath getPath();
		void registerAll();
		void unregisterAll();
		void setAllDBusConnection(QDBusConnection * connection);
		void emitChange(bool change);

	// DBus API: de.unistuttgart.nut /devices/<devname>/<envNdx> de.unistuttgart.nut.Environment.*
	public slots:
		libnutcommon::EnvironmentProperties getProperties();
		libnutcommon::EnvironmentConfig getConfig();
		QList<QDBusObjectPath> getInterfaces();
		QList<qint32> getInterfaceIds();
		libnutcommon::SelectResult getSelectResult();
		QVector<libnutcommon::SelectResult> getSelectResults();

		/** <envNdx> in QDBusObjectPath for this object */
		qint32 getID();
		QString getName();
		/** whether this environment is the active environment for the device */
		bool getState();
	signals:
		void interfaceAdded(const QDBusObjectPath &objectpath);
		void interfaceRemoved(const QDBusObjectPath &objectpath);
		void selectsResultChanged(libnutcommon::SelectResult result, QVector<libnutcommon::SelectResult> results);
		void stateChanged(bool state);
	};

	class DBusInterface_IPv4: public QDBusAbstractAdaptor {
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".Interface_IPv4")
	private:
		Interface_IPv4 *m_interface;
		QDBusConnection *m_dbusConnection;
		QDBusObjectPath m_dbusPath;
		bool registered;
	private slots:
		void interfaceStatusChanged(libnutcommon::InterfaceState state);
	public:
		DBusInterface_IPv4(Interface_IPv4 *iface, QDBusConnection *connection, const QDBusObjectPath &path);
		virtual ~DBusInterface_IPv4();

		QDBusObjectPath getPath();
		void registerAll();
		void unregisterAll();
		void setAllDBusConnection(QDBusConnection * connection) { m_dbusConnection = connection; }

	// DBus API: de.unistuttgart.nut /devices/<devname>/<envNdx>/<ifNdx> de.unistuttgart.nut.Interface_IPv4.*
	public slots:
		libnutcommon::InterfaceProperties getProperties();
		libnutcommon::IPv4Config getConfig();

		bool needUserSetup();
		bool setUserConfig(libnutcommon::IPv4UserConfig userConfig);
		libnutcommon::IPv4UserConfig getUserConfig();

		libnutcommon::InterfaceState getState();
		QHostAddress getIP();
		QHostAddress getNetmask();
		QHostAddress getGateway();
		QList<QHostAddress> getDns();

	signals:
		void stateChanged(const libnutcommon::InterfaceProperties &properties);
	};

#ifdef IPv6
	class DBusInterface_IPv6: public QDBusAbstractAdaptor {
	Q_OBJECT
	Q_CLASSINFO("D-BUS Interface", "de.unistuttgart.nut" ".Interface_IPv6");
	};
#endif
}

#endif
