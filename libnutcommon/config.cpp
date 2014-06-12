#include "common.h"

namespace libnutcommon {
	QDBusArgument &operator<< (QDBusArgument &argument, const DeviceConfig &data) {
		argument.beginStructure();
		argument << data.noAutoStart;
		argument << data.wpaConfigFile;
		argument << data.wpaDriver;
		argument << false; // data.isRegExp got killed
		argument << data.gateway_metric;
		argument.beginArray( qMetaTypeId<EnvironmentConfig>() );
		for(auto const& ec: data.environments) {
			argument << *ec;
		}
		argument.endArray();
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, DeviceConfig &data) {
		argument.beginStructure();
		argument >> data.noAutoStart;
		argument >> data.wpaConfigFile;
		argument >> data.wpaDriver;
		bool deprecated_isRegExp;
		argument >> deprecated_isRegExp;
		argument >> data.gateway_metric;
		argument.beginArray();
		while (!argument.atEnd()) {
			auto ec = std::make_shared<EnvironmentConfig>();
			argument >> *ec;
			data.environments.push_back(std::move(ec));
		}
		argument.endArray();
		argument.endStructure();
		return argument;
	}

	QDBusArgument &operator<< (QDBusArgument &argument, const SelectResult &data) {
		argument.beginStructure();
		argument << (quint8) (SelectResult::bool_t) data;
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, SelectResult &data) {
		argument.beginStructure();
		quint8 tmp;
		argument >> tmp;
		data = (SelectResult::bool_t) tmp;
		argument.endStructure();
		return argument;
	}

	QDBusArgument &operator<< (QDBusArgument &argument, const SelectRule &data) {
		argument.beginStructure();
		argument << data.invert << (quint8) data.selType << data.block << data.essid << data.ipAddr << data.macAddr;
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, SelectRule &data) {
		argument.beginStructure();
		quint8 selType;
		argument >> data.invert >> selType >> data.block >> data.essid >> data.ipAddr >> data.macAddr;
		data.selType = (SelectRule::SelectType) selType;
		argument.endStructure();
		return argument;
	}

	QDBusArgument &operator<< (QDBusArgument &argument, const SelectConfig &data) {
		argument.beginStructure();
		argument << data.filters << data.blocks;
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, SelectConfig &data) {
		argument.beginStructure();
		argument >> data.filters >> data.blocks;
		argument.endStructure();
		return argument;
	}

	QDBusArgument &operator<< (QDBusArgument &argument, const EnvironmentConfig &data) {
		argument.beginStructure();
		argument << data.name << data.select;
		argument.beginArray( qMetaTypeId<IPv4Config>() );
		for(auto const& ic: data.ipv4Interfaces) {
			argument << *ic;
		}
		argument.endArray();
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, EnvironmentConfig &data) {
		argument.beginStructure();
		argument >> data.name >> data.select;
		argument.beginArray();
		while (!argument.atEnd()) {
			auto ic = std::make_shared<IPv4Config>();
			argument >> *ic;
			data.ipv4Interfaces.push_back(std::move(ic));
		}
		argument.endArray();
		argument.endStructure();
		return argument;
	}

	QDBusArgument &operator<< (QDBusArgument &argument, const IPv4Config &data) {
		argument.beginStructure();
		argument << (quint32) data.flags << (quint32) data.overwriteFlags
			<< data.static_ip
			<< data.static_netmask
			<< data.static_gateway
			<< data.static_dnsservers
			<< data.gateway_metric;

		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, IPv4Config &data) {
		argument.beginStructure();
		quint32 flags, oFlags;
		argument >> flags >> oFlags;
		data.flags = (IPv4Config::Flags) flags;
		data.overwriteFlags = (IPv4Config::OverwriteFlags) oFlags;
		argument
			>> data.static_ip
			>> data.static_netmask
			>> data.static_gateway
			>> data.static_dnsservers
			>> data.gateway_metric;
		argument.endStructure();
		return argument;
	}

	QDBusArgument &operator<< (QDBusArgument &argument, const IPv4UserConfig &data) {
		argument.beginStructure();
		argument << data.ip << data.netmask << data.gateway << data.dnsservers;
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, IPv4UserConfig &data) {
		argument.beginStructure();
		argument >> data.ip >> data.netmask >> data.gateway >> data.dnsservers;
		argument.endStructure();
		return argument;
	}

	bool DeviceNamePattern::operator<(const DeviceNamePattern &other) const {
		return type < other.type || (type == other.type &&
			pattern < other.pattern
			);
	}

	bool DeviceNamePattern::match(QString name) const {
		switch (type) {
		case Plain:
			return pattern == name;
		case RegExp:
			return QRegExp{ pattern, Qt::CaseSensitive, QRegExp::RegExp }.exactMatch(name);
		case Wildcard:
			return QRegExp{ pattern, Qt::CaseSensitive, QRegExp::Wildcard }.exactMatch(name);
		}
		return false;
	}

	QString DeviceNamePattern::typeString() const {
		switch (type) {
		case Plain:
			return { "Plain" };
		case RegExp:
			return { "RegExp" };
		case Wildcard:
			return { "Wildcard" };
		}
		return { "" };
	}

	std::shared_ptr<DeviceConfig> Config::create(const DeviceNamePattern& pattern) {
		return namedDeviceConfigs.emplace(pattern, std::make_shared<DeviceConfig>()).first->second;
	}

	std::shared_ptr<DeviceConfig> Config::lookup(QString deviceName) {
		for (auto const& ndc: namedDeviceConfigs) {
			if (ndc.first.match(deviceName)) return ndc.second;
		}
		return { };
	}

	SelectResult SelectResult::operator||(const SelectResult &other) const {
		const bool_t op_or[16] = {
			False  , User , NotUser, True,
			User   , User , True   , True,
			NotUser, True , NotUser, True,
			True   , True , True   , True
		};
		return op_or[m_value*4 + other.m_value];
	}
	SelectResult SelectResult::operator&&(const SelectResult &other) const {
		const bool_t op_and[16] = {
			False  , False, False  , False,
			False  , User , False  , User,
			False  , False, NotUser, NotUser,
			False  , User , NotUser, True
		};
		return op_and[m_value*4 + other.m_value];
	}

	SelectResult SelectResult::operator!() const {
		return (SelectResult::bool_t) (3 - m_value);
	}


	// called by common.cpp: init()
	void config_init() {
		qRegisterMetaType< DeviceConfig >("libnutcommon::DeviceConfig");
		qRegisterMetaType< SelectResult >("libnutcommon::SelectResult");
		qRegisterMetaType< QVector< SelectResult > >("QVector<libnutcommon::SelectRule>");
		qRegisterMetaType< SelectRule >("libnutcommon::SelectRule");
		qRegisterMetaType< SelectConfig >("libnutcommon::SelectConfig");
		qRegisterMetaType< EnvironmentConfig >("libnutcommon::EnvironmentConfig");
		qRegisterMetaType< IPv4Config >("libnutcommon::IPv4Config");
		qRegisterMetaType< IPv4UserConfig >("libnutcommon::IPv4UserConfig");

		qDBusRegisterMetaType< DeviceConfig >();
		qDBusRegisterMetaType< SelectResult >();
		qDBusRegisterMetaType< QVector< SelectResult > >();
		qDBusRegisterMetaType< SelectRule >();
		qDBusRegisterMetaType< SelectConfig >();
		qDBusRegisterMetaType< EnvironmentConfig >();
		qDBusRegisterMetaType< IPv4Config >();
		qDBusRegisterMetaType< IPv4UserConfig >();
	}
}
