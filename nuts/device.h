
#ifndef _NUTS_DEVICE_H
#define _NUTS_DEVICE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QHostAddress>

namespace nuts {
	class DeviceManager;
	class Device;
	class Environment;
	class Interface;
	class Interface_IPv4;
	class MacAddress;
};

#include "config.h"

namespace nuts {
	class DeviceManager : public QObject {
		Q_OBJECT
		protected:
			Config *config;
			
			QList<Device*> devices;
			
			friend class HardwareManager;
			
			void gotCarrier(int ifIndex);
			void lostCarrier(int ifIndex);
		public:
			DeviceManager(const QString &configFile);
			virtual ~DeviceManager();
	};
	
	class Device : public QObject {
		Q_OBJECT
		Q_PROPERTY(QString name READ getName)
		Q_PROPERTY(int current READ getCurrent WRITE setCurrent)
		Q_PROPERTY(bool enabled READ getEnabled WRITE setEnabled)
		protected:
			QString name;
			int interfaceIndex;
			int activeEnv;
			bool enabled;
			QList<Environment*> envs;
			
			DeviceConfig *config;
		
			void envUp(Environment*);
			void envDown(Environment*);
			
		public:
			Device(const QString &name, int interfaceIndex, DeviceConfig *config);
			virtual ~Device();
			
			// Properties
			QString getName();
			
			int getCurrent();
			void setCurrent(int i);
			
			bool getEnabled();
			void setEnabled(bool b);
			
			const QList<Environment*>& getEnvironments();
	};
	
	class Environment : public QObject {
		Q_OBJECT
		Q_PROPERTY(Device* device READ getDevice)
		
		protected:
			Device *device;
			QList<Interface*> ifs;
			
			EnvironmentConfig *config;
			
			void start();
			void stop();
			
			void ifUp(Interface*);
			void ifDown(Interface*);
			
		public:
			Environment(Device *device, EnvironmentConfig *config);
			virtual ~Environment();
			
			Device* getDevice();
			
			const QList<Interface*>& getInterfaces();
	};
	
	class Interface : public QObject {
		Q_OBJECT
		public:
			Interface();
			virtual ~Interface();
	};
	
	class Interface_IPv4 : public Interface {
		Q_OBJECT
		protected:
			void startDHCP();
			void startZeroconf();
			void startStatic();
		
		public:
			Interface_IPv4(IPv4Config *config);
			virtual ~Intercace_IPv4();
			void start();
			
			QHostAddress ip, netmask, gateway;
			QList<QHostAddress> dnsserver;
			
			IPv4Config *config;
	};

	class MacAddress {
		public:
			MacAddress(const QString &str);
			quint8 data[6];
	};
};

#endif
