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

#ifndef NUT_COMMON_CONFIG_H
#define NUT_COMMON_CONFIG_H

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
	namespace internal {
		/**
		 * @brief Used as a member in other classes, it determines wheter that class is a original or a copy.
		 *
		 * On normal construct (-> default constructor) without params,
		 * the instance evals to False (no copy)
		 * On copy, it changes to true.
		 * So obviously this breaks the idea that a copy is equal to the original.
		 * This helps you to build copy-constructor (or to remove them), and do
		 * some cleanups only in the original destructor.
		 *
		 * In a ideal world we would never copy objects which need this behaviour,
		 * but in conjunction with the Qt DBus layer this is helpful.
		 */
		class CopyMark {
			private:
				bool m_isCopy;
				CopyMark(bool isCopy) : m_isCopy(isCopy) { }
			public:
				CopyMark() : m_isCopy(false) { }
				CopyMark(const CopyMark &) : m_isCopy(true) { }
				operator bool () { return m_isCopy; }
				
				static CopyMark Copy() { return CopyMark(true); }
				static CopyMark NoCopy() { return CopyMark(false); }
		};
	}
	
	class Config {
		private:
			internal::CopyMark m_isCopy;
		
		protected:
			friend class nuts::ConfigParser;
			friend QDBusArgument &operator<< (QDBusArgument &argument, const Config &data);
			friend const QDBusArgument &operator>> (const QDBusArgument &argument, Config &data);
			
			QHash<QString, DeviceConfig*> m_devices;
		
		public:
			Config();
			virtual ~Config();
			
			DeviceConfig* getDevice(const QString &deviceName) {
				return m_devices.value(deviceName, 0);
			}
			
			const QHash<QString, DeviceConfig*> &getDevices() {
				return m_devices;
			}
	};
	
	class DeviceConfig {
		private:
			internal::CopyMark m_isCopy;
		
		protected:
			friend class nuts::ConfigParser;
			friend QDBusArgument &operator<< (QDBusArgument &argument, const DeviceConfig &data);
			friend const QDBusArgument &operator>> (const QDBusArgument &argument, DeviceConfig &data);
			
			QList<EnvironmentConfig*> m_environments;
			bool m_noAutoStart;
			QString m_wpaConfigFile;
			QString m_wpaDriver;
		
		public:
			DeviceConfig();
			virtual ~DeviceConfig();
			
			const QList<EnvironmentConfig*>& getEnvironments() {
				return m_environments;
			}
			
			bool noAutoStart() { return m_noAutoStart; }
			
			QString wpaConfigFile() { return m_wpaConfigFile; }
			QString wpaDriver() { return m_wpaDriver; }
	};
	
	class SelectResult {
		public:
			typedef enum { False = 0, User = 1, NotUser = 2, True = 3 } bool_t;
		protected:
			friend QDBusArgument &operator<< (QDBusArgument &argument, const SelectResult &data);
			friend const QDBusArgument &operator>> (const QDBusArgument &argument, SelectResult &data);
			
			bool_t m_value;
			
		public:
			SelectResult(bool_t value = False)
			: m_value(value) { }
			
			SelectResult& operator = (bool_t value) {
				m_value = value;
				return *this;
			}
			SelectResult& operator = (const SelectResult &other) {
				m_value = other.m_value;
				return *this;
			}
			SelectResult& operator = (qint8 value) {
				m_value = (bool_t) value;
				return *this;
			}
			
			SelectResult operator || (const SelectResult &other) {
				const bool_t op_or[16] = {
					False  , User , NotUser, True,
					User   , User , True   , True,
					NotUser, True , NotUser, True,
					True   , True , True   , True
				};
				return op_or[m_value*4 + other.m_value];
			}
			SelectResult operator && (const SelectResult &other) {
				const bool_t op_and[16] = {
					False  , False, False  , False,
					False  , User , False  , User,
					False  , False, NotUser, NotUser,
					False  , User , NotUser, True
				};
				return op_and[m_value*4 + other.m_value];
			}
			
			operator bool_t () const {
				return m_value;
			}
			
			operator qint8 () const {
				return (qint8) m_value;
			}
			
			SelectResult operator !() const {
				return (SelectResult::bool_t) (3 - m_value);
			}
	};
	
	class SelectRule {
		protected:
			friend QDBusArgument &operator<< (QDBusArgument &argument, const SelectRule &data);
			friend const QDBusArgument &operator>> (const QDBusArgument &argument, SelectRule &data);

		public:
			typedef enum { SEL_USER, SEL_ARP, SEL_ESSID, SEL_AND_BLOCK, SEL_OR_BLOCK } SelectType;
			
			SelectRule() : invert(false), selType(SEL_USER) { }
			SelectRule(const QHostAddress &ipAddr, bool invert = false) : invert(invert), selType(SEL_ARP), ipAddr(ipAddr) { }
			SelectRule(const QHostAddress &ipAddr, const nut::MacAddress &macAddr, bool invert = false) : invert(invert), selType(SEL_ARP), ipAddr(ipAddr), macAddr(macAddr) { }
			SelectRule(const QString &essid, bool invert = false) : invert(invert), selType(SEL_ESSID), essid(essid) { }
			SelectRule(quint32 block, SelectType blockType, bool invert = false) : invert(invert), selType(blockType), block(block) { }
			
			bool invert;
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
			
			// List of filters; the first rule (if it exists) always references the global block.
			QVector<SelectRule> filters;
			// List of blocks; each block is a list of fileder ids. The type of the block (AND/OR) is specified in the rule
			QVector< QVector<quint32> > blocks;
	};
	
	class EnvironmentConfig {
		private:
			internal::CopyMark m_isCopy;
		protected:
			friend class nuts::ConfigParser;
			friend QDBusArgument &operator<< (QDBusArgument &argument, const EnvironmentConfig &data);
			friend const QDBusArgument &operator>> (const QDBusArgument &argument, EnvironmentConfig &data);
			
			QString m_name;
			QList<IPv4Config*> m_ipv4Interfaces;
			SelectConfig m_select;
			
		public:
			EnvironmentConfig(const QString &name = "");
			virtual ~EnvironmentConfig();
			
			QString getName() { return m_name; }
			const QList<IPv4Config*>& getIPv4Interfaces() { return m_ipv4Interfaces; }
			const SelectConfig &getSelect() { return m_select; }
	};
	
	class IPv4Config {
		public:
			// assert(USERSTATIC xor (DHCP or (ZEROCONF xor STATIC)))
			typedef enum {
				DO_DHCP      = 1,
				DO_ZEROCONF  = 2,
				DO_STATIC    = 4,
				DO_USERSTATIC = 8,	// Exclusive
			} Flags;
			
			typedef enum {
				OW_IP        = 1,
				OW_NETMASK   = 2,
				OW_GATEWAY   = 4,
				OW_DNSSERVER = 8,
			} OverwriteFlags;
		
		protected:
			friend class nuts::ConfigParser;
			friend QDBusArgument &operator<< (QDBusArgument &argument, const IPv4Config &data);
			friend const QDBusArgument &operator>> (const QDBusArgument &argument, IPv4Config &data);
			
			QHostAddress m_static_ip, m_static_netmask, m_static_gateway;
			QList<QHostAddress> m_static_dnsservers;
			
			int m_flags;
			int m_overwriteFlags;
		
		public:	
			IPv4Config(int flags = IPv4Config::DO_DHCP | IPv4Config::DO_ZEROCONF, int overwriteFlags = 0);
			
			const QHostAddress& getStaticIP() const { return m_static_ip; }
			const QHostAddress& getStaticNetmask() const { return m_static_netmask; }
			const QHostAddress& getStaticGateway() const { return m_static_gateway; }
			const QList<QHostAddress>& getStaticDNS() const { return m_static_dnsservers; }
			
			Flags getFlags() const { return (Flags) m_flags; }
			OverwriteFlags getOverwriteFlags() const { return (OverwriteFlags) m_overwriteFlags; }
	};
	
	class IPv4UserConfig {
		protected:
			friend QDBusArgument &operator<< (QDBusArgument &argument, const IPv4UserConfig &data);
			friend const QDBusArgument &operator>> (const QDBusArgument &argument, IPv4UserConfig &data);
			
			QHostAddress m_ip, m_netmask, m_gateway;
			QList<QHostAddress> m_dnsservers;
		
		public:
			const QHostAddress& ip() const { return m_ip; }
			bool setIP(const QHostAddress &ip) { m_ip = ip; return true; }
		
			const QHostAddress& netmask() const { return m_netmask; }
			bool setNetmask(const QHostAddress &netmask) { m_netmask = netmask; return true; }
		
			const QHostAddress& gateway() const { return m_gateway; }
			bool setGateway(const QHostAddress &gateway) { m_gateway = gateway; return true; }
		
			const QList<QHostAddress>& dnsservers() const { return m_dnsservers; }
			bool setDnsservers(const QList<QHostAddress>& dnsservers) { m_dnsservers = dnsservers; return true; }
			
			bool valid() {
				return !m_ip.isNull();
			}
	};
}

Q_DECLARE_METATYPE(nut::Config);
Q_DECLARE_METATYPE(nut::DeviceConfig);
Q_DECLARE_METATYPE(nut::SelectResult);
Q_DECLARE_METATYPE(nut::SelectRule);
Q_DECLARE_METATYPE(nut::SelectConfig);
Q_DECLARE_METATYPE(nut::EnvironmentConfig);
Q_DECLARE_METATYPE(nut::IPv4Config);
Q_DECLARE_METATYPE(nut::IPv4UserConfig);
Q_DECLARE_METATYPE(QVector< quint32 >);
Q_DECLARE_METATYPE(QVector< QVector< quint32 > >);

#endif
