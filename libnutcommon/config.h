#ifndef NUT_COMMON_CONFIG_H
#define NUT_COMMON_CONFIG_H

#include <memory>
#include <map>

#include <QObject>
#include <QString>
#include <QList>
#include <QVector>
#include <QHostAddress>
#include <QDBusArgument>

#include "macaddress.h"

namespace libnutcommon {
	class Config;
	class DeviceConfig;
	class SelectRule;
	class SelectConfig;
	class EnvironmentConfig;
	class IPv4Config;
}

namespace libnutcommon {
	/** @brief Name, Wildcard or RegExp pattern for a device name
	 * sorts plain value before regexp before wildcard, for use
	 * as a key in std::map; first matching entry should "win".
	 */
	class DeviceNamePattern {
	public:
		enum PatternType {
			Plain = 0,
			RegExp = 1,
			Wildcard = 2,
		};

		QString pattern;
		PatternType type;

		bool operator<(const DeviceNamePattern &other) const;
		bool operator>(const DeviceNamePattern &other) const { return other < *this; }
		bool operator<=(const DeviceNamePattern &other) const { return !(other < *this); }
		bool operator>=(const DeviceNamePattern &other) const { return !(*this < other); }

		/* whether name matches pattern */
		bool match(QString name) const;

		/* human readable "type" */
		QString typeString() const;
	};

	/** @brief Container for all \link DeviceConfig device configs\endlink.
	 */
	class Config {
	public:
		std::map< DeviceNamePattern, std::shared_ptr<DeviceConfig> > namedDeviceConfigs;

		/* create (overwrite) mapping for pattern with a new empty config; return config reference */
		std::shared_ptr<DeviceConfig> create(const DeviceNamePattern& pattern);

		/* lookup first matching pattern and return associated config reference */
		std::shared_ptr<DeviceConfig> lookup(QString deviceName);
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
	public:
		std::vector< std::shared_ptr<EnvironmentConfig> > environments;
		bool noAutoStart = false;
		QString wpaConfigFile;
		QString wpaDriver;
		int gateway_metric = -1;
	};
	QDBusArgument &operator<< (QDBusArgument &argument, const DeviceConfig &data);
	const QDBusArgument &operator>> (const QDBusArgument &argument, DeviceConfig &data);

	/** @brief Result type of a select test.
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
		bool_t m_value;

	public:
		SelectResult(bool_t value = False) : m_value(value) { }

		SelectResult& operator=(bool_t value) {
			m_value = value;
			return *this;
		}
		SelectResult& operator=(const SelectResult &other) {
			m_value = other.m_value;
			return *this;
		}

		operator bool_t () const {
			return m_value;
		}

		SelectResult operator||(const SelectResult &other) const;
		SelectResult operator&&(const SelectResult &other) const;
		SelectResult operator!() const;
	};
	QDBusArgument &operator<< (QDBusArgument &argument, const SelectResult &data);
	const QDBusArgument &operator>> (const QDBusArgument &argument, SelectResult &data);

	/** @brief A select operation.
	 */
	class SelectRule {
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
	QDBusArgument &operator<< (QDBusArgument &argument, const SelectRule &data);
	const QDBusArgument &operator>> (const QDBusArgument &argument, SelectRule &data);

	/** @brief SelectConfig for an environment.
	 *
	 * This structure represents a tree of SelectRules; try qnut for visualization.
	 */
	class SelectConfig {
	public:
		SelectConfig() { }

		QVector<SelectRule> filters;         //!< List of \link SelectRule SelectRules\endlink
		//! List of blocks; each block is a list of filter ids.
		//! The type of the block (AND/OR) is specified in the rule for the block
		QVector< QVector<quint32> > blocks;
	};
	QDBusArgument &operator<< (QDBusArgument &argument, const SelectConfig &data);
	const QDBusArgument &operator>> (const QDBusArgument &argument, SelectConfig &data);

	/** @brief Each EnvironmentConfig of a \link DeviceConfig device\endlink has a list
	 *         of \link IPv4Config interfaces\endlink, which configure the ips.
	 *
	 * And it contains a SelectConfig, which determines which Environment is loaded for a device.
	 */
	class EnvironmentConfig {
	public:
		QString name; //!< A description for that environment. It does not have to be unique.
		std::vector<std::shared_ptr<IPv4Config>> ipv4Interfaces;
		SelectConfig select;

		EnvironmentConfig(const QString &name = "") : name(name) { }
	};
	QDBusArgument &operator<< (QDBusArgument &argument, const EnvironmentConfig &data);
	const QDBusArgument &operator>> (const QDBusArgument &argument, EnvironmentConfig &data);

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
		enum Flag : unsigned int {
			DO_DHCP      = 1,   //!< Find ip/gateway/dns via DHCP
			DO_ZEROCONF  = 2,   //!< Probe for ip in the IPv4 Link-Local range (RFC 3927).
			DO_STATIC    = 4,   //!< Use values from config file.
			DO_USERSTATIC = 8   //!< Use values specified at runtime by a user
		};
		Q_DECLARE_FLAGS(Flags, Flag)

		/** @brief Unused/Unsupported. Could be used to overwrite some value with static configured ones.
		 *
		 */
		enum OverwriteFlag : unsigned int {
			OW_IP        = 1,
			OW_NETMASK   = 2,
			OW_GATEWAY   = 4,
			OW_DNSSERVER = 8
		};
		Q_DECLARE_FLAGS(OverwriteFlags, OverwriteFlag)

		QHostAddress static_ip;
		QHostAddress static_netmask;
		QHostAddress static_gateway;
		QList<QHostAddress> static_dnsservers;

		Flags flags;
		OverwriteFlags overwriteFlags;

		int gateway_metric = -1;
		int timeout = 0;
		bool continue_dhcp = false;

		IPv4Config(int flags = IPv4Config::DO_DHCP | IPv4Config::DO_ZEROCONF, int overwriteFlags = 0) : flags(flags), overwriteFlags(overwriteFlags) { }
	};
	QDBusArgument &operator<< (QDBusArgument &argument, const IPv4Config &data);
	const QDBusArgument &operator>> (const QDBusArgument &argument, IPv4Config &data);
	Q_DECLARE_OPERATORS_FOR_FLAGS(IPv4Config::Flags)
	Q_DECLARE_OPERATORS_FOR_FLAGS(IPv4Config::OverwriteFlags)

	/** @brief If an interface has to be configured by the user (IPv4Config::DO_USERSTATIC), he/she has to
	 *         set that information with this class.
	 *
	 */
	class IPv4UserConfig {
	public:
		QHostAddress ip;
		QHostAddress netmask;
		QHostAddress gateway;
		QList<QHostAddress> dnsservers;

		/** @brief A very basic check if the configuration is valid.
		 */
		bool valid() {
			return !ip.isNull();
		}
	};
	QDBusArgument &operator<< (QDBusArgument &argument, const IPv4UserConfig &data);
	const QDBusArgument &operator>> (const QDBusArgument &argument, IPv4UserConfig &data);
}

Q_DECLARE_METATYPE(libnutcommon::DeviceConfig)
Q_DECLARE_METATYPE(libnutcommon::SelectResult)
Q_DECLARE_METATYPE(QVector< libnutcommon::SelectResult >)
Q_DECLARE_METATYPE(libnutcommon::SelectRule)
Q_DECLARE_METATYPE(libnutcommon::SelectConfig)
Q_DECLARE_METATYPE(libnutcommon::EnvironmentConfig)
Q_DECLARE_METATYPE(libnutcommon::IPv4Config)
Q_DECLARE_METATYPE(libnutcommon::IPv4UserConfig)

#endif
