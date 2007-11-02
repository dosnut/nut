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
	const QString DBusDeviceManager::dbus_path("/manager");
	const QString DBusDeviceManager::dbus_devices_path("/devices");

	DBusDeviceManager::DBusDeviceManager(DeviceManager *devmgr)
	: QDBusAbstractAdaptor(devmgr), dbus_connection(QDBusConnection::systemBus()), s_devmgr(devmgr) {
		//Register Service and device manager object
		dbus_connection.registerService(NUT_DBUS_URL);
		dbus_connection.registerObject(dbus_path, devmgr);
		//Insert devices into devices Hash.
		QHashIterator<QString, Device *> i(s_devmgr->getDevices());
		while (i.hasNext()) {
			i.next();
			devAdded(i.key(), i.value());
		}
		connect(devmgr, SIGNAL(deviceAdded(QString, Device*)), SLOT(devAdded(QString, Device*)));
		connect(devmgr, SIGNAL(deviceRemoved(QString, Device*)), SLOT(devRemoved(QString, Device*)));
	}

	DBusDeviceManager::~DBusDeviceManager() {
		dbus_connection.unregisterObject(dbus_path);
		dbus_connection.unregisterService(NUT_DBUS_URL);
	}
	
	void DBusDeviceManager::stopDBus() {
		dbus_connection.unregisterService(NUT_DBUS_URL);
		dbus_connection.unregisterObject(dbus_path);
	}
	
	//SLOT: Inserts device into device hash
	void DBusDeviceManager::devAdded(QString devName, Device *dev) {
		DBusDevice *dbus_device = new DBusDevice(dev, &dbus_connection, dbus_devices_path);
		dbus_devices.insert(devName, dbus_device);
		emit deviceAdded(QDBusObjectPath(dbus_device->getPath()));
	}
	
	
	void DBusDeviceManager::devRemoved(QString devName, Device* /* *dev */) {
		DBusDevice *dbus_device = dbus_devices[devName];
		emit deviceRemoved(QDBusObjectPath(dbus_device->getPath()));
		dbus_devices.remove(devName);
		// dbus_device is deleted as child of dev
	}
	
	QList<QDBusObjectPath> DBusDeviceManager::getDeviceList() {
		QList<QDBusObjectPath> paths;
		foreach (DBusDevice *dbus_device, dbus_devices) {
			paths.append(QDBusObjectPath(dbus_device->getPath()));
		}
		return paths;
	}

	DBusDevice::DBusDevice(Device *dev, QDBusConnection *connection, const QString &path)
	: QDBusAbstractAdaptor(dev), s_device(dev), dbus_connection(connection) {

		//Set Device Properties
		dbus_properties.type = s_device->hasWLAN() ? libnut::DT_AIR : libnut::DT_ETH;
		dbus_properties.name = s_device->getName();
		dbus_properties.state = s_device->getState();

		//Set dbus device path an register objects
		dbus_path = path + '/' + dev->getName();
		dbus_connection->registerObject(dbus_path, s_device);
		
		//Add Environments
		foreach (Environment *env, s_device->getEnvironments()) {
			DBusEnvironment *dbus_env = new DBusEnvironment(env, dbus_connection, dbus_path);
			dbus_environments.append(dbus_env);
			emit(environmentAdded(QDBusObjectPath(dbus_env->getPath())));
		}

		//Set active Environment
		int active_environment = s_device->getEnvironment();
		if (active_environment >= 0) {
			dbus_properties.activeEnvironment = dbus_environments[active_environment]->getPath();
		}
		else {
			dbus_properties.activeEnvironment = "";
		}
		connect(s_device,SIGNAL(stateChanged(libnut::DeviceState, libnut::DeviceState, Device*)),this,SLOT(stateChanged(libnut::DeviceState, libnut::DeviceState)));
		setAutoRelaySignals(true);
	}
	
	DBusDevice::~DBusDevice() {
		dbus_connection->unregisterObject(dbus_path, QDBusConnection::UnregisterTree);
	}
	
	QString DBusDevice::getPath() {
		return dbus_path;
	}

	libnut::DeviceProperties DBusDevice::getProperties() {
		dbus_properties.state = s_device->getState();
		dbus_properties.type = s_device->hasWLAN() ? libnut::DT_AIR : libnut::DT_ETH;
		int active_environment = s_device->getEnvironment();
		if (active_environment >= 0) {
			dbus_properties.activeEnvironment = dbus_environments[active_environment]->getPath();
		} else {
			dbus_properties.activeEnvironment = QString();
		}
		return dbus_properties;
	}
	
	QList<QDBusObjectPath> DBusDevice::getEnvironments() {
		QList<QDBusObjectPath> paths;
		foreach (DBusEnvironment *dbus_env, dbus_environments) {
			paths.append(QDBusObjectPath(dbus_env->getPath()));
		}
		return paths;
	}
	nut::DeviceConfig DBusDevice::getConfig() {
		return (s_device->getConfig());
	}

	void DBusDevice::setEnvironment(const QDBusObjectPath &path) {
		foreach(DBusEnvironment * i, dbus_environments) {
			if (i->getPath() == path.path()) {
				s_device->setUserPreferredEnvironment((i->getEnvironment())->getID());
				break;
			}
		}
	}
	
	void DBusDevice::enable() {
		s_device->enable(true);
	}

	void DBusDevice::disable() {
		s_device->disable();
	}

	DBusEnvironment::DBusEnvironment(Environment *env, QDBusConnection *connection, const QString &path)
	: QDBusAbstractAdaptor(env), s_environment(env), dbus_connection(connection) {
		//Set dbus path an register object
		dbus_path = path + QString("/%1").arg(s_environment->getID());
		dbus_connection->registerObject(dbus_path, s_environment);

		//Insert interfaces
		foreach (Interface *interface, s_environment->getInterfaces()) {
			//Check if interface is IPv4 or IPv6
			Interface_IPv4 *ifv4 = dynamic_cast<Interface_IPv4*>(interface);
			if (ifv4) {
				DBusInterface_IPv4 *dbus_interface = new DBusInterface_IPv4(ifv4, dbus_connection, dbus_path);
				dbus_interfaces_IPv4.append(dbus_interface);
				emit(interfaceAdded(QDBusObjectPath(dbus_interface->getPath())));
			}
			#ifdef IPv6
			else {
				Interface_IPv6 *if6 = dynamic_cast<Interface_IPv6*>(interface);
				if (if6) {
					DBusInterface_IPv6 *dbus_interface = new DBusInterface_IPv6(ifv6, dbus_connection, dbus_path);
					dbus_interfaces_IPv6.append(dbus_interface);
					emit(interfaceAdded(dbus_interface->getPath()));
				}
			}
			#endif
		}
	}
	
	DBusEnvironment::~DBusEnvironment() {
		dbus_connection->unregisterObject(dbus_path);
	}
	
	QString DBusEnvironment::getPath() {
		return dbus_path;
	}
	
	libnut::EnvironmentProperties DBusEnvironment::getProperties() {
		dbus_properties.name = s_environment->getName();
		return dbus_properties;
	}
	nut::EnvironmentConfig DBusEnvironment::getConfig() {
		return (s_environment->getConfig());
	}
	
	QList<QDBusObjectPath> DBusEnvironment::getInterfaces() {
		QList<QDBusObjectPath> paths;
		//Append IPv4 Paths
		foreach (DBusInterface_IPv4 *i, dbus_interfaces_IPv4) {
			paths.append(QDBusObjectPath(i->getPath()));
		}
		#ifdef IPv6
		//Append IPv6 Paths
		foreach (DBusInterface_IPv6 *i, dbus_interfaces_IPv6) {
			paths.append(QDBusObjectPath(i->getPath()));
		}
		#endif
		return paths;
	}

	DBusInterface_IPv4::DBusInterface_IPv4(Interface_IPv4 *iface, QDBusConnection *connection, const QString &path)
	: QDBusAbstractAdaptor(iface), s_interface(iface), dbus_connection(connection) {
		dbus_path = path + QString("/%1").arg(s_interface->getIndex());
		dbus_connection->registerObject(dbus_path, s_interface);
		//Set Interface properties
		dbus_properties.ip = s_interface->ip;
		dbus_properties.gateway = s_interface->gateway;
		dbus_properties.netmask = s_interface->netmask;
		dbus_properties.ifState = s_interface->getState();
		
		if (!s_interface->dnsserver.isEmpty()) {
			dbus_properties.dns = s_interface->dnsserver;
		}
		else {
			dbus_properties.dns = QList<QHostAddress>();
		}
		connect(s_interface,SIGNAL(interfaceUp(Interface_IPv4*)),this,SLOT(interfaceUp()));
		connect(s_interface,SIGNAL(interfaceDown(Interface_IPv4*)),this,SLOT(interfaceDown()));
	}
	
	DBusInterface_IPv4::~DBusInterface_IPv4() {
	}

	QString DBusInterface_IPv4::getPath() {
		return dbus_path;
	}
	//Private SLOTS:

	void DBusInterface_IPv4::interfaceUp() {
		dbus_properties.ip = s_interface->ip;
		dbus_properties.gateway = s_interface->gateway;
		dbus_properties.netmask = s_interface->netmask;
		dbus_properties.ifState = s_interface->getState();
		emit stateChanged(dbus_properties);
	}
	void DBusInterface_IPv4::interfaceDown() {
		dbus_properties.ip = s_interface->ip;
		dbus_properties.gateway = s_interface->gateway;
		dbus_properties.netmask = s_interface->netmask;
		dbus_properties.ifState = s_interface->getState();
		emit(stateChanged(dbus_properties));
	}


	libnut::InterfaceProperties DBusInterface_IPv4::getProperties() {
		dbus_properties.ip = s_interface->ip;
		dbus_properties.gateway = s_interface->gateway;
		dbus_properties.netmask = s_interface->netmask;
		dbus_properties.ifState = s_interface->getState();
	
		if (!s_interface->dnsserver.isEmpty()) {
			dbus_properties.dns = s_interface->dnsserver;
		}
		else {
			dbus_properties.dns = QList<QHostAddress>();
		}
		return dbus_properties;
	}
	nut::IPv4Config DBusInterface_IPv4::getConfig() {
		return (s_interface->getConfig());
	}
	void DBusInterface_IPv4::setIP(quint32 /*HostAddress*/) {
	}
	void DBusInterface_IPv4::setNetmask(quint32 /*Netmask*/) {
	}
	void DBusInterface_IPv4::setGateway(quint32 /*Gateway*/) {
	}
	void DBusInterface_IPv4::setDNS(QList<QHostAddress> /*DNS*/) {
	}

}
