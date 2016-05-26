#include "common.h"
#include "enum.h"

namespace libnutcommon {
	QDBusArgument& operator<<(QDBusArgument& argument, DeviceConfig const& data) {
		argument.beginStructure();
		argument
			<< data.noAutoStart
			<< data.wpaConfigFile
			<< data.wpaDriver
			<< data.gatewayMetric;
		argument.beginArray(qMetaTypeId<EnvironmentConfig>());
		for(auto const& ec: data.environments) {
			argument << *ec;
		}
		argument.endArray();
		argument.endStructure();
		return argument;
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, DeviceConfig& data) {
		argument.beginStructure();
		argument
			>> data.noAutoStart
			>> data.wpaConfigFile
			>> data.wpaDriver
			>> data.gatewayMetric;
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

	QDBusArgument& operator<<(QDBusArgument& argument, SelectResult selectResult) {
		return dbusSerializeEnum(argument, selectResult);
	}
	const QDBusArgument& operator>>(const QDBusArgument& argument, SelectResult& selectResult) {
		return dbusUnserializeEnum(argument, selectResult);
	}

	QDBusArgument& operator<<(QDBusArgument& argument, SelectType selectType) {
		return dbusSerializeEnum(argument, selectType);
	}
	const QDBusArgument& operator>>(const QDBusArgument& argument, SelectType& selectType) {
		return dbusUnserializeEnum(argument, selectType);
	}

	QDBusArgument& operator<<(QDBusArgument& argument, SelectRule const& data) {
		argument.beginStructure();
		argument
			<< data.invert
			<< data.selType
			<< data.block
			<< data.essid
			<< data.ipAddr
			<< data.macAddr;
		argument.endStructure();
		return argument;
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, SelectRule& data) {
		argument.beginStructure();
		argument
			>> data.invert
			>> data.selType
			>> data.block
			>> data.essid
			>> data.ipAddr
			>> data.macAddr;
		argument.endStructure();
		return argument;
	}

	QDBusArgument& operator<<(QDBusArgument& argument, SelectConfig const& data) {
		argument.beginStructure();
		argument
			<< data.filters
			<< data.blocks;
		argument.endStructure();
		return argument;
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, SelectConfig& data) {
		argument.beginStructure();
		argument
			>> data.filters
			>> data.blocks;
		argument.endStructure();
		return argument;
	}

	QDBusArgument& operator<<(QDBusArgument& argument, EnvironmentConfig const& data) {
		argument.beginStructure();
		argument
			<< data.name
			<< data.select;
		argument.beginArray( qMetaTypeId<IPv4Config>() );
		for(auto const& ic: data.ipv4Interfaces) {
			argument << *ic;
		}
		argument.endArray();
		argument.endStructure();
		return argument;
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, EnvironmentConfig& data) {
		argument.beginStructure();
		argument
			>> data.name
			>> data.select;
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

	QDBusArgument& operator<<(QDBusArgument& argument, IPv4ConfigFlags flags) {
		return dbusSerializeFlags(argument, flags);
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, IPv4ConfigFlags& flags) {
		return dbusUnserializeFlags(argument, flags);
	}

	QDBusArgument& operator<<(QDBusArgument& argument, IPv4Config const& data) {
		argument.beginStructure();
		argument
			<< data.flags
			<< data.static_ip
			<< data.static_netmask
			<< data.static_gateway
			<< data.static_dnsservers
			<< data.gatewayMetric;
		argument.endStructure();
		return argument;
	}
	QDBusArgument const& operator>>(QDBusArgument const& argument, IPv4Config& data) {
		argument.beginStructure();
		argument
			>> data.flags
			>> data.static_ip
			>> data.static_netmask
			>> data.static_gateway
			>> data.static_dnsservers
			>> data.gatewayMetric;
		argument.endStructure();
		return argument;
	}

	bool DeviceNamePattern::operator<(const DeviceNamePattern& other) const {
		return type < other.type || (type == other.type &&
			pattern < other.pattern
			);
	}

	bool DeviceNamePattern::match(QString const& name) const {
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

	std::shared_ptr<DeviceConfig> Config::create(DeviceNamePattern const& pattern) {
		return namedDeviceConfigs.emplace(pattern, std::make_shared<DeviceConfig>()).first->second;
	}

	std::shared_ptr<DeviceConfig> Config::lookup(QString const& deviceName) {
		for (auto const& ndc: namedDeviceConfigs) {
			if (ndc.first.match(deviceName)) return ndc.second;
		}
		return { };
	}

	bool evaluate(SelectResult a, bool user) {
		bool result[] = { true, user, !user, false };
		return result[(quint8) a];
	}

	SelectResult operator||(SelectResult a, SelectResult b) {
		using SR = SelectResult;
		auto ai = static_cast<quint8>(a);
		auto bi = static_cast<quint8>(b);
		static const SR op_or[16] = {
			SR::False  , SR::User , SR::NotUser, SR::True,
			SR::User   , SR::User , SR::True   , SR::True,
			SR::NotUser, SR::True , SR::NotUser, SR::True,
			SR::True   , SR::True , SR::True   , SR::True
		};
		return op_or[ai*4 + bi];
	}

	SelectResult operator&&(SelectResult a, SelectResult b) {
		using SR = SelectResult;
		auto ai = static_cast<quint8>(a);
		auto bi = static_cast<quint8>(b);
		static const SR op_and[16] = {
			SR::False  , SR::False, SR::False  , SR::False,
			SR::False  , SR::User , SR::False  , SR::User,
			SR::False  , SR::False, SR::NotUser, SR::NotUser,
			SR::False  , SR::User , SR::NotUser, SR::True
		};
		return op_and[ai*4 + bi];
	}

	SelectResult operator^(SelectResult a, SelectResult b) {
		using SR = SelectResult;
		auto ai = static_cast<quint8>(a);
		auto bi = static_cast<quint8>(b);
		static const SR op_xor[16] = {
			SR::False  , SR::User   , SR::NotUser, SR::True,
			SR::User   , SR::False  , SR::True   , SR::NotUser,
			SR::NotUser, SR::True   , SR::False  , SR::User,
			SR::True   , SR::NotUser, SR::User   , SR::False
		};
		return op_xor[ai*4 + bi];
	}

	SelectResult operator!(SelectResult a) {
		auto ai = static_cast<quint8>(a);
		return static_cast<SelectResult>(3 - ai);
	}

	// called by common.cpp: init()
	void config_init() {
		qRegisterMetaType<DeviceConfig>("libnutcommon::DeviceConfig");
		qRegisterMetaType<SelectResult>("libnutcommon::SelectResult");
		qRegisterMetaType<QVector<SelectResult>>("QVector<libnutcommon::SelectRule>");
		qRegisterMetaType<SelectType>("libnutcommon::SelectType");
		qRegisterMetaType<SelectRule>("libnutcommon::SelectRule");
		qRegisterMetaType<SelectConfig>("libnutcommon::SelectConfig");
		qRegisterMetaType<EnvironmentConfig>("libnutcommon::EnvironmentConfig");
		qRegisterMetaType<IPv4ConfigFlags>("libnutcommon::IPv4ConfigFlags");
		qRegisterMetaType<IPv4Config>("libnutcommon::IPv4Config");

		qDBusRegisterMetaType<DeviceConfig>();
		qDBusRegisterMetaType<SelectResult>();
		qDBusRegisterMetaType<QVector<SelectResult>>();
		qDBusRegisterMetaType<SelectType>();
		qDBusRegisterMetaType<SelectRule>();
		qDBusRegisterMetaType<SelectConfig>();
		qDBusRegisterMetaType<EnvironmentConfig>();
		qDBusRegisterMetaType<IPv4ConfigFlags>();
		qDBusRegisterMetaType<IPv4Config>();
	}
}
