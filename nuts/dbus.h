#ifndef _NUTS_DBUS_H
#define _NUTS_DBUS_H

#pragma once

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
	class DBusDeviceManager final: public libnutcommon::DBusAbstractAdaptor {
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".DeviceManager")
		Q_PROPERTY(QList<QDBusObjectPath> deviceList READ getDeviceList)
		Q_PROPERTY(QStringList deviceNames READ getDeviceNames)
	public:
		explicit DBusDeviceManager(DeviceManager* devmgr);

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
		void deviceAddedPath(QDBusObjectPath const& objectpath);
		void deviceRemovedPath(QDBusObjectPath const& objectpath);
		void deviceAddedName(QString const& devname);
		void deviceRemovedName(QString const& devname);

	private slots:
		void devAdded(QString devName, Device* dev);
		void devRemoved(QString devName, Device* dev);

	private:
		DeviceManager* const m_devmgr;
		QHash<QString, DBusDevice*> m_dbusDevices;
		static const QDBusObjectPath m_dbusPath, m_dbusDevicesPath;
	};

	class DBusDevice final: public libnutcommon::DBusAbstractAdaptor {
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".Device")
		Q_PROPERTY(libnutcommon::DeviceProperties properties READ getProperties NOTIFY propertiesChanged)
		Q_PROPERTY(QString name READ getName CONSTANT)
		Q_PROPERTY(libnutcommon::DeviceType type READ getType)
		Q_PROPERTY(libnutcommon::OptionalQDBusObjectPath activeEnvironment READ getActiveEnvironment NOTIFY activeEnvironmentChanged)
		Q_PROPERTY(qint32 activeEnvironmentIndex READ getActiveEnvironmentIndex NOTIFY activeEnvironmentChanged)
		Q_PROPERTY(libnutcommon::DeviceState state READ getState NOTIFY stateChanged)
		Q_PROPERTY(QString essid READ getEssid)
		Q_PROPERTY(libnutcommon::MacAddress macAddress READ getMacAddress)
		Q_PROPERTY(libnutcommon::DeviceConfig config READ getConfig CONSTANT)
		Q_PROPERTY(QList<QDBusObjectPath> environments READ getEnvironments CONSTANT)
		Q_PROPERTY(QList<qint32> environmentIds READ getEnvironmentIds CONSTANT)
	public:
		explicit DBusDevice(Device* dev, QString const& path);

	// DBus API: de.unistuttgart.nut /devices/<devname> de.unistuttgart.nut.Device.*
	public slots:
		libnutcommon::DeviceProperties getProperties();
		/* single properties */
		/* constant properties */
		QString getName();
		/* variable properties */
		libnutcommon::DeviceType getType();
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

	private slots:
		void checkPropertiesUpdate();
		void devPropertiesChanged(libnutcommon::DeviceProperties properties);
		void devActiveEnvironmentChanged(int environment);

	private:
		Device* const m_device;
		QList<DBusEnvironment*> m_dbusEnvironments;

		libnutcommon::DeviceProperties m_properties, m_last_notified_properties;
		int m_activeEnvironment;
	};

	class DBusEnvironment final: public libnutcommon::DBusAbstractAdaptor {
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".Environment")
		Q_PROPERTY(libnutcommon::EnvironmentProperties properties READ getProperties)
		Q_PROPERTY(qint32 ID READ getID)
		Q_PROPERTY(QString name READ getName)
		Q_PROPERTY(bool active READ isActive)
		Q_PROPERTY(libnutcommon::SelectResult selectResult READ getSelectResult)
		Q_PROPERTY(QVector<libnutcommon::SelectResult> selectResults READ getSelectResults)
		Q_PROPERTY(libnutcommon::EnvironmentConfig config READ getConfig)
		Q_PROPERTY(QList<QDBusObjectPath> interfaces READ getInterfaces)
		Q_PROPERTY(QList<qint32> interfaceIds READ getInterfaceIds)
	public:
		explicit DBusEnvironment(Environment* env, QDBusObjectPath const& path);
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

	private slots:
		void checkPropertiesUpdate();
		void envPropertiesChanged(libnutcommon::EnvironmentProperties properties);

	private:
		Environment* const m_environment;
		QList<DBusInterface_IPv4*> m_dbusInterfacesIPv4;
#ifdef IPv6
		QList<DBusInterface_IPv6*> m_dbusInterfacesIPv6;
#endif

		libnutcommon::EnvironmentProperties m_properties, m_last_notified_properties;
	};

	class DBusInterface_IPv4 final: public libnutcommon::DBusAbstractAdaptor {
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".Interface_IPv4")
		Q_PROPERTY(libnutcommon::InterfaceProperties properties READ getProperties NOTIFY propertiesChanged)
		Q_PROPERTY(libnutcommon::InterfaceState state READ getState)
		Q_PROPERTY(QHostAddress IP READ getIP)
		Q_PROPERTY(QHostAddress netmask READ getNetmask)
		Q_PROPERTY(QHostAddress gateway READ getGateway)
		Q_PROPERTY(QList<QHostAddress> dnsServers READ getDnsServers)
		Q_PROPERTY(int gatewayMetric READ getGatewayMetric)
		Q_PROPERTY(bool needUserSetup READ needUserSetup)
		Q_PROPERTY(libnutcommon::IPv4Config config READ getConfig CONSTANT)
		Q_PROPERTY(libnutcommon::IPv4UserConfig userConfig READ getUserConfig NOTIFY userConfigChanged)
	public:
		explicit DBusInterface_IPv4(Interface_IPv4* iface, QDBusObjectPath const& path);

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

	private slots:
		void checkPropertiesUpdate();
		void checkUserConfigUpdate();
		void interfacePropertiesChanged(libnutcommon::InterfaceProperties properties);
		void interfaceUserConfigChanged(libnutcommon::IPv4UserConfig userConfig);

	private:
		Interface_IPv4* const m_interface;

		libnutcommon::InterfaceProperties m_properties, m_last_notified_properties;
		libnutcommon::IPv4UserConfig m_userConfig, m_last_notified_userConfig;
	};

#ifdef IPv6
	class DBusInterface_IPv6: public libnutcommon::DBusAbstractAdapater {
		Q_OBJECT
		Q_CLASSINFO("D-BUS Interface", "de.unistuttgart.nut" ".Interface_IPv6");
	};
#endif
}

#endif /* _NUTS_DBUS_H */
