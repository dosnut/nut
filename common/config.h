//
// C++ Interface: config
//
// Description: 
//
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef _NUT_CONFIG_H
#define _NUT_CONFIG_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVector>
#include <QHostAddress>
#include <QHash>
#include <QDBusArgument>
#include "macaddress.h"
#include "types.h"

namespace nut {
	class Config;
	class DeviceConfig;
	class SelectRule;
	class SelectConfig;
	class EnvironmentConfig;
	class IPv4Config;
	
	QDBusArgument &operator<< (QDBusArgument &argument, const Config &data);
	const QDBusArgument &operator>> (const QDBusArgument &argument, Config &data);
	
	QDBusArgument &operator<< (QDBusArgument &argument, const DeviceConfig &data);
	const QDBusArgument &operator>> (const QDBusArgument &argument, DeviceConfig &data);
	
	QDBusArgument &operator<< (QDBusArgument &argument, const SelectRule &data);
	const QDBusArgument &operator>> (const QDBusArgument &argument, SelectRule &data);
	
	QDBusArgument &operator<< (QDBusArgument &argument, const SelectConfig &data);
	const QDBusArgument &operator>> (const QDBusArgument &argument, SelectConfig &data);
	
	QDBusArgument &operator<< (QDBusArgument &argument, const EnvironmentConfig &data);
	const QDBusArgument &operator>> (const QDBusArgument &argument, EnvironmentConfig &data);
	
	QDBusArgument &operator<< (QDBusArgument &argument, const IPv4Config &data);
	const QDBusArgument &operator>> (const QDBusArgument &argument, IPv4Config &data);
}

namespace nuts {
	class ConfigParser;
}

namespace nut {
	class Config {
		private:
			bool m_isCopy;
		protected:
			friend class nuts::ConfigParser;
			friend QDBusArgument &operator<< (QDBusArgument &argument, const Config &data);
			friend const QDBusArgument &operator>> (const QDBusArgument &argument, Config &data);
			QHash<QString, DeviceConfig*> m_devices;
		
		public:
			Config();
			Config(const Config &other);
			virtual ~Config();
			
			DeviceConfig* getDevice(const QString &deviceName);
			
			const QHash<QString, DeviceConfig*> &getDevices() {
				return m_devices;
			}
	};
	
	class DeviceConfig {
		private:
			bool m_isCopy;
		protected:
			friend class nuts::ConfigParser;
			friend QDBusArgument &operator<< (QDBusArgument &argument, const DeviceConfig &data);
			friend const QDBusArgument &operator>> (const QDBusArgument &argument, DeviceConfig &data);
			QList<EnvironmentConfig*> m_environments;
			bool m_noAutoStart;
		
		public:
			DeviceConfig();
			DeviceConfig(const DeviceConfig &other);
			virtual ~DeviceConfig();
			
			const QList<EnvironmentConfig*>& getEnvironments() {
				return m_environments;
			}
			
			bool noAutoStart() { return m_noAutoStart; }
	};
	
	class SelectRule {
		protected:
			friend QDBusArgument &operator<< (QDBusArgument &argument, const SelectRule &data);
			friend const QDBusArgument &operator>> (const QDBusArgument &argument, SelectRule &data);

		public:
			typedef enum { SEL_USER, SEL_ARP, SEL_ESSID, SEL_BLOCK } SelectType;
			
			SelectRule() : selType(SEL_USER) { }
			SelectRule(const QHostAddress &ipAddr) : selType(SEL_ARP), ipAddr(ipAddr) { }
			SelectRule(const QHostAddress &ipAddr, const nut::MacAddress &macAddr) : selType(SEL_ARP), ipAddr(ipAddr), macAddr(macAddr) { }
			SelectRule(const QString &essid) : selType(SEL_ESSID), essid(essid) { }
			SelectRule(quint32 block) : selType(SEL_BLOCK), block(block) { }
			
			SelectType selType;
			quint32 block;
			QString essid;
			QHostAddress ipAddr;
			nut::MacAddress macAddr;
	};
	
	class SelectConfig {
			friend class nuts::ConfigParser;
			friend QDBusArgument &operator<< (QDBusArgument &argument, const SelectConfig &data);
			friend const QDBusArgument &operator>> (const QDBusArgument &argument, SelectConfig &data);

		public:
			SelectConfig() { }
			
			// List of filters
			QVector<SelectRule> filters;
			// List of blocks: each block contains at the first position the type:
			//  0 = AND, 1 = OR
			// followed by the list of filter ids for the block.
			QVector< QVector<quint32> > blocks;
	};
	
	class EnvironmentConfig {
		private:
			bool m_isCopy;
		protected:
			friend class nuts::ConfigParser;
			friend QDBusArgument &operator<< (QDBusArgument &argument, const EnvironmentConfig &data);
			friend const QDBusArgument &operator>> (const QDBusArgument &argument, EnvironmentConfig &data);
			QString m_name;
			bool m_canUserSelect;
			bool m_noDefaultDHCP, m_noDefaultZeroconf;
			QList<IPv4Config*> m_ipv4Interfaces;
			IPv4Config *m_dhcp, *m_zeroconf; // should be moved to ConfigParser, only for parsing
			SelectConfig m_select;
			
		public:
			EnvironmentConfig(const QString &name = "");
			EnvironmentConfig(const EnvironmentConfig& other);
			virtual ~EnvironmentConfig();
			
			QString getName() { return m_name; }
			bool getCanUserSelect() { return m_canUserSelect; }
			bool getNoDefaultDHCP() { return m_noDefaultDHCP; }
			bool getNoDefaultZeroconf() { return m_noDefaultZeroconf; }
			const QList<IPv4Config*>& getIPv4Interfaces() { return m_ipv4Interfaces; }
			const SelectConfig &getSelect() { return m_select; }
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
		
		protected:
			friend class nuts::ConfigParser;
			friend QDBusArgument &operator<< (QDBusArgument &argument, const IPv4Config &data);
			friend const QDBusArgument &operator>> (const QDBusArgument &argument, IPv4Config &data);
			
			QHostAddress m_static_ip, m_static_netmask, m_static_gateway;
			QList<QHostAddress> m_static_dnsservers;
			
			int m_flags;
			int m_overwriteFlags;
			bool m_canUserEnable;
		
		public:	
			IPv4Config(int flags = IPv4Config::DO_DHCP | IPv4Config::DO_ZEROCONF, int overwriteFlags = 0);
			
			const QHostAddress& getStaticIP() const { return m_static_ip; }
			const QHostAddress& getStaticNetmask() const { return m_static_netmask; }
			const QHostAddress& getStaticGateway() const { return m_static_gateway; }
			const QList<QHostAddress>& getStaticDNS() const { return m_static_dnsservers; }
			
			
			Flags getFlags() const { return (Flags) m_flags; }
			OverwriteFlags getOverwriteFlags() const { return (OverwriteFlags) m_overwriteFlags; }
			bool getCanUserEnable() const { return m_canUserEnable; }
			
	};
}

Q_DECLARE_METATYPE(nut::Config)
Q_DECLARE_METATYPE(nut::DeviceConfig)
Q_DECLARE_METATYPE(nut::SelectRule)
Q_DECLARE_METATYPE(nut::SelectConfig)
Q_DECLARE_METATYPE(nut::EnvironmentConfig)
Q_DECLARE_METATYPE(nut::IPv4Config)
Q_DECLARE_METATYPE(QVector< quint32 >);
Q_DECLARE_METATYPE(QVector< QVector< quint32 > >);

#endif
