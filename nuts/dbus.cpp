#include "dbus.h"
#include "log.h"
#include <QHashIterator>

// #include <QDebug>
#define DBUS_POLL_RATE 5000

namespace nuts {
	const QDBusObjectPath DBusDeviceManager::m_dbusPath("/manager");
	const QDBusObjectPath DBusDeviceManager::m_dbusDevicesPath("/devices");

	DBusDeviceManager::DBusDeviceManager(DeviceManager *devmgr)
	: QDBusAbstractAdaptor(devmgr), m_dbusConnection(QDBusConnection::connectToBus(QDBusConnection::SystemBus, QString::fromLatin1("nuts_system_bus"))), m_devmgr(devmgr), m_timerId(-1), m_dbusMonitor(0),registered(0) {

		initTree();
		registerAll();
		connect(m_devmgr, SIGNAL(deviceAdded(QString, Device*)), SLOT(devAdded(QString, Device*)));
		connect(m_devmgr, SIGNAL(deviceRemoved(QString, Device*)), SLOT(devRemoved(QString, Device*)));

		//Init dbus monitor
		connect(&m_dbusMonitor,SIGNAL(stopped(void)),this,SLOT(dbusStopped(void)));
		connect(&m_dbusMonitor,SIGNAL(started(void)),this,SLOT(dbusStarted(void)));
		m_dbusMonitor.setPidFileDir(DBUS_PID_FILE_DIR);
		m_dbusMonitor.setPidFileName(DBUS_PID_FILE_NAME);
		m_dbusMonitor.setEnabled(true);

		//start dbus connection check timer
		m_timerId = startTimer(DBUS_POLL_RATE);
	}

	void DBusDeviceManager::registerAll() {
		if (!registered) {
			m_dbusConnection.registerService(NUT_DBUS_URL);
			m_dbusConnection.registerObject(m_dbusPath.path(), m_devmgr);
			registered = true;
		}
		QHash<QString, nuts::DBusDevice *>::iterator dbusdev = m_dbusDevices.begin();
		while (dbusdev != m_dbusDevices.end()) {
			(*dbusdev)->registerAll();
			dbusdev++;
		}
	}

	void DBusDeviceManager::unregisterAll() {
		QHash<QString, nuts::DBusDevice *>::iterator dbusdev = m_dbusDevices.begin();
		while (dbusdev != m_dbusDevices.end()) {
			(*dbusdev)->unregisterAll();
			dbusdev++;
		}
		//unregister self
		if (registered) {
			m_dbusConnection.unregisterObject(m_dbusPath.path(), QDBusConnection::UnregisterTree);
			m_dbusConnection.unregisterService(NUT_DBUS_URL);
			registered = false;
		}
	}

	void DBusDeviceManager::setAllDBusConnection(QDBusConnection * connection) {
		QHash<QString, nuts::DBusDevice *>::iterator dbusdev = m_dbusDevices.begin();
		while (dbusdev != m_dbusDevices.end()) {
			(*dbusdev)->setAllDBusConnection(connection);
			dbusdev++;
		}
	}

	void DBusDeviceManager::initTree() {
		//Insert devices into devices Hash.
		QHashIterator<QString, Device *> i(m_devmgr->getDevices());
		while (i.hasNext()) {
			i.next();
			devAdded(i.key(), i.value());
		}
	}

	void DBusDeviceManager::stopDBus() {
		unregisterAll();
		m_dbusConnection.disconnectFromBus(QString::fromLatin1("nuts_system_bus"));
	}

	void DBusDeviceManager::dbusStopped() {
		stopDBus();
	}

	void DBusDeviceManager::dbusStarted() {
		//call stopDBus to clear all information
		stopDBus();
		m_dbusConnection = QDBusConnection::connectToBus(QDBusConnection::SystemBus, QString::fromLatin1("nuts_system_bus"));
		setAllDBusConnection(&m_dbusConnection);
		registerAll();

		//Restart dbus connection check timer
		if (-1 == m_timerId)
			m_timerId = startTimer(DBUS_POLL_RATE);
	}

	DBusDeviceManager::~DBusDeviceManager() {
		if (registered) {
			m_dbusConnection.unregisterObject(m_dbusPath.path(), QDBusConnection::UnregisterTree);
			m_dbusConnection.unregisterService(NUT_DBUS_URL);
		}
	}

	void DBusDeviceManager::timerEvent(QTimerEvent *event) {
		if ( event->timerId() == m_timerId ) {
			if (!m_dbusConnection.isConnected()) {
				killTimer(m_timerId);
				m_timerId = -1;
				//DBusMonitor will start dbus
				log << "dbus has been stopped, please restart it";
				dbusStopped();
			}
		}
	}

	//SLOT: Inserts device into device hash
	void DBusDeviceManager::devAdded(QString devName, Device *dev) {
		DBusDevice *dbus_device;
		if (!m_dbusDevices.contains(devName)) {
			dbus_device = new DBusDevice(dev, &m_dbusConnection, m_dbusDevicesPath.path());
			m_dbusDevices.insert(devName, dbus_device);
			dbus_device->registerAll();
		}
		else {
			dbus_device = m_dbusDevices.value(devName);
		}
		emit deviceAdded(dbus_device->getPath());
		emit deviceAdded(devName);
	}


	void DBusDeviceManager::devRemoved(QString devName, Device* /* dev */) {
		if (m_dbusDevices.contains(devName)) {
			DBusDevice * dbus_device = m_dbusDevices[devName];
			emit deviceRemoved(dbus_device->getPath());
			emit deviceRemoved(devName);
			m_dbusDevices.remove(devName);
		}
		// dbus_device is deleted as child of dev
	}

	QList<QDBusObjectPath> DBusDeviceManager::getDeviceList() {
		QList<QDBusObjectPath> paths;
		foreach (DBusDevice *dbus_device, m_dbusDevices) {
			paths.append(dbus_device->getPath());
		}
		return paths;
	}

	QStringList DBusDeviceManager::getDeviceNames() {
		QStringList names;
		foreach (nuts::Device* dev, m_devmgr->getDevices()) {
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

	DBusDevice::DBusDevice(Device *dev, QDBusConnection *connection, const QString &path)
	: QDBusAbstractAdaptor(dev), m_device(dev), m_dbusConnection(connection), registered(false) {

		//Set dbus device path an register objects
		m_dbusPath = QDBusObjectPath(path + '/' + dev->getName());

		// remember active environment
		m_activeEnvironment = m_device->getEnvironment();

		//Add Environments
		foreach (Environment *env, m_device->getEnvironments()) {
			DBusEnvironment *dbus_env = new DBusEnvironment(env, m_dbusConnection, m_dbusPath, m_device);
			m_dbusEnvironments.append(dbus_env);
		}

		connect(m_device,SIGNAL(stateChanged(libnutcommon::DeviceState, libnutcommon::DeviceState, Device*)),this,SLOT(devStateChanged(libnutcommon::DeviceState, libnutcommon::DeviceState)));
		setAutoRelaySignals(true);
		connect(m_device,SIGNAL(environmentChanged(int)),this,SLOT(devEnvironmentChanged(int)));
	}

	DBusDevice::~DBusDevice() {
		if (registered) {
			m_dbusConnection->unregisterObject(m_dbusPath.path(), QDBusConnection::UnregisterTree);
		}
	}

	QDBusObjectPath DBusDevice::getPath() {
		return m_dbusPath;
	}

	void DBusDevice::registerAll() {
		if (!registered && m_dbusConnection->isConnected()) {
			m_dbusConnection->registerObject(m_dbusPath.path(), m_device);
			registered = true;
		}
		foreach(DBusEnvironment * dbusenv, m_dbusEnvironments) {
			dbusenv->registerAll();
		}
	}

	void DBusDevice::unregisterAll() {
		foreach(DBusEnvironment * dbusenv, m_dbusEnvironments) {
			dbusenv->unregisterAll();
		}
		if (registered) {
			m_dbusConnection->unregisterObject(m_dbusPath.path(), QDBusConnection::UnregisterTree);
			registered = false;
		}
	}

	void DBusDevice::setAllDBusConnection(QDBusConnection * connection) {
		m_dbusConnection = connection;
		foreach(DBusEnvironment * dbusenv, m_dbusEnvironments) {
			dbusenv->setAllDBusConnection(connection);
		}
	}

	void DBusDevice::devStateChanged(libnutcommon::DeviceState newState, libnutcommon::DeviceState oldState) {
		//Check if active environment has changed:
		if (m_activeEnvironment != m_device->getEnvironment() ) {
			int oldActive = m_activeEnvironment;
			m_activeEnvironment = m_device->getEnvironment();
			if (0 <= oldActive) {
				emit(m_dbusEnvironments[oldActive]->emitChange(false));
			}
			if (0 <= m_activeEnvironment) {
				emit(m_dbusEnvironments[m_activeEnvironment]->emitChange(true));
			}
			//active Environment has changed for device:
			if (0 > m_activeEnvironment) {
				emit( environmentChangedActive({}) );
			}
			else {
				emit( environmentChangedActive( m_dbusEnvironments[m_activeEnvironment]->getPath() ) );
			}
		}
		emit(stateChanged(newState, oldState));
	}
	void DBusDevice::devEnvironmentChanged(int newEnvironment) {
		//Check if active environment has changed:
		if (m_activeEnvironment != newEnvironment ) {
			int oldActive = m_activeEnvironment;
			m_activeEnvironment = newEnvironment;
			if (0 <= oldActive) {
				emit(m_dbusEnvironments[oldActive]->emitChange(false));
			}
			if (0 <= m_activeEnvironment) {
				emit(m_dbusEnvironments[m_activeEnvironment]->emitChange(true));
			}
			//active Environment has changed for device:
			if (0 > m_activeEnvironment) {
				emit( environmentChangedActive({}) );
			}
			else {
				emit( environmentChangedActive( m_dbusEnvironments[m_activeEnvironment]->getPath() ) );
			}
			//emit generice environmentChangedActive
			emit environmentChangedActive(newEnvironment);
		}
	}

	void DBusDevice::enable() {
		m_device->enable(true);
	}

	void DBusDevice::disable() {
		m_device->disable();
	}

	void DBusDevice::setEnvironment(const QDBusObjectPath &path) {
		foreach(DBusEnvironment * i, m_dbusEnvironments) {
			if (i->getPath() == path) {
				m_device->setUserPreferredEnvironment(i->getEnvironment()->getID());
				break;
			}
		}
	}

	void DBusDevice::setEnvironment(qint32 env) {
		m_device->setUserPreferredEnvironment(env);
	}

	libnutcommon::DeviceProperties DBusDevice::getProperties() {
		auto props = libnutcommon::DeviceProperties();

		props.name = m_device->getName();
		props.state = m_device->getState();
		props.type = m_device->hasWLAN() ? libnutcommon::DeviceType::AIR : libnutcommon::DeviceType::ETH;
		int envNdx = m_device->getEnvironment();
		if (envNdx >= 0) props.activeEnvironment = libnutcommon::OptionalQDBusObjectPath{m_dbusEnvironments[envNdx]->getPath()};

		return props;
	}

	QList<QDBusObjectPath> DBusDevice::getEnvironments() {
		QList<QDBusObjectPath> paths;
		foreach (DBusEnvironment *dbus_env, m_dbusEnvironments) {
			paths.append(dbus_env->getPath());
		}
		return paths;
	}

	QList<qint32> DBusDevice::getEnvironmentIds() {
		QList<qint32> envs;
		foreach (nuts::Environment* env, m_device->getEnvironments()) {
			envs.append(env->getID());
		}
		return envs;
	}

	libnutcommon::DeviceConfig DBusDevice::getConfig() {
		return (m_device->getConfig());
	}

	libnutcommon::DeviceState DBusDevice::getState() {
		return m_device->getState();
	}

	libnutcommon::DeviceType DBusDevice::getType() {
		return m_device->hasWLAN() ? libnutcommon::DeviceType::AIR : libnutcommon::DeviceType::ETH;
	}

	libnutcommon::OptionalQDBusObjectPath DBusDevice::getActiveEnvironment() {
		int m_activeEnvironment = m_device->getEnvironment();
		if (m_activeEnvironment >= 0) {
			return libnutcommon::OptionalQDBusObjectPath{m_dbusEnvironments[m_activeEnvironment]->getPath()};
		}
		else {
			return { };
		}
	}

	qint32 DBusDevice::getActiveEnvironmentIndex() {
		return m_device->getEnvironment();
	}

	QString DBusDevice::getEssid() {
		return m_device->essid();
	}


	DBusEnvironment::DBusEnvironment(Environment *env, QDBusConnection *connection, const QDBusObjectPath &path, Device * dev)
	: QDBusAbstractAdaptor(env), m_environment(env), m_dbusConnection(connection), m_device(dev), registered(false) {
		//Set dbus path an register object
		m_dbusPath = QDBusObjectPath(path.path() + QString("/%1").arg(getID()));

		//Insert interfaces
		foreach (Interface *interface, m_environment->getInterfaces()) {
			//Check if interface is IPv4 or IPv6
			Interface_IPv4 *ifv4 = dynamic_cast<Interface_IPv4*>(interface);
			if (ifv4) {
				DBusInterface_IPv4 *dbus_interface = new DBusInterface_IPv4(ifv4, m_dbusConnection, m_dbusPath);
				m_dbusInterfacesIPv4.append(dbus_interface);
				emit(interfaceAdded(dbus_interface->getPath()));
			}
			#ifdef IPv6
			else {
				Interface_IPv6 *if6 = dynamic_cast<Interface_IPv6*>(interface);
				if (if6) {
					DBusInterface_IPv6 *dbus_interface = new DBusInterface_IPv6(ifv6, m_dbusConnection, m_dbusPath);
					m_dbusInterfacesIPv6.append(dbus_interface);
					emit(interfaceAdded(dbus_interface->getPath()));
				}
			}
			#endif
		}
	}

	DBusEnvironment::~DBusEnvironment() {
		if (registered) {
			m_dbusConnection->unregisterObject(m_dbusPath.path(), QDBusConnection::UnregisterTree);
		}
	}

	QDBusObjectPath DBusEnvironment::getPath() {
		return m_dbusPath;
	}

	void DBusEnvironment::registerAll() {
		if (!registered && m_dbusConnection->isConnected()) {
			m_dbusConnection->registerObject(m_dbusPath.path(), m_environment);
			registered = true;
		}
		foreach(DBusInterface_IPv4 * ifs, m_dbusInterfacesIPv4) {
			ifs->registerAll();
		}
		#ifdef IPv6
		foreach(DBusInterface_IPv6 * ifs, m_dbusInterfacesIPv6) {
			ifs->registerAll();
		}
		#endif
	}

	void DBusEnvironment::unregisterAll() {
		foreach(DBusInterface_IPv4 * ifs, m_dbusInterfacesIPv4) {
			ifs->unregisterAll();
		}
		#ifdef IPv6
		foreach(DBusInterface_IPv6 * ifs, m_dbusInterfacesIPv6) {
			ifs->unregisterAll();
		}
		#endif
		if (registered) {
			m_dbusConnection->unregisterObject(m_dbusPath.path(), QDBusConnection::UnregisterTree);
			registered = false;
		}
	}

	void DBusEnvironment::setAllDBusConnection(QDBusConnection * connection) {
		m_dbusConnection = connection;
		foreach(DBusInterface_IPv4 * ifs, m_dbusInterfacesIPv4) {
			ifs->setAllDBusConnection(connection);
		}
		#ifdef IPv6
		foreach(DBusInterface_IPv6 * ifs, m_dbusInterfacesIPv6) {
			ifs->setAllDBusConnection(connection);
		}
		#endif
	}

	void DBusEnvironment::selectResultReady() {
		emit selectsResultChanged(m_environment->getSelectResult(),m_environment->getSelectResults());
	}

	//Function for device to emit a statechange of an environment
	void DBusEnvironment::emitChange(bool change) {
		emit stateChanged(change);
	}

	libnutcommon::EnvironmentProperties DBusEnvironment::getProperties() {
		auto props = libnutcommon::EnvironmentProperties();
		props.name = m_environment->getName();
		props.active = (m_device->getEnvironment() == getID());
		return props;
	}

	libnutcommon::EnvironmentConfig DBusEnvironment::getConfig() {
		return m_environment->getConfig();
	}

	QList<QDBusObjectPath> DBusEnvironment::getInterfaces() {
		QList<QDBusObjectPath> paths;
		//Append IPv4 Paths
		foreach (DBusInterface_IPv4 *i, m_dbusInterfacesIPv4) {
			paths.append(i->getPath());
		}
		#ifdef IPv6
		//Append IPv6 Paths
		foreach (DBusInterface_IPv6 *i, m_dbusInterfacesIPv6) {
			paths.append(i->getPath());
		}
		#endif
		return paths;
	}

	QList<qint32> DBusEnvironment::getInterfaceIds() {
		QList<qint32> ifs;
		foreach(nuts::Interface *i, m_environment->getInterfaces()) {
			ifs.append(i->getIndex());
		}
		return ifs;
	}

	libnutcommon::SelectResult DBusEnvironment::getSelectResult() {
		return m_environment->getSelectResult();
	}

	QVector<libnutcommon::SelectResult> DBusEnvironment::getSelectResults() {
		return m_environment->getSelectResults();
	}

	qint32 DBusEnvironment::getID() {
		return m_environment->getID();
	}

	QString DBusEnvironment::getName() {
		return m_environment->getName();
	}

	bool DBusEnvironment::getState() {
		return (m_device->getEnvironment() == getID());
	}


	DBusInterface_IPv4::DBusInterface_IPv4(Interface_IPv4 *iface, QDBusConnection *connection, const QDBusObjectPath &path)
	: QDBusAbstractAdaptor(iface), m_interface(iface), m_dbusConnection(connection), registered(false) {
		m_dbusPath = QDBusObjectPath(path.path() + QString("/%1").arg(m_interface->getIndex()));
		connect(m_interface,SIGNAL(statusChanged(libnutcommon::InterfaceState, Interface_IPv4*)),SLOT(interfaceStatusChanged(libnutcommon::InterfaceState)));
	}

	DBusInterface_IPv4::~DBusInterface_IPv4() {
		m_dbusConnection->unregisterObject(m_dbusPath.path(), QDBusConnection::UnregisterTree);
	}

	void DBusInterface_IPv4::registerAll() {
		if (!registered && m_dbusConnection->isConnected()) {
			m_dbusConnection->registerObject(m_dbusPath.path(), m_interface);
			registered = true;
		}
	}

	void DBusInterface_IPv4::unregisterAll() {
		if (registered) {
			m_dbusConnection->unregisterObject(m_dbusPath.path(), QDBusConnection::UnregisterTree);
			registered = false;
		}
	}

	QDBusObjectPath DBusInterface_IPv4::getPath() {
		return m_dbusPath;
	}

	void DBusInterface_IPv4::interfaceStatusChanged(libnutcommon::InterfaceState) {
		emit stateChanged(getProperties());
	}

	libnutcommon::InterfaceProperties DBusInterface_IPv4::getProperties() {
		auto props = libnutcommon::InterfaceProperties();
		props.ifState = m_interface->getState();
		props.ip = m_interface->ip;
		props.netmask = m_interface->netmask;
		props.gateway = m_interface->gateway;
		props.dns = m_interface->dnsserver;
		props.gateway_metric = m_interface->gatewayMetric();
		return props;
	}

	libnutcommon::IPv4Config DBusInterface_IPv4::getConfig() {
		return (m_interface->getConfig());
	}

	bool DBusInterface_IPv4::needUserSetup() {
		return m_interface->needUserSetup();
	}

	bool DBusInterface_IPv4::setUserConfig(libnutcommon::IPv4UserConfig userConfig) {
		return m_interface->setUserConfig(userConfig);
	}

	libnutcommon::IPv4UserConfig DBusInterface_IPv4::getUserConfig() {
		return m_interface->getUserConfig();
	}

	libnutcommon::InterfaceState DBusInterface_IPv4::getState() {
		return m_interface->getState();
	}

	QHostAddress DBusInterface_IPv4::getIP() {
		return m_interface->ip;
	}

	QHostAddress DBusInterface_IPv4::getNetmask() {
		return m_interface->netmask;
	}

	QHostAddress DBusInterface_IPv4::getGateway() {
		return m_interface->gateway;
	}

	QList<QHostAddress> DBusInterface_IPv4::getDns() {
		return m_interface->dnsserver;
	}
}
