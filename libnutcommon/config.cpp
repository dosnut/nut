#include "common.h"
#include "enum.h"

namespace libnutcommon {
	bool DeviceNamePattern::operator<(const DeviceNamePattern& other) const {
		return m_type < other.m_type || (m_type == other.m_type &&
			m_pattern < other.m_pattern
			);
	}

	bool DeviceNamePattern::match(QString const& name) const {
		switch (m_type) {
		case DeviceNamePatternType::Plain:
			return m_pattern == name;
		case DeviceNamePatternType::RegExp:
			return QRegExp{ m_pattern, Qt::CaseSensitive, QRegExp::RegExp }.exactMatch(name);
		case DeviceNamePatternType::Wildcard:
			return QRegExp{ m_pattern, Qt::CaseSensitive, QRegExp::Wildcard }.exactMatch(name);
		}
		return false;
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
