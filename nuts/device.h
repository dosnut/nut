
#ifndef _NUTS_DEVICE_H
#define _NUTS_DEVICE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QHostAddress>
#include <QBitArray>

namespace nuts {
	class DeviceManager;
	class Device;
	class Environment;
	class Interface;
	class Interface_IPv4;
	class MacAddress;
};

#include "config.h"
#include "hardware.h"

namespace nuts {
	class DeviceManager : public QObject {
		Q_OBJECT
		protected:
			Config *config;
			HardwareManager hwman;
			
			QHash<QString, Device*> devices;
			
			friend class HardwareManager;
			friend class Device;
			friend class Environment;
			friend class Interface_IPv4;
			
		protected slots:
			void gotCarrier(const QString &ifName, int ifIndex);
			void lostCarrier(const QString &ifName);
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
			friend class DeviceManager;
			friend class Environment;
			friend class Interface_IPv4;
			
			DeviceManager *dm;
			QString name;
			int interfaceIndex;
			DeviceConfig *config;
			int activeEnv, nextEnv;
			bool enabled;
			QList<Environment*> envs;
			
		
			void envUp(Environment*);
			void envDown(Environment*);
			
			void gotCarrier(int ifIndex);
			void lostCarrier();
		public:
			Device(DeviceManager* dm, const QString &name, DeviceConfig *config);
			virtual ~Device();
			
			// Properties
			QString getName();
			
			int getCurrent();
			void setCurrent(int i);
			
			bool getEnabled();
			void setEnabled(bool b, bool force = false);
			
			const QList<Environment*>& getEnvironments();
		signals:
			void deviceUp();
			void deviceDown();
	};
	
	class Environment : public QObject {
		Q_OBJECT
		Q_PROPERTY(Device* device READ getDevice)
		
		protected:
			friend class Device;
			friend class Interface;
			friend class Interface_IPv4;
			
			Device *device;
			QList<Interface*> ifs;
			
			EnvironmentConfig *config;
			QBitArray ifUpStatus;
			bool envIsUp, envStart;
			
			void checkStatus();
			
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
		protected:
			friend class Environment;
			int index;
			
		public:
			Interface(int index);
			virtual ~Interface();
			
			virtual void start() = 0;
			virtual void stop() = 0;
	};
	
	class Interface_IPv4 : public Interface {
		Q_OBJECT
		protected:
			Environment *env;
			DeviceManager *dm;
			
			void startDHCP();
			void startZeroconf();
			void startStatic();
			
			void systemUp();
			void systemDown();
		
		public:
			Interface_IPv4(Environment *env, int index, IPv4Config *config);
			virtual ~Interface_IPv4();
			virtual void start();
			virtual void stop();
			
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
