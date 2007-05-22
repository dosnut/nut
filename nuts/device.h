
#ifndef _NUTS_DEVICE_H
#define _NUTS_DEVICE_H

#include <QObject>
#include <QDBusAbstractAdaptor>
#include <QString>
#include <QList>

namespace nuts {
	class DeviceManager;
	class Device;
	class Environment;
	class Interface;
};

namespace nuts {
	class DeviceManager : public QObject {
		Q_OBJECT
		protected slots:
			friend class HardwareManager;
			
			bool addHardwareManager(HardwareManager* hwm);
			
			void gotCarrier(int ifIndex);
			void lostCarrier(int ifIndex);
			
			
	};
	
	class Device : public QObject {
		Q_OBJECT
		private:
			QString p_name;
			int interfaceIndex;
		public:
			Device(const QString &name, int interfaceIndex);
			~Device();
			QString name();
		
			int activeEnv;
			QList<Environment*> envs;
	};
	
	class Environment : public QObject {
		Q_OBJECT
		private:
			Device *p_device;
		public:
			Environment(Device *device);
			~Environment();
			
			Device *device();
			QList<Interface*> ifs;
	};
	
	class Interface : public QObject {
		Q_OBJECT
		public:
			Interface();
			virtual ~Interface();
	};
	
	class IPv4 : public Interface {
		Q_OBJECT
		public:
			typedef enum {
				DHCP     = 1,
				ZEROCONF = 2,
				STATIC   = 4
			} Flags;
			
			Flags flags;
	};
};

#endif
