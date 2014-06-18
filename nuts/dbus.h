#ifndef NUTSDBUS_H
#define NUTSDBUS_H

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

	class DBusDeviceManager: public libnutcommon::DBusAbstractAdapater {
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".DeviceManager")
	private:
		DeviceManager* const m_devmgr;
		QHash<QString, DBusDevice*> m_dbusDevices;
		static const QDBusObjectPath m_dbusPath, m_dbusDevicesPath;

	private slots:
		void devAdded(QString devName, Device* dev);
		void devRemoved(QString devName, Device* dev);

	public:
		DBusDeviceManager(DeviceManager* devmgr);

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
		void deviceAdded(const QDBusObjectPath& objectpath);
		void deviceRemoved(const QDBusObjectPath& objectpath);
		void deviceAdded(const QString& devname);
		void deviceRemoved(const QString& devname);
	};

	class DBusDevice: public libnutcommon::DBusAbstractAdapater {
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".Device")
	private:
		Device* const m_device;
		QList<DBusEnvironment*> m_dbusEnvironments;

		libnutcommon::DeviceProperties m_properties, m_last_notified_properties;
		int m_activeEnvironment;

	private slots:
		void checkPropertiesUpdate();
		void devPropertiesChanged(libnutcommon::DeviceProperties properties);
		void devActiveEnvironmentChanged(int environment);

	public:
		DBusDevice(Device* dev, const QString& path);

	// DBus API: de.unistuttgart.nut /devices/<devname> de.unistuttgart.nut.Device.*
	public slots:
		libnutcommon::DeviceProperties getProperties();
		/* single properties */
		/* constant properties */
		QString getName();
		libnutcommon::DeviceType getType();
		/* variable properties */
		libnutcommon::OptionalQDBusObjectPath getActiveEnvironment();
		qint32 getActiveEnvironmentIndex();
		libnutcommon::DeviceState getState();
		QString getEssid();
		libnutcommon::MacAddress getMacAddress();

		/* constant config */
		libnutcommon::DeviceConfig getConfig();

		/* list of sub objects (list doesn't change) */
		QList<QDBusObjectPath> getEnvironments();
		QList<qint32> getEnvironmentIds();

		/* actions */
		Q_NOREPLY void enable();
		Q_NOREPLY void disable();
		Q_NOREPLY void setEnvironment(QDBusObjectPath path);
		Q_NOREPLY void setEnvironment(qint32 env);

	signals:
		void propertiesChanged(libnutcommon::DeviceProperties properties);
		void stateChanged(libnutcommon::DeviceState state);
		void activeEnvironmentChanged(libnutcommon::OptionalQDBusObjectPath objectpath);
		void activeEnvironmentChanged(qint32 envId);
		void newWirelessNetworkFound();
	};

	class DBusEnvironment: public libnutcommon::DBusAbstractAdapater {
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".Environment")
	private:
		Environment* const m_environment;
		QList<DBusInterface_IPv4*> m_dbusInterfacesIPv4;
#ifdef IPv6
		QList<DBusInterface_IPv6*> m_dbusInterfacesIPv6;
#endif

		libnutcommon::EnvironmentProperties m_properties, m_last_notified_properties;

	private slots:
		void checkPropertiesUpdate();
		void envPropertiesChanged(libnutcommon::EnvironmentProperties properties);

	public:
		DBusEnvironment(Environment *env, const QDBusObjectPath &path);
		Environment* getEnvironment() const { return m_environment; }

	// DBus API: de.unistuttgart.nut /devices/<devname>/<envNdx> de.unistuttgart.nut.Environment.*
	public slots:
		libnutcommon::EnvironmentProperties getProperties();
		/* single properties */
		/* constant properties */
		qint32 getID(); /*! <envNdx> in QDBusObjectPath for this object */
		QString getName();
		/* variable properties */
		bool isActive();
		libnutcommon::SelectResult getSelectResult();
		QVector<libnutcommon::SelectResult> getSelectResults();

		/* constant config */
		libnutcommon::EnvironmentConfig getConfig();

		/* list of sub objects (list doesn't change) */
		QList<QDBusObjectPath> getInterfaces();
		QList<qint32> getInterfaceIds();

	signals:
		void propertiesChanged(libnutcommon::EnvironmentProperties properties);
	};

	class DBusInterface_IPv4: public libnutcommon::DBusAbstractAdapater {
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".Interface_IPv4")
	private:
		Interface_IPv4* const m_interface;

		libnutcommon::InterfaceProperties m_properties, m_last_notified_properties;
		libnutcommon::IPv4UserConfig m_userConfig, m_last_notified_userConfig;

	private slots:
		void checkPropertiesUpdate();
		void checkUserConfigUpdate();
		void interfacePropertiesChanged(libnutcommon::InterfaceProperties properties);
		void interfaceUserConfigChanged(libnutcommon::IPv4UserConfig userConfig);

	public:
		DBusInterface_IPv4(Interface_IPv4* iface, const QDBusObjectPath& path);

	// DBus API: de.unistuttgart.nut /devices/<devname>/<envNdx>/<ifNdx> de.unistuttgart.nut.Interface_IPv4.*
	public slots:
		libnutcommon::InterfaceProperties getProperties();
		/* single properties */
		/* variable properties */
		libnutcommon::InterfaceState getState();
		QHostAddress getIP();
		QHostAddress getNetmask();
		QHostAddress getGateway();
		QList<QHostAddress> getDnsServers();
		int getGatewayMetric();
		bool needUserSetup();

		/* constant config */
		libnutcommon::IPv4Config getConfig();

		/* actions */
		bool setUserConfig(libnutcommon::IPv4UserConfig userConfig);
		libnutcommon::IPv4UserConfig getUserConfig();

	signals:
		void propertiesChanged(libnutcommon::InterfaceProperties properties);
		void userConfigChanged(libnutcommon::IPv4UserConfig userConfig);
	};

#ifdef IPv6
	class DBusInterface_IPv6: public libnutcommon::DBusAbstractAdapater {
	Q_OBJECT
	Q_CLASSINFO("D-BUS Interface", "de.unistuttgart.nut" ".Interface_IPv6");
	};
#endif
}

#endif
