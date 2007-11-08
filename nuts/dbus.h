#ifndef NUTSDBUS_H
#define NUTSDBUS_H

#include <QDBusConnection>
#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <QHostAddress>

#include <common/types.h>
#include <common/dbus.h>
#include <common/config.h>

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
			QDBusConnection dbus_connection;
			DeviceManager *s_devmgr;
			QHash<QString, DBusDevice *> dbus_devices;
			static const QString dbus_path, dbus_devices_path;
			
		private slots:
			void devAdded(QString devName, Device *dev);
			void devRemoved(QString devName, Device *dev);
			
			friend int mainApp(int argc, char* argv[]);
			friend class DeviceManager;
			void stopDBus();
		
		public:
			DBusDeviceManager(DeviceManager *devmgr);
			virtual ~DBusDeviceManager();
	
		public slots:
			QList<QDBusObjectPath> getDeviceList();
		signals:
			void deviceAdded(const QDBusObjectPath &objectpath);
			void deviceRemoved(const QDBusObjectPath &objectpath);
	};
	
	class DBusDevice: public QDBusAbstractAdaptor {
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".Device")
		private:
			Device *s_device;
			QDBusConnection *dbus_connection;
			QList<DBusEnvironment* > dbus_environments;
			QString dbus_path;
			libnut::DeviceProperties dbus_properties;
		
		private slots:
			void stateChanged(libnut::DeviceState newState, libnut::DeviceState oldState) {
				emit(stateChanged((int) newState, (int) oldState));
			}
		
		public:
			DBusDevice(Device *dev, QDBusConnection *connection, const QString &path);
			virtual ~DBusDevice();
			
			QString getPath();
	
		public slots:
			libnut::DeviceProperties getProperties();
			QList<QDBusObjectPath> getEnvironments();
			nut::DeviceConfig getConfig();
			Q_NOREPLY void enable();
			Q_NOREPLY void disable();
			Q_NOREPLY void setEnvironment(const QDBusObjectPath &path);
			Q_NOREPLY void setEnvironment(int env) {
				s_device->setUserPreferredEnvironment(env);
			}
			
			QString getEssid() { return s_device->essid(); }
		
		signals:
			void stateChanged(int newState, int oldState);
			void environmentChangedActive(const QDBusObjectPath &objectpath);
	};
	
	class DBusEnvironment: public QDBusAbstractAdaptor {
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".Environment")
		private:
			Environment *s_environment;
			QDBusConnection *dbus_connection;
			QList<DBusInterface_IPv4*> dbus_interfaces_IPv4;
			#ifdef IPv6
			QList<DBusInterface_IPv6*> dbus_interfaces_IPv6;
			#endif
			QString dbus_path;
			libnut::EnvironmentProperties dbus_properties;
		
		public:
			DBusEnvironment(Environment *env, QDBusConnection *connection, const QString &path);
			virtual ~DBusEnvironment();
			inline Environment * getEnvironment() const { return s_environment; }
	
			QString getPath();
	
		public slots:
			libnut::EnvironmentProperties getProperties();
			nut::EnvironmentConfig getConfig();
			QList<QDBusObjectPath> getInterfaces();
			nut::SelectResult getSelectResult();
			QVector<nut::SelectResult> getSelectResults();
		signals:
			void interfaceAdded(const QDBusObjectPath &objectpath);
			void interfaceRemoved(const QDBusObjectPath &objectpath);
	};

	class DBusInterface_IPv4: public QDBusAbstractAdaptor {
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".Interface_IPv4")
		private:
			Interface_IPv4 *s_interface;
			QDBusConnection *dbus_connection;
			QString dbus_path;
			libnut::InterfaceProperties dbus_properties;
		private slots:
			void interfaceStatusChanged(libnut::InterfaceState state);
		public:
			DBusInterface_IPv4(Interface_IPv4 *iface, QDBusConnection *connection, const QString &path);
			virtual ~DBusInterface_IPv4();
	
			QString getPath();
		
		public slots:
			libnut::InterfaceProperties getProperties();
			nut::IPv4Config getConfig();
	
			bool needUserSetup() { return s_interface->needUserSetup(); }
			bool setUserConfig(nut::IPv4UserConfig userConfig) { return s_interface->setUserConfig(userConfig); }
			nut::IPv4UserConfig getUserConfig() { return s_interface->getUserConfig(); }
		
		signals:
			void stateChanged(const libnut::InterfaceProperties &properties);
	};
	#ifdef IPv6
	class DBusInterface_IPv6: public QDBusAbstractAdaptor {
		Q_OBJECT
		Q_CLASSINFO("D-BUS Interface", "de.unistuttgart.de" ".Interface_IPv6");
		
	}
	#endif
}

#endif
