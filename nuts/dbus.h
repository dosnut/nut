#ifndef NUTSDBUS_H
#define NUTSDBUS_H

#include <QDBusConnection>
#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

#include <common/types.h>
#include <common/dbus.h>

namespace nuts {
	class DBusDeviceManager;
	class DBusDevice;
	class DBusEnvironment;
	class DBusInterface;
}

#include "device.h"

namespace nuts {
	class DBusDeviceManager: public QDBusAbstractAdaptor {
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".DeviceManager")
		private:
			QDBusConnection m_connection;
			DeviceManager *m_devmgr;
			QHash<QString, DBusDevice *> m_devices;
			static const QString s_manager_path, s_devices_path;
			
		private slots:
			void devAdded(QString devName, Device *dev);
			void devRemoved(QString devName, Device *dev);
		
		public:
			DBusDeviceManager(DeviceManager *devmgr);
			virtual ~DBusDeviceManager();
	
		public slots:
			QList<QDBusObjectPath> getDeviceList();
		signals:
			void deviceAdded(QDBusObjectPath objectpath);
			void deviceRemoved(QDBusObjectPath objectpath);
	};
	
	class DBusDevice: public QDBusAbstractAdaptor {
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".Device")
		private:
			Device *m_dev;
			QDBusConnection *m_connection;
			QList<DBusEnvironment*> m_envs;
			QString m_dbusPath;
			libnut::libnut_DeviceProperties m_properties;
		
		public:
			DBusDevice(Device *dev, QDBusConnection *connection, const QString &path);
			virtual ~DBusDevice();
			
			QString getPath();
	
		public slots:
			libnut::libnut_DeviceProperties getProperties();
			QList<libnut::libnut_wlanScanresult> getwlanScan();
			QList<QDBusObjectPath> getEnvironments();
			void enable();
			void disable();
		
		signals:
			void deviceEnabled();
			void deviceDisabled();
			void deviceUp();
			void deviceDown();
	};
	
	class DBusEnvironment: public QDBusAbstractAdaptor {
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".Environment")
		private:
			Environment *m_env;
			QDBusConnection *m_connection;
			QList<QObject*> m_ifaces;
			QString m_dbusPath;
			libnut::libnut_EnvironmentProperties m_properties;
		
		public:
			DBusEnvironment(Environment *env, QDBusConnection *connection, const QString &path);
			virtual ~DBusEnvironment();
	
			QString getPath();
	
		public slots:
//			QList<libnut::libnut_SelectConfig> getSelectConfig();
			libnut::libnut_EnvironmentProperties getProperties();
			
			QList<QDBusObjectPath> getInterfaces();
		signals:
	};
	
	class DBusInterface_IPv4: public QDBusAbstractAdaptor {
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".Interface_IPv4")
		private:
			Interface_IPv4 *m_iface;
			QDBusConnection *m_connection;
			QString m_dbusPath;
			libnut::libnut_InterfaceProperties m_properties;
		
		public:
			DBusInterface_IPv4(Interface_IPv4 *iface, QDBusConnection *connection, const QString &path);
			virtual ~DBusInterface_IPv4();
	
			QString getPath();
		
		public slots:
			libnut::libnut_InterfaceProperties getProperties();
			void setIP(quint32 HostAddress);
			void setNetmask(quint32 Netmask);
			void setGateway(quint32 Gateway);
		signals:
	};

}

#endif
