//
// C++ Implementation: dbus
//
// Description: 
//
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "dbus.h"
#include "log.h"
#include <QHashIterator>

namespace nuts {
	const QString DBusDeviceManager::m_dbusPath("/manager");
	const QString DBusDeviceManager::m_dbusDevicesPath("/devices");

	DBusDeviceManager::DBusDeviceManager(DeviceManager *devmgr)
	: QDBusAbstractAdaptor(devmgr), m_dbusConnection(QDBusConnection::systemBus()), m_devmgr(devmgr) {
		//Register Service and device manager object
		m_dbusConnection.registerService(NUT_DBUS_URL);
		m_dbusConnection.registerObject(m_dbusPath, devmgr);
		//Insert devices into devices Hash.
		QHashIterator<QString, Device *> i(m_devmgr->getDevices());
		while (i.hasNext()) {
			i.next();
			devAdded(i.key(), i.value());
		}
		connect(devmgr, SIGNAL(deviceAdded(QString, Device*)), SLOT(devAdded(QString, Device*)));
		connect(devmgr, SIGNAL(deviceRemoved(QString, Device*)), SLOT(devRemoved(QString, Device*)));
	}

	DBusDeviceManager::~DBusDeviceManager() {
		m_dbusConnection.unregisterObject(m_dbusPath);
		m_dbusConnection.unregisterService(NUT_DBUS_URL);
	}
	
	void DBusDeviceManager::stopDBus() {
		m_dbusConnection.unregisterService(NUT_DBUS_URL);
		m_dbusConnection.unregisterObject(m_dbusPath);
	}
	
	//SLOT: Inserts device into device hash
	void DBusDeviceManager::devAdded(QString devName, Device *dev) {
		DBusDevice *dbus_device = new DBusDevice(dev, &m_dbusConnection, m_dbusDevicesPath);
		m_dbusDevices.insert(devName, dbus_device);
		emit deviceAdded(QDBusObjectPath(dbus_device->getPath()));
	}
	
	
	void DBusDeviceManager::devRemoved(QString devName, Device* /* *dev */) {
		DBusDevice *dbus_device = m_dbusDevices[devName];
		emit deviceRemoved(QDBusObjectPath(dbus_device->getPath()));
		m_dbusDevices.remove(devName);
		// dbus_device is deleted as child of dev
	}
	
	QList<QDBusObjectPath> DBusDeviceManager::getDeviceList() {
		QList<QDBusObjectPath> paths;
		foreach (DBusDevice *dbus_device, m_dbusDevices) {
			paths.append(QDBusObjectPath(dbus_device->getPath()));
		}
		return paths;
	}

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
		m_dbusConnection->unregisterObject(m_dbusPath, QDBusConnection::UnregisterTree);
	}
	
	QString DBusDevice::getPath() {
		return m_dbusPath;
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
		}
	}

	libnutcommon::DeviceProperties DBusDevice::getProperties() {
		m_dbusProperties.state = m_device->getState();
		m_dbusProperties.type = m_device->hasWLAN() ? libnutcommon::DT_AIR : libnutcommon::DT_ETH;
		int m_activeEnvironment = m_device->getEnvironment();
		if (m_activeEnvironment >= 0) {
			m_dbusProperties.activeEnvironment = m_dbusEnvironments[m_activeEnvironment]->getPath();
		} else {
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
		m_dbusConnection->unregisterObject(m_dbusPath);
	}
	
	QString DBusEnvironment::getPath() {
		return m_dbusPath;
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
	libnutcommon::SelectResult DBusEnvironment::getSelectResult() {
// 		if (m_environment->selectionDone()) {
// 			return m_environment->getSelectResult();
// 		}
// 		else {
// 			return libnutcommon::SelectResult();
// 		}
		//Workaround so far, as qnut crashes if SelectResults are empty
		return m_environment->getSelectResult();
	}
	QVector<libnutcommon::SelectResult> DBusEnvironment::getSelectResults() {
// 		if (m_environment->selectionDone()) {
// 			return m_environment->getSelectResults();
// 		}
// 		else {
// 			return QVector<libnutcommon::SelectResult>();
// 		}
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
