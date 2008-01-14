#ifndef NUTSDBUS_H
#define NUTSDBUS_H

#include <QDBusConnection>
#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <QHostAddress>
#include <QTimerEvent>

#include "libnutcommon/common.h"

//Hardcoded pidfile/pidfiledir
#define DBUS_PID_FILE_DIR "/var/run"
#define DBUS_PID_FILE_NAME "dbus.pid"

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
			QDBusConnection m_dbusConnection;
			DeviceManager * m_devmgr;
			QHash<QString, DBusDevice *> m_dbusDevices;
			static const QString m_dbusPath, m_dbusDevicesPath;
			int m_timerId;
			void startDBus();
			void timerEvent(QTimerEvent *event);
			libnutcommon::CDBusMonitor m_dbusMonitor;
			
		private slots:
			void devAdded(QString devName, Device *dev);
			void devRemoved(QString devName, Device *dev);
			
			friend int mainApp(int argc, char* argv[]);
			friend class DeviceManager;

			void stopDBus();
			void dbusStopped();
			void dbusStarted();
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
			Device *m_device;
			QDBusConnection *m_dbusConnection;
			QList<DBusEnvironment* > m_dbusEnvironments;
			QString m_dbusPath;
			libnutcommon::DeviceProperties m_dbusProperties;
			int m_activeEnvironment;
		
		private slots:
			void stateChanged(libnutcommon::DeviceState newState, libnutcommon::DeviceState oldState);
			void environmentChanged(int newEnvironment);
		
		public:
			DBusDevice(Device *dev, QDBusConnection *connection, const QString &path);
			virtual ~DBusDevice();
			
			QString getPath();
	
		public slots:
			libnutcommon::DeviceProperties getProperties();
			QList<QDBusObjectPath> getEnvironments();
			libnutcommon::DeviceConfig getConfig();
			Q_NOREPLY void enable();
			Q_NOREPLY void disable();
			Q_NOREPLY void setEnvironment(const QDBusObjectPath &path);
			Q_NOREPLY void setEnvironment(qint32 env) {
				m_device->setUserPreferredEnvironment(env);
			}
			
			QString getEssid() { return m_device->essid(); }
		
		signals:
			void stateChanged(int newState, int oldState);
			void environmentChangedActive(const QString &objectpath);
			void newWirelessNetworkFound();
	};
	
	class DBusEnvironment: public QDBusAbstractAdaptor {
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".Environment")
		private:
			Environment *m_environment;
			QDBusConnection *m_dbusConnection;
			QList<DBusInterface_IPv4*> m_dbusInterfacesIPv4;
			#ifdef IPv6
			QList<DBusInterface_IPv6*> m_dbusInterfacesIPv6;
			#endif
			QString m_dbusPath;
			libnutcommon::EnvironmentProperties m_dbusProperties;
			Device * m_device;

		private slots:
			void selectResultReady();
		public:
			DBusEnvironment(Environment *env, QDBusConnection *connection, const QString &path, Device* dev);
			virtual ~DBusEnvironment();
			inline Environment * getEnvironment() const { return m_environment; }
	
			QString getPath();
			void emitChange(bool change);
	
		public slots:
			libnutcommon::EnvironmentProperties getProperties();
			libnutcommon::EnvironmentConfig getConfig();
			QList<QDBusObjectPath> getInterfaces();
			libnutcommon::SelectResult getSelectResult();
			QVector<libnutcommon::SelectResult> getSelectResults();
		signals:
			void interfaceAdded(const QDBusObjectPath &objectpath);
			void interfaceRemoved(const QDBusObjectPath &objectpath);
			void selectsResultChanged(libnutcommon::SelectResult result, QVector<libnutcommon::SelectResult> results);
			void stateChanged(bool state);
	};

	class DBusInterface_IPv4: public QDBusAbstractAdaptor {
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "de.unistuttgart.nut" ".Interface_IPv4")
		private:
			Interface_IPv4 *m_interface;
			QDBusConnection *m_dbusConnection;
			QString m_dbusPath;
			libnutcommon::InterfaceProperties m_dbusProperties;
		private slots:
			void interfaceStatusChanged(libnutcommon::InterfaceState state);
		public:
			DBusInterface_IPv4(Interface_IPv4 *iface, QDBusConnection *connection, const QString &path);
			virtual ~DBusInterface_IPv4();
	
			QString getPath();
		
		public slots:
			libnutcommon::InterfaceProperties getProperties();
			libnutcommon::IPv4Config getConfig();
	
			bool needUserSetup() { return m_interface->needUserSetup(); }
			bool setUserConfig(libnutcommon::IPv4UserConfig userConfig) { return m_interface->setUserConfig(userConfig); }
			libnutcommon::IPv4UserConfig getUserConfig() { return m_interface->getUserConfig(); }

			//void activate() { m_interface->start();}
			//void deactivate() { m_interface->stop();}
		
		signals:
			void stateChanged(const libnutcommon::InterfaceProperties &properties);
	};
	#ifdef IPv6
	class DBusInterface_IPv6: public QDBusAbstractAdaptor {
		Q_OBJECT
		Q_CLASSINFO("D-BUS Interface", "de.unistuttgart.nut" ".Interface_IPv6");
		
	}
	#endif
}

#endif
