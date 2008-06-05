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

namespace nuts {
	class ConfigParser;
}

namespace libnutcommon {
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

namespace libnutcommon {
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
	
	/** @brief Container for all \link DeviceConfig device configs\endlink. A deviceName may contain wildcards (not supported yet).
	 */
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
	
	/** @brief Each device has a list of \link EnvironmentConfig Environments\endlink and some additional config values.
	 *
	 * Each device can have many Environments, but it can only be in one; each
	 * Environment consists of zero, one or more Interfaces.
	 *
	 * Which Environment is taken is controlled by the SelectRules of an Environment.
	 *
	 * For WLAN devices supported by wpa_supplicant you can specify a configfile and a driver (wext by default).
	 */
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
	
	/** @brief Result type of a select test.
	 *
	 */
	class SelectResult {
		public:
			typedef enum {
				False = 0,    //!< Cannot select
				User = 1,     //!< Do select only if user does want it
				NotUser = 2,  //!< Should not happen; "only select if user does not want it..." is not good
				True = 3     //!< Select
			} bool_t;
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
	
	/** @brief A select operation.
	 *
	 */
	class SelectRule {
		protected:
			friend QDBusArgument &operator<< (QDBusArgument &argument, const SelectRule &data);
			friend const QDBusArgument &operator>> (const QDBusArgument &argument, SelectRule &data);

		public:
			typedef enum {
				SEL_USER,       //!< Select if user does want it (=> return SelectResult::User)
				SEL_ARP,        //!< Select if ipAddr is found on the network from the device; if macAddr != 0 it is matched too.
				SEL_ESSID,      //!< Select if in wlan essid;
				SEL_AND_BLOCK,  //!< Select a list of \link SelectRule SelectRules\endlink, results combined with AND
				SEL_OR_BLOCK   //!< Select a list of \link SelectRule SelectRules\endlink, results combined with OR
			} SelectType;
			
			SelectRule() : invert(false), selType(SEL_USER) { }
			SelectRule(const QHostAddress &ipAddr, bool invert = false) : invert(invert), selType(SEL_ARP), ipAddr(ipAddr) { }
			SelectRule(const QHostAddress &ipAddr, const libnutcommon::MacAddress &macAddr, bool invert = false) : invert(invert), selType(SEL_ARP), ipAddr(ipAddr), macAddr(macAddr) { }
			SelectRule(const QString &essid, bool invert = false) : invert(invert), selType(SEL_ESSID), essid(essid) { }
			SelectRule(quint32 block, SelectType blockType, bool invert = false) : invert(invert), selType(blockType), block(block) { }
			
			bool invert;        //!< Invert result; unused for now.
			SelectType selType;
			quint32 block;      //!< Block identifier in SelectConfig for SEL_*_BLOCK
			QString essid;
			QHostAddress ipAddr;
			libnutcommon::MacAddress macAddr;
	};
	
	/** @brief SelectConfig for an environment.
	 *
	 * This structure represents a tree of SelectRules; try qnut for visualization.
	 */
	class SelectConfig {
			friend class nuts::ConfigParser;
			friend QDBusArgument &operator<< (QDBusArgument &argument, const SelectConfig &data);
			friend const QDBusArgument &operator>> (const QDBusArgument &argument, SelectConfig &data);

		public:
			SelectConfig() { }
			
			QVector<SelectRule> filters;         //!< List of \link SelectRule SelectRules\endlink
			//! List of blocks; each block is a list of filter ids.
			//! The type of the block (AND/OR) is specified in the rule for the block
			QVector< QVector<quint32> > blocks;
	};
	
	/** @brief Each EnvironmentConfig of a \link DeviceConfig device\endlink has a list
	 *         of \link IPv4Config interfaces\endlink, which configure the ips.
	 *
	 * And it contains a SelectConfig, which determines which Environment is loaded for a device.
	 */
	class EnvironmentConfig {
		private:
			internal::CopyMark m_isCopy;
		protected:
			friend class nuts::ConfigParser;
			friend QDBusArgument &operator<< (QDBusArgument &argument, const EnvironmentConfig &data);
			friend const QDBusArgument &operator>> (const QDBusArgument &argument, EnvironmentConfig &data);
			
			QString m_name;
			QString m_ppp_start_command;
			QString m_ppp_stop_command;
			QList<IPv4Config*> m_ipv4Interfaces;
			SelectConfig m_select;
			
		public:
			EnvironmentConfig(const QString &name = "");
			virtual ~EnvironmentConfig();
			
			QString getName() { return m_name; } //!< A description for that environment. It does not have to be unique.
			const QList<IPv4Config*>& getIPv4Interfaces() { return m_ipv4Interfaces; }
			const SelectConfig &getSelect() { return m_select; }
	};
	
	/** @brief Each IPv4Config stands for one ip of an interface.
	 *
	 * There are several methods how to to this, and some
	 * additional values: netmask, gateway, dns-servers.
	 *
	 * If the interface has the method IPv4Config::DO_STATIC,
	 * the configured values for ip/netmask/... can be queried
	 * with the corresponding functions.
	 */
	class IPv4Config {
		public:
			/** @brief Selects which method is used to determine the ip address.
			 *
			 * Not all flags can be set:
			 *   assert(USERSTATIC xor (DHCP or (ZEROCONF xor STATIC)))
			 */
			typedef enum {
				DO_DHCP      = 1,   //!< Find ip/gateway/dns via DHCP
				DO_ZEROCONF  = 2,   //!< Probe for ip in the IPv4 Link-Local range (RFC 3927).
				DO_STATIC    = 4,   //!< Use values from config file.
				DO_USERSTATIC = 8	//!< Use values specified at runtime by a user
			} Flags;
			
			/** @brief Unused/Unsupported. Could be used to overwrite some value with static configured ones.
			 *
			 */
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
			int m_timeout;
		
		public:	
			IPv4Config(int flags = IPv4Config::DO_DHCP | IPv4Config::DO_ZEROCONF, int overwriteFlags = 0);
			
			const QHostAddress& getStaticIP() const { return m_static_ip; }
			const QHostAddress& getStaticNetmask() const { return m_static_netmask; }
			const QHostAddress& getStaticGateway() const { return m_static_gateway; }
			const QList<QHostAddress>& getStaticDNS() const { return m_static_dnsservers; }
			
			Flags getFlags() const { return (Flags) m_flags; }
			OverwriteFlags getOverwriteFlags() const { return (OverwriteFlags) m_overwriteFlags; }
			int getTimeOut() const { return m_timeout; }
	};
	
	/** @brief If an interface has to be configured by the user (IPv4Config::DO_USERSTATIC), he/she has to
	 *         set that information with this class.
	 *
	 */
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
			
			/** @brief A very basic check if the configuration is valid.
			 */
			bool valid() {
				return !m_ip.isNull();
			}
	};
}

Q_DECLARE_METATYPE(libnutcommon::Config)
Q_DECLARE_METATYPE(libnutcommon::DeviceConfig)
Q_DECLARE_METATYPE(libnutcommon::SelectResult)
Q_DECLARE_METATYPE(QVector< libnutcommon::SelectResult >)
Q_DECLARE_METATYPE(libnutcommon::SelectRule)
Q_DECLARE_METATYPE(libnutcommon::SelectConfig)
Q_DECLARE_METATYPE(libnutcommon::EnvironmentConfig)
Q_DECLARE_METATYPE(libnutcommon::IPv4Config)
Q_DECLARE_METATYPE(libnutcommon::IPv4UserConfig)

#endif
