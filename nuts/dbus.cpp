#include "dbus.h"
#include "log.h"
#include <QHashIterator>

// #include <QDebug>

namespace nuts {
	const QString DBusDeviceManager::m_dbusPath("/manager");
	const QString DBusDeviceManager::m_dbusDevicesPath("/devices");

	DBusDeviceManager::DBusDeviceManager(DeviceManager *devmgr)
	: QDBusAbstractAdaptor(devmgr), m_dbusConnection(QDBusConnection::connectToBus(QDBusConnection::SystemBus, QString::fromLatin1("nuts_system_bus"))), m_devmgr(devmgr), m_timerId(-1), m_dbusMonitor(0) {
		startDBus();

		//Init dbus monitor
		connect(&m_dbusMonitor,SIGNAL(stopped(void)),this,SLOT(dbusStopped(void)));
		connect(&m_dbusMonitor,SIGNAL(started(void)),this,SLOT(dbusStarted(void)));
		m_dbusMonitor.setPidFileDir(DBUS_PID_FILE_DIR);
		m_dbusMonitor.setPidFileName(DBUS_PID_FILE_NAME);
		m_dbusMonitor.setEnabled(true);

	}
	void DBusDeviceManager::startDBus() {
		//Check if connection to dbus exists
		if ( !m_dbusConnection.isConnected() ) {
			log << "Please start dbus.";
			log << "Otherwise you will not be able to control nuts" << endl;
			return;
		}
		//Register Service and device manager object
		m_dbusConnection.registerService(NUT_DBUS_URL);
		m_dbusConnection.registerObject(m_dbusPath, m_devmgr);
		//Insert devices into devices Hash.
		QHashIterator<QString, Device *> i(m_devmgr->getDevices());
		while (i.hasNext()) {
			i.next();
			devAdded(i.key(), i.value());
		}
		connect(m_devmgr, SIGNAL(deviceAdded(QString, Device*)), SLOT(devAdded(QString, Device*)));
		connect(m_devmgr, SIGNAL(deviceRemoved(QString, Device*)), SLOT(devRemoved(QString, Device*)));
	}

	void DBusDeviceManager::stopDBus() {
		//kill timers:
		if (-1 != m_timerId) {
			killTimer(m_timerId);
		}
		//clear all informations
		QHash<QString, nuts::DBusDevice *>::iterator dev = m_dbusDevices.begin();
		DBusDevice * dbusdev;
		while (dev != m_dbusDevices.end()) {
			dbusdev = *dev;
			dev = m_dbusDevices.erase(dev);
			dbusdev->dbusStopped();
			delete dbusdev;
		}

		m_dbusConnection.unregisterObject(m_dbusPath, QDBusConnection::UnregisterTree);
		m_dbusConnection.unregisterService(NUT_DBUS_URL);
		m_dbusConnection.disconnectFromBus(QString::fromLatin1("nuts_system_bus"));
// 		if (m_dbusConnection.isConnected()) {
// 			qDebug() << "(BIG FAT WARNING) Dbus is connected after disconnect";
// 		}
// 		qDebug() << "Deleted dev-manager";
	}

	void DBusDeviceManager::dbusStopped() {
		stopDBus();
	}
	
	void DBusDeviceManager::dbusStarted() {
		//call stopDBus to clear all information
		stopDBus();
		m_dbusConnection = QDBusConnection(QDBusConnection::connectToBus(QDBusConnection::SystemBus, QString::fromLatin1("nuts_system_bus")));
		startDBus();
	}

	DBusDeviceManager::~DBusDeviceManager() {
		m_dbusConnection.unregisterObject(m_dbusPath, QDBusConnection::UnregisterTree);
		m_dbusConnection.unregisterService(NUT_DBUS_URL);
	}
	
	void DBusDeviceManager::timerEvent(QTimerEvent *event) {
		if ( event->timerId() == m_timerId ) {
			killTimer(m_timerId);
			m_timerId = -1;
			//try to connect to dbus
			m_dbusConnection.disconnectFromBus(QString::fromLatin1("nuts_system_bus"));
			m_dbusConnection = QDBusConnection(QDBusConnection::connectToBus(QDBusConnection::SystemBus, QString::fromLatin1("nuts_system_bus")));
			startDBus();
		}
	}
	
	//SLOT: Inserts device into device hash
	void DBusDeviceManager::devAdded(QString devName, Device *dev) {
		DBusDevice *dbus_device = new DBusDevice(dev, &m_dbusConnection, m_dbusDevicesPath);
		m_dbusDevices.insert(devName, dbus_device);
		emit deviceAdded(QDBusObjectPath(dbus_device->getPath()));
		emit deviceAdded(devName);
	}
	
	
	void DBusDeviceManager::devRemoved(QString devName, Device* /* *dev */) {
		DBusDevice *dbus_device = m_dbusDevices[devName];
		emit deviceRemoved(QDBusObjectPath(dbus_device->getPath()));
		emit deviceRemoved(devName);
		m_dbusDevices.remove(devName);
		// dbus_device is deleted as child of dev
	}
	
	QList<QDBusObjectPath> DBusDeviceManager::getDeviceList() {
		QList<QDBusObjectPath> paths;
		foreach (DBusDevice *dbus_device, m_dbusDevices) {
			paths.append(QDBusObjectPath(dbus_device->getPath()));
		}
// 		qDebug() << "Returining device list";
		return paths;
	}
	QStringList DBusDeviceManager::getDeviceNames() {
		QStringList names;
		foreach (nuts::Device* dev, m_devmgr->getDevices()) {
			names.append(dev->getName());
		}
		return names;
	}

	bool DBusDeviceManager::createBridge(QString name) { return false; }
	bool DBusDeviceManager::destroyBridge(QDBusObjectPath devicePath) { return false; }
	bool DBusDeviceManager::destroyBridge(qint32 deviceId) { return false; }
	bool DBusDeviceManager::addToBridge(QDBusObjectPath bridge, QList<QDBusObjectPath> devicePaths) { return false; }
	bool DBusDeviceManager::addToBridge(qint32 bridgeId, QList<qint32> deviceIds) { return false; }
	bool DBusDeviceManager::removeFromBridge(QDBusObjectPath bridge, QList<QDBusObjectPath> devicePaths) { return false; }
	bool DBusDeviceManager::removeFromBridge(qint32 bridgeId, QList<qint32> deviceIds) { return false; }


	DBusDevice::DBusDevice(Device *dev, QDBusConnection *connection, const QString &path)
	: QDBusAbstractAdaptor(dev), m_device(dev), m_dbusConnection(connection) {

		//Set Device Properties
		m_dbusProperties.type = m_device->hasWLAN() ? libnutcommon::DT_AIR : libnutcommon::DT_ETH;
		m_dbusProperties.name = m_device->getName();
		m_dbusProperties.state = m_device->getState();

		//Set dbus device path an register objects
		m_dbusPath = path + '/' + dev->getName();
		m_dbusConnection->registerObject(m_dbusPath, m_device);
		
		//Add Environments
		foreach (Environment *env, m_device->getEnvironments()) {
			DBusEnvironment *dbus_env = new DBusEnvironment(env, m_dbusConnection, m_dbusPath,m_device);
			m_dbusEnvironments.append(dbus_env);
		}

		//Set active Environment
		m_activeEnvironment = m_device->getEnvironment();
		if (m_activeEnvironment >= 0) {
			m_dbusProperties.activeEnvironment = m_dbusEnvironments[m_activeEnvironment]->getPath();
		}
		else {
			m_dbusProperties.activeEnvironment = "";
		}
		connect(m_device,SIGNAL(stateChanged(libnutcommon::DeviceState, libnutcommon::DeviceState, Device*)),this,SLOT(stateChanged(libnutcommon::DeviceState, libnutcommon::DeviceState)));
		setAutoRelaySignals(true);
		connect(m_device,SIGNAL(environmentChanged(int)),this,SLOT(environmentChanged(int)));
	}
	
	DBusDevice::~DBusDevice() {
		m_dbusEnvironments.clear();
		m_dbusConnection->unregisterObject(m_dbusPath, QDBusConnection::UnregisterTree);
// 		qDebug() << "Deleted device";
	}
	
	QString DBusDevice::getPath() {
		return m_dbusPath;
	}

	void DBusDevice::dbusStopped() {
		DBusEnvironment * env;
		while (!m_dbusEnvironments.isEmpty()) {
			env = m_dbusEnvironments.takeFirst();
			env->dbusStopped();
			delete env;
		}
	}

	void DBusDevice::stateChanged(libnutcommon::DeviceState newState, libnutcommon::DeviceState oldState) {
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
				emit( environmentChangedActive(QString()) );
			}
			else {
				emit( environmentChangedActive( m_dbusEnvironments[m_activeEnvironment]->getPath() ) );
			}
		}
		emit(stateChanged((int) newState, (int) oldState));
	}
	void DBusDevice::environmentChanged(int newEnvironment) {
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
				emit( environmentChangedActive(QString()) );
			}
			else {
				emit( environmentChangedActive( m_dbusEnvironments[m_activeEnvironment]->getPath() ) );
			}
			//emit generice environmentChangedActive
			emit environmentChangedActive(newEnvironment);
		}
	}

	libnutcommon::DeviceProperties DBusDevice::getProperties() {
		m_dbusProperties.state = m_device->getState();
		m_dbusProperties.type = m_device->hasWLAN() ? libnutcommon::DT_AIR : libnutcommon::DT_ETH;
		int m_activeEnvironment = m_device->getEnvironment();
		if (m_activeEnvironment >= 0) {
			m_dbusProperties.activeEnvironment = m_dbusEnvironments[m_activeEnvironment]->getPath();
		}
		else {
			m_dbusProperties.activeEnvironment = QString();
		}
		return m_dbusProperties;
	}
	
	QList<QDBusObjectPath> DBusDevice::getEnvironments() {
		QList<QDBusObjectPath> paths;
		foreach (DBusEnvironment *dbus_env, m_dbusEnvironments) {
			paths.append(QDBusObjectPath(dbus_env->getPath()));
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

	void DBusDevice::setEnvironment(const QDBusObjectPath &path) {
		foreach(DBusEnvironment * i, m_dbusEnvironments) {
			if (i->getPath() == path.path()) {
				m_device->setUserPreferredEnvironment((i->getEnvironment())->getID());
				break;
			}
		}
	}
	QString DBusDevice::getActiveEnvironment() {
		int m_activeEnvironment = m_device->getEnvironment();
		if (m_activeEnvironment >= 0) {
			return m_dbusEnvironments[m_activeEnvironment]->getPath();
		}
		else {
			return m_dbusProperties.activeEnvironment = QString();
		}
	}
	
	void DBusDevice::enable() {
		m_device->enable(true);
	}

	void DBusDevice::disable() {
		m_device->disable();
	}

	DBusEnvironment::DBusEnvironment(Environment *env, QDBusConnection *connection, const QString &path, Device * dev)
	: QDBusAbstractAdaptor(env), m_environment(env), m_dbusConnection(connection), m_device(dev) {
		//Set dbus path an register object
		m_dbusPath = path + QString("/%1").arg(m_environment->getID());
		m_dbusConnection->registerObject(m_dbusPath, m_environment);
		
		//Insert interfaces
		foreach (Interface *interface, m_environment->getInterfaces()) {
			//Check if interface is IPv4 or IPv6
			Interface_IPv4 *ifv4 = dynamic_cast<Interface_IPv4*>(interface);
			if (ifv4) {
				DBusInterface_IPv4 *dbus_interface = new DBusInterface_IPv4(ifv4, m_dbusConnection, m_dbusPath);
				m_dbusInterfacesIPv4.append(dbus_interface);
				emit(interfaceAdded(QDBusObjectPath(dbus_interface->getPath())));
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
		//deleted as child of Interface
		m_dbusInterfacesIPv4.clear();
		m_dbusConnection->unregisterObject(m_dbusPath);
// 		qDebug() << "deleted environment";
	}
	
	QString DBusEnvironment::getPath() {
		return m_dbusPath;
	}
	void DBusEnvironment::dbusStopped() {
		DBusInterface_IPv4 * ifs;
		while (! m_dbusInterfacesIPv4.isEmpty()) {
			ifs = m_dbusInterfacesIPv4.takeFirst();
			delete ifs;
		}
		#ifdef IPv6
		DBusInterface_IPv6 * ifs6;
		while (! m_dbusInterfacesIPv6.isEmpty()) {
			ifs6 = m_dbusInterfacesIPv6.takeFirst();
			delete ifs6;
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
		m_dbusProperties.name = m_environment->getName();
		m_dbusProperties.active = (m_device->getEnvironment() == m_environment->getID());
		return m_dbusProperties;
	}
	libnutcommon::EnvironmentConfig DBusEnvironment::getConfig() {
		return (m_environment->getConfig());
	}
	
	QList<QDBusObjectPath> DBusEnvironment::getInterfaces() {
		QList<QDBusObjectPath> paths;
		//Append IPv4 Paths
		foreach (DBusInterface_IPv4 *i, m_dbusInterfacesIPv4) {
			paths.append(QDBusObjectPath(i->getPath()));
		}
		#ifdef IPv6
		//Append IPv6 Paths
		foreach (DBusInterface_IPv6 *i, m_dbusInterfacesIPv6) {
			paths.append(QDBusObjectPath(i->getPath()));
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


	DBusInterface_IPv4::DBusInterface_IPv4(Interface_IPv4 *iface, QDBusConnection *connection, const QString &path)
	: QDBusAbstractAdaptor(iface), m_interface(iface), m_dbusConnection(connection) {
		m_dbusPath = path + QString("/%1").arg(m_interface->getIndex());
		m_dbusConnection->registerObject(m_dbusPath, m_interface);
		//Set Interface properties
		m_dbusProperties.ip = m_interface->ip;
		m_dbusProperties.gateway = m_interface->gateway;
		m_dbusProperties.netmask = m_interface->netmask;
		m_dbusProperties.ifState = m_interface->getState();
		
		if (!m_interface->dnsserver.isEmpty()) {
			m_dbusProperties.dns = m_interface->dnsserver;
		}
		else {
			m_dbusProperties.dns = QList<QHostAddress>();
		}
		connect(m_interface,SIGNAL(statusChanged(libnutcommon::InterfaceState, Interface_IPv4*)),SLOT(interfaceStatusChanged(libnutcommon::InterfaceState)));
	}
	
	DBusInterface_IPv4::~DBusInterface_IPv4() {
// 		qDebug() << "deleted interface";
	}

	QString DBusInterface_IPv4::getPath() {
		return m_dbusPath;
	}
	//Private SLOTS:

	void DBusInterface_IPv4::interfaceStatusChanged(libnutcommon::InterfaceState) {
		m_dbusProperties.ip = m_interface->ip;
		m_dbusProperties.gateway = m_interface->gateway;
		m_dbusProperties.netmask = m_interface->netmask;
		m_dbusProperties.ifState = m_interface->getState();
		m_dbusProperties.dns = m_interface->dnsserver;

		emit stateChanged(m_dbusProperties);
	}

	libnutcommon::InterfaceProperties DBusInterface_IPv4::getProperties() {
		m_dbusProperties.ip = m_interface->ip;
		m_dbusProperties.gateway = m_interface->gateway;
		m_dbusProperties.netmask = m_interface->netmask;
		m_dbusProperties.ifState = m_interface->getState();
		m_dbusProperties.dns = m_interface->dnsserver;
		return m_dbusProperties;
	}
	libnutcommon::IPv4Config DBusInterface_IPv4::getConfig() {
		return (m_interface->getConfig());
	}

}
