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
	const QString DBusDeviceManager::s_manager_path("/manager");
	const QString DBusDeviceManager::s_devices_path("/devices");

	DBusDeviceManager::DBusDeviceManager(DeviceManager *devmgr)
	: QDBusAbstractAdaptor(devmgr), m_connection(QDBusConnection::systemBus()), m_devmgr(devmgr) {
		m_connection.registerService(NUT_DBUS_URL);
		m_connection.registerObject(s_manager_path, devmgr);
		QHashIterator<QString, Device *> i(m_devmgr->getDevices());
		while (i.hasNext()) {
			i.next();
			devAdded(i.key(), i.value());
		}
		connect(devmgr, SIGNAL(deviceAdded(QString, Device*)), SLOT(devAdded(QString, Device*)));
		connect(devmgr, SIGNAL(deviceRemoved(QString, Device*)), SLOT(devRemoved(QString, Device*)));
	}

	DBusDeviceManager::~DBusDeviceManager() {
		m_connection.unregisterService(NUT_DBUS_URL);
	}

	void DBusDeviceManager::devAdded(QString devName, Device *dev) {
		DBusDevice *ddev = new DBusDevice(dev, &m_connection, s_devices_path);
		m_devices.insert(devName, ddev);
		emit deviceAdded(QDBusObjectPath(ddev->getPath()));
	}
	
	void DBusDeviceManager::devRemoved(QString devName, Device* /* *dev */) {
		DBusDevice *ddev = m_devices[devName];
		emit deviceRemoved(QDBusObjectPath(ddev->getPath()));
		m_devices.remove(devName);
		// ddev is deleted as child of dev
	}
	
	QList<QDBusObjectPath> DBusDeviceManager::getDeviceList() {
		QList<QDBusObjectPath> paths;
		foreach (DBusDevice *ddev, m_devices) {
			paths.append(QDBusObjectPath(ddev->getPath()));
		}
		return paths;
	}

	DBusDevice::DBusDevice(Device *dev, QDBusConnection *connection, const QString &path)
	: QDBusAbstractAdaptor(dev), m_dev(dev), m_connection(connection) {
		m_dbusPath = path + '/' + dev->getName();
		m_connection->registerObject(m_dbusPath, dev);
		foreach (Environment *env, m_dev->getEnvironments()) {
			DBusEnvironment *denv = new DBusEnvironment(env, m_connection, m_dbusPath);
			m_envs.append(denv);
		}
		setAutoRelaySignals(true);
	}
	
	DBusDevice::~DBusDevice() {
	}
	
	QString DBusDevice::getPath() {
		return m_dbusPath;
	}

	libnut::libnut_DeviceProperties DBusDevice::getProperties() {
		return m_properties;
	}
	
	QList<libnut::libnut_wlanScanresult> DBusDevice::getwlanScan() {
		return QList<libnut::libnut_wlanScanresult>();
	}
	
	QList<QDBusObjectPath> DBusDevice::getEnvironments() {
		QList<QDBusObjectPath> paths;
		foreach (DBusEnvironment *denv, m_envs) {
			paths.append(QDBusObjectPath(denv->getPath()));
		}
		return paths;
	}
	
	void DBusDevice::enable() {
		m_dev->setEnabled(true);
	}
	void DBusDevice::disable() {
		m_dev->setEnabled(false);
	}

	DBusEnvironment::DBusEnvironment(Environment *env, QDBusConnection *connection, const QString &path)
	: QDBusAbstractAdaptor(env), m_env(env), m_connection(connection) {
		m_dbusPath = path + QString("/%1").arg(env->getID());
		log << "Dbus Path: " << m_dbusPath << endl;
		m_connection->registerObject(m_dbusPath, env);
		foreach (Interface *iface, m_env->getInterfaces()) {
			Interface_IPv4 *ifv4 = dynamic_cast<Interface_IPv4*>(iface);
			if (ifv4) {
				DBusInterface_IPv4 *diface = new DBusInterface_IPv4(ifv4, m_connection, m_dbusPath);
				m_ifaces.append(diface);
			}
		}
	}
	
	DBusEnvironment::~DBusEnvironment() {
	}
	
	QString DBusEnvironment::getPath() {
		return m_dbusPath;
	}
	
	libnut::libnut_EnvironmentProperties DBusEnvironment::getProperties() {
		return m_properties;
	}
	
	QList<QDBusObjectPath> DBusEnvironment::getInterfaces() {
		QList<QDBusObjectPath> paths;
		foreach (QObject *diface, m_ifaces) {
			DBusInterface_IPv4 *difv4 = dynamic_cast<DBusInterface_IPv4*>(diface);
			if (difv4)
				paths.append(QDBusObjectPath(difv4->getPath()));
		}
		return paths;
	}

	DBusInterface_IPv4::DBusInterface_IPv4(Interface_IPv4 *iface, QDBusConnection *connection, const QString &path)
	: QDBusAbstractAdaptor(iface), m_iface(iface), m_connection(connection) {
		m_dbusPath = path + QString("/%1").arg(iface->getIndex());
		m_connection->registerObject(m_dbusPath, iface);
	}
	
	DBusInterface_IPv4::~DBusInterface_IPv4() {
	}

	QString DBusInterface_IPv4::getPath() {
		return m_dbusPath;
	}

	libnut::libnut_InterfaceProperties DBusInterface_IPv4::getProperties() {
		return m_properties;
	}
	void DBusInterface_IPv4::setIP(quint32 HostAddress) {
	}
	void DBusInterface_IPv4::setNetmask(quint32 Netmask) {
	}
	void DBusInterface_IPv4::setGateway(quint32 Gateway) {
	}
}