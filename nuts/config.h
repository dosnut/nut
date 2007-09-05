
#ifndef _NUTS_CONFIG_H
#define _NUTS_CONFIG_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVector>
#include <QHostAddress>
#include <QHash>
#include <common/macaddress.h>

namespace nuts {
	class Config;
	class DeviceConfig;
	class EnvironmentConfig;
	class IPv4Config;
};

namespace nuts {
	class Config {
		protected:
			friend class DeviceManager;
			QString configFile;
			QHash<QString, DeviceConfig*> devices;
		
		public:
			Config(const QString &configFile);
			virtual ~Config();
			
			DeviceConfig* getDevice(const QString &deviceName);
			
			DeviceConfig* createDevice(const QString &deviceName);
	};
	
	class DeviceConfig {
		protected:
			friend class Device;
			QList<EnvironmentConfig*> environments;
			EnvironmentConfig *defEnv;
		
		public:
			DeviceConfig();
			virtual ~DeviceConfig();
			
			EnvironmentConfig* getDefaultEnv();
			EnvironmentConfig* createEnvironment(const QString &name = "");
			
			bool canUserEnable;
	};
	
	class SelectFilter {
		public:
			typedef enum { SEL_USER, SEL_ARP, SEL_ESSID, SEL_BLOCK } SelectType;
			
			SelectFilter() : selType(SEL_USER) { }
			SelectFilter(const nut::MacAddress &macAddr) : selType(SEL_ARP), macAddr(macAddr) { }
			SelectFilter(const nut::MacAddress &macAddr, const QHostAddress &ipAddr) : selType(SEL_ARP), macAddr(macAddr), ipAddr(ipAddr) { }
			SelectFilter(const QString &essid) : selType(SEL_ESSID), essid(essid) { }
			SelectFilter(size_t block) : selType(SEL_BLOCK), block(block) { }
			
			SelectType selType;
			size_t block;
			QString essid;
			nut::MacAddress macAddr;
			QHostAddress ipAddr;
	};
	
	class SelectConfig {
		public:
			SelectConfig() { }
			
			// List of filters
			QVector<SelectFilter> filters;
			// List of blocks: each block contains at the first position the type:
			//  0 = AND, 1 = OR
			// followed by the list of filter ids for the block.
			QVector< QVector<size_t> > blocks;
	};
	
	class EnvironmentConfig {
		protected:
			friend class Environment;
		public:
			EnvironmentConfig(const QString &name = "");
			virtual ~EnvironmentConfig();
			
			QString name;
			bool canUserSelect;
			bool noDefaultDHCP, noDefaultZeroconf;
			QList<IPv4Config*> ipv4Interfaces;
			IPv4Config *dhcp, *zeroconf;
			SelectConfig *select;
			
			IPv4Config *doDHCP();
			IPv4Config *createStatic();
	};
	
	class IPv4Config {
		protected:
			friend class Interface_IPv4;
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
