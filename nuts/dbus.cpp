#include "dbus.h"
#include "log.h"
#include <QHashIterator>

namespace nuts {
	using namespace libnutcommon;

	const QDBusObjectPath DBusDeviceManager::m_dbusPath("/manager");
	const QDBusObjectPath DBusDeviceManager::m_dbusDevicesPath("/devices");

	DBusDeviceManager::DBusDeviceManager(DeviceManager* devmgr)
	: DBusAbstractAdapater(m_dbusPath, devmgr), m_devmgr(devmgr) {
		registerService(NUT_DBUS_URL);

		for (auto i: m_devmgr->getDevices()) {
			devAdded(i->getName(), i);
		}

		connect(m_devmgr, &DeviceManager::deviceAdded, this, &DBusDeviceManager::devAdded);
		connect(m_devmgr, &DeviceManager::deviceRemoved, this, &DBusDeviceManager::devRemoved);
	}

	//SLOT: Inserts device into device hash
	void DBusDeviceManager::devAdded(QString devName, Device* dev) {
		if (m_dbusDevices.contains(devName)) return;

		auto dbus_device = new DBusDevice(dev, m_dbusDevicesPath.path());
		m_dbusDevices.insert(devName, dbus_device);
		registerAdaptor(dbus_device);

		emit deviceAdded(dbus_device->getPath());
		emit deviceAdded(devName);
	}


	void DBusDeviceManager::devRemoved(QString devName, Device* /* dev */) {
		if (!m_dbusDevices.contains(devName)) return;

		auto dbus_device = m_dbusDevices[devName];
		m_dbusDevices.remove(devName);

		emit deviceRemoved(dbus_device->getPath());
		emit deviceRemoved(devName);
	}

	QList<QDBusObjectPath> DBusDeviceManager::getDeviceList() {
		QList<QDBusObjectPath> paths;
		for (DBusDevice* dbus_device: m_dbusDevices) {
			paths.append(dbus_device->getPath());
		}
		return paths;
	}

	QStringList DBusDeviceManager::getDeviceNames() {
		QStringList names;
		for (nuts::Device* dev: m_devmgr->getDevices()) {
			names.append(dev->getName());
		}
		return names;
	}

#if 0
	bool DBusDeviceManager::createBridge(QString /*name*/) { return false; }
	bool DBusDeviceManager::destroyBridge(QDBusObjectPath /*devicePath*/) { return false; }
	bool DBusDeviceManager::destroyBridge(qint32 /*deviceId*/) { return false; }
	bool DBusDeviceManager::addToBridge(QDBusObjectPath /*bridge*/, QList<QDBusObjectPath> /*devicePaths*/) { return false; }
	bool DBusDeviceManager::addToBridge(qint32 /*bridgeId*/, QList<qint32> /*deviceIds*/) { return false; }
	bool DBusDeviceManager::removeFromBridge(QDBusObjectPath /*bridge*/, QList<QDBusObjectPath> /*devicePaths*/) { return false; }
	bool DBusDeviceManager::removeFromBridge(qint32 /*bridgeId*/, QList<qint32> /*deviceIds*/) { return false; }
#endif

	static inline QDBusObjectPath makeDevicePath(const QString& path, const QString& name) {
		return QDBusObjectPath(path + "/" + name);
	}

	DBusDevice::DBusDevice(Device* dev, const QString& path)
	: DBusAbstractAdapater(makeDevicePath(path, dev->getName()), dev), m_device(dev) {
		m_properties = dev->getProperties();
		// remember active environment
		m_activeEnvironment = m_device->getEnvironment();

		//Add Environments
		for (Environment* env: m_device->getEnvironments()) {
			DBusEnvironment* dbus_env = new DBusEnvironment(env, m_path);
			m_dbusEnvironments.append(dbus_env);
			registerAdaptor(dbus_env);
		}

		m_properties.activeEnvironment = m_activeEnvironment >= 0
			? m_dbusEnvironments[m_activeEnvironment]->getPath()
			: OptionalQDBusObjectPath { };

		m_last_notified_properties = m_properties;

		connect(m_device, &Device::activeEnvironmentChanged, this, &DBusDevice::devActiveEnvironmentChanged);
		connect(m_device, &Device::propertiesChanged, this, &DBusDevice::devPropertiesChanged);
	}

	void DBusDevice::checkPropertiesUpdate() {
		auto &l = m_last_notified_properties;
		auto &c = m_properties;
		auto changed = false;

		if (l.activeEnvironment != c.activeEnvironment) {
			changed = true;
			l.activeEnvironment = c.activeEnvironment;
			emit activeEnvironmentChanged(c.activeEnvironment);
			emit activeEnvironmentChanged(m_activeEnvironment);
		}

		if (l.state != c.state) {
			changed = true;
			l.state = c.state;
			emit stateChanged(c.state);
		}

		if (l != c) {
			changed = true;
			l = c;
		}

		if (changed) emit propertiesChanged(c);
	}

	void DBusDevice::devPropertiesChanged(libnutcommon::DeviceProperties properties) {
		m_properties = properties;
		m_properties.activeEnvironment = m_activeEnvironment >= 0
			? m_dbusEnvironments[m_activeEnvironment]->getPath()
			: OptionalQDBusObjectPath { };
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
		QTimer::singleShot(10, this, &DBusDevice::checkPropertiesUpdate);
#else
		QTimer::singleShot(10, this, SLOT(checkPropertiesUpdate()));
#endif
	}

	void DBusDevice::devActiveEnvironmentChanged(int newEnvironment) {
		m_activeEnvironment = newEnvironment;
		m_properties.activeEnvironment = m_activeEnvironment >= 0
			? m_dbusEnvironments[m_activeEnvironment]->getPath()
			: OptionalQDBusObjectPath { };
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
		QTimer::singleShot(10, this, &DBusDevice::checkPropertiesUpdate);
#else
		QTimer::singleShot(10, this, SLOT(checkPropertiesUpdate()));
#endif
	}

	DeviceProperties DBusDevice::getProperties() {
		return m_properties;
	}

	QString DBusDevice::getName() {
		return m_properties.name;
	}

	DeviceType DBusDevice::getType() {
		return m_properties.type;
	}

	OptionalQDBusObjectPath DBusDevice::getActiveEnvironment() {
		return m_properties.activeEnvironment;
	}

	qint32 DBusDevice::getActiveEnvironmentIndex() {
		return m_activeEnvironment;
	}

	DeviceState DBusDevice::getState() {
		return m_properties.state;
	}

	QString DBusDevice::getEssid() {
		return m_properties.essid;
	}

	MacAddress DBusDevice::getMacAddress() {
		return m_properties.macAddress;
	}

	DeviceConfig DBusDevice::getConfig() {
		return m_device->getConfig();
	}

	QList<QDBusObjectPath> DBusDevice::getEnvironments() {
		QList<QDBusObjectPath> paths;
		for (auto const& dbus_env: m_dbusEnvironments) {
			paths << dbus_env->getPath();
		}
		return paths;
	}

	QList<qint32> DBusDevice::getEnvironmentIds() {
		QList<qint32> envs;
		for (auto const& dbus_env: m_dbusEnvironments) {
			envs << dbus_env->getID();
		}
		return envs;
	}

	void DBusDevice::enable() {
		m_device->enable(true);
	}

	void DBusDevice::disable() {
		m_device->disable();
	}

	void DBusDevice::setEnvironment(QDBusObjectPath path) {
		for (auto const& dbus_env: m_dbusEnvironments) {
			if (dbus_env->getPath() == path) {
				m_device->setUserPreferredEnvironment(dbus_env->getID());
				break;
			}
		}
	}

	void DBusDevice::setEnvironment(qint32 env) {
		m_device->setUserPreferredEnvironment(env);
	}

	static inline QDBusObjectPath makeEnvironmentPath(QDBusObjectPath const& path, int ndx) {
		return QDBusObjectPath(path.path() + "/" + QString::number(ndx));
	}

	DBusEnvironment::DBusEnvironment(Environment* env, const QDBusObjectPath& path)
	: DBusAbstractAdapater(makeEnvironmentPath(path, env->getID()), env), m_environment(env) {
		//Set dbus path an register object
		m_last_notified_properties = m_properties = m_environment->getProperties();

		//Insert interfaces
		for (Interface* interface: m_environment->getInterfaces()) {
			//Check if interface is IPv4 or IPv6
			if (auto ifv4 = dynamic_cast<Interface_IPv4*>(interface)) {
				DBusInterface_IPv4* dbus_interface = new DBusInterface_IPv4(ifv4, m_path);
				m_dbusInterfacesIPv4.append(dbus_interface);
				registerAdaptor(dbus_interface);
			}
#ifdef IPv6
			else if (auto if6 = dynamic_cast<Interface_IPv6*>(interface)) {
				DBusInterface_IPv6* dbus_interface = new DBusInterface_IPv6(ifv6, m_path);
				m_dbusInterfacesIPv6.append(dbus_interface);
				registerAdaptor(dbus_interface);
			}
#endif
		}

		connect(m_environment, &Environment::propertiesChanged, this, &DBusEnvironment::envPropertiesChanged);
	}

	void DBusEnvironment::checkPropertiesUpdate() {
		if (m_last_notified_properties != m_properties) {
			m_last_notified_properties = m_properties;
			emit propertiesChanged(m_properties);
		}
	}

	void DBusEnvironment::envPropertiesChanged(EnvironmentProperties properties) {
		m_properties = properties;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
		QTimer::singleShot(10, this, &DBusEnvironment::checkPropertiesUpdate);
#else
		QTimer::singleShot(10, this, SLOT(checkPropertiesUpdate()));
#endif
	}

	EnvironmentProperties DBusEnvironment::getProperties() {
		return m_properties;
	}

	qint32 DBusEnvironment::getID() {
		return m_properties.id;
	}

	QString DBusEnvironment::getName() {
		return m_properties.name;
	}

	bool DBusEnvironment::isActive() {
		return m_properties.active;
	}

	SelectResult DBusEnvironment::getSelectResult() {
		return m_properties.selectResult;
	}

	QVector<SelectResult> DBusEnvironment::getSelectResults() {
		return m_properties.selectResults;
	}

	EnvironmentConfig DBusEnvironment::getConfig() {
		return m_environment->getConfig();
	}

	QList<QDBusObjectPath> DBusEnvironment::getInterfaces() {
		QList<QDBusObjectPath> paths;
		//Append IPv4 Paths
		for (auto const& interface: m_dbusInterfacesIPv4) {
			paths << interface->getPath();
		}
#ifdef IPv6
		//Append IPv6 Paths
		for (auto const& interface: m_dbusInterfacesIPv6) {
			paths << interface->getPath();
		}
#endif
		return paths;
	}

	QList<qint32> DBusEnvironment::getInterfaceIds() {
		QList<qint32> ifs;
		for (auto const& interface: m_environment->getInterfaces()) {
			ifs << interface->getIndex();
		}
		return ifs;
	}

	static inline QDBusObjectPath makeInterfacePath(QDBusObjectPath const& path, int ndx) {
		return QDBusObjectPath(path.path() + "/" + QString::number(ndx));
	}

	DBusInterface_IPv4::DBusInterface_IPv4(Interface_IPv4* iface, const QDBusObjectPath& path)
	: DBusAbstractAdapater(makeInterfacePath(path, iface->getIndex()), iface), m_interface(iface) {
		m_last_notified_properties = m_properties = m_interface->getProperties();
		m_last_notified_userConfig = m_userConfig = m_interface->getUserConfig();

		connect(m_interface, &Interface_IPv4::propertiesChanged, this, &DBusInterface_IPv4::interfacePropertiesChanged);
		connect(m_interface, &Interface_IPv4::userConfigChanged, this, &DBusInterface_IPv4::interfaceUserConfigChanged);
	}

	void DBusInterface_IPv4::checkPropertiesUpdate() {
		if (m_last_notified_properties != m_properties) {
			m_last_notified_properties = m_properties;
			emit propertiesChanged(m_properties);
		}
	}
	void DBusInterface_IPv4::checkUserConfigUpdate() {
		if (m_last_notified_userConfig != m_userConfig) {
			m_last_notified_userConfig = m_userConfig;
			emit userConfigChanged(m_userConfig);
		}
	}

	void DBusInterface_IPv4::interfacePropertiesChanged(libnutcommon::InterfaceProperties properties) {
		m_properties = properties;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
		QTimer::singleShot(10, this, &DBusInterface_IPv4::checkPropertiesUpdate);
#else
		QTimer::singleShot(10, this, SLOT(checkPropertiesUpdate()));
#endif
	}
	void DBusInterface_IPv4::interfaceUserConfigChanged(libnutcommon::IPv4UserConfig userConfig) {
		m_userConfig = userConfig;
		checkUserConfigUpdate();
	}

	InterfaceProperties DBusInterface_IPv4::getProperties() {
		return m_properties;
	}

	InterfaceState DBusInterface_IPv4::getState() {
		return m_properties.state;
	}

	QHostAddress DBusInterface_IPv4::getIP() {
		return m_properties.ip;
	}

	QHostAddress DBusInterface_IPv4::getNetmask() {
		return m_properties.netmask;
	}

	QHostAddress DBusInterface_IPv4::getGateway() {
		return m_properties.gateway;
	}

	QList<QHostAddress> DBusInterface_IPv4::getDnsServers() {
		return m_properties.dnsServers;
	}

	int DBusInterface_IPv4::getGatewayMetric() {
		return m_properties.gatewayMetric;
	}

	bool DBusInterface_IPv4::needUserSetup() {
		return m_properties.needUserSetup;
	}

	IPv4Config DBusInterface_IPv4::getConfig() {
		return m_interface->getConfig();
	}

	bool DBusInterface_IPv4::setUserConfig(IPv4UserConfig userConfig) {
		return m_interface->setUserConfig(userConfig);
	}

	IPv4UserConfig DBusInterface_IPv4::getUserConfig() {
		return m_userConfig;
	}
}
