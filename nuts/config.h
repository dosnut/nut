
#ifndef _NUTS_CONFIG_H
#define _NUTS_CONFIG_H

#include <QObject>
#include <QString>
#include <QList>
#include <QHostAddress>
#include <QHash>

namespace nuts {
	class Config;
	class DeviceConfig;
	class EnvironmentConfig;
	class IPv4Config;
};

namespace nuts {
	class Config {
		private:
			QString configFile;
			QHash<QString, DeviceConfig*> devices;
		
		public:
			Config(const QString &configFile);
			virtual ~Config();
			
			DeviceConfig* getDevice(const QString &deviceName);
			
			DeviceConfig* createDevice(const QString &deviceName);
	};
	
	class DeviceConfig {
		public:
			DeviceConfig();
			virtual ~DeviceConfig();
			QList<EnvironmentConfig*> environments;
			
			EnvironmentConfig* getDefaultEnv();
			EnvironmentConfig* createEnvironment();
			
			EnvironmentConfig *defEnv;
			bool canUserEnable;
	};
	
	class EnvironmentConfig {
		public:
			EnvironmentConfig();
			virtual ~EnvironmentConfig();
			bool canUserSelect;
			bool noDefaultDHCP, noDefaultZeroconf;
			QList<IPv4Config*> ipv4Interfaces;
			IPv4Config *dhcp, *zeroconf;
			
			IPv4Config *doDHCP();
	};
	
	class IPv4Config {
		public:
			typedef enum {
				DO_DHCP      = 1,
				DO_ZEROCONF  = 2,
				DO_STATIC    = 4,
				MAY_USERSTATIC = 8
			} Flags;
			
			typedef enum {
				OW_IP        = 1,
				OW_NETMASK   = 2,
				OW_GATEWAY   = 4,
				OW_DNSSERVER = 8
			} OverwriteFlags;
			
			IPv4Config(int flags, int overwriteFlags);
			
			QHostAddress static_ip, static_netmask, static_gateway;
			QList<QHostAddress> static_dnsservers;
			
			int flags;
			int overwriteFlags;
			bool canUserEnable;
	};
};

#endif
