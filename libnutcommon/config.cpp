#include "common.h"

namespace libnutcommon {
	QDBusArgument &operator<< (QDBusArgument &argument, const Config &data) {
		argument.beginStructure();
		argument.beginMap( QVariant::String, qMetaTypeId<DeviceConfig>() );
		QListIterator<QString> name(data.m_devNames);
		QListIterator<DeviceConfig*> config(data.m_devConfigs);
		while (name.hasNext() && config.hasNext()) {
			argument.beginMapEntry();
			argument << name.next() << *config.next();
			argument.endMapEntry();
		}
		argument.endMap();
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, Config &data) {
		argument.beginStructure();
		argument.beginMap();
		while (!argument.atEnd()) {
			QString name;
			DeviceConfig *dc = new DeviceConfig();
			argument.beginMapEntry();
			argument >> name >> *dc;
			argument.endMapEntry();
			data.m_devNames.append(name);
			data.m_devConfigs.append(dc);
		}
		argument.endMap();
		argument.endStructure();
		return argument;
	}
	
	QDBusArgument &operator<< (QDBusArgument &argument, const DeviceConfig &data) {
		argument.beginStructure();
		argument << data.m_noAutoStart;
		argument << data.m_wpaConfigFile;
		argument << data.m_wpaDriver;
		argument << data.m_isRegExp;
		argument << data.m_gateway_metric;
		argument.beginArray( qMetaTypeId<EnvironmentConfig>() );
		foreach(EnvironmentConfig* ec, data.m_environments) {
			argument << *ec;
		}
		argument.endArray();
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, DeviceConfig &data) {
		argument.beginStructure();
		argument >> data.m_noAutoStart;
		argument >> data.m_wpaConfigFile;
		argument >> data.m_wpaDriver;
		argument >> data.m_isRegExp;
		argument >> data.m_gateway_metric;
		argument.beginArray();
		while (!argument.atEnd()) {
			EnvironmentConfig *ec = new EnvironmentConfig();
			argument >> *ec;
			data.m_environments.push_back(ec);
		}
		argument.endArray();
		argument.endStructure();
		return argument;
	}
	
	QDBusArgument &operator<< (QDBusArgument &argument, const SelectResult &data) {
		argument.beginStructure();
		argument << (quint8) (qint8) data;
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, SelectResult &data) {
		argument.beginStructure();
		quint8 tmp;
		argument >> tmp;
		data = (qint8) tmp;
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
		argument << data.m_name << data.m_select;
		argument.beginArray( qMetaTypeId<IPv4Config>() );
		foreach(IPv4Config* ic, data.m_ipv4Interfaces) {
			argument << *ic;
		}
		argument.endArray();
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, EnvironmentConfig &data) {
		argument.beginStructure();
		argument >> data.m_name >> data.m_select;
		argument.beginArray();
		while (!argument.atEnd()) {
			IPv4Config *ic = new IPv4Config();
			argument >> *ic;
			data.m_ipv4Interfaces.push_back(ic);
		}
		argument.endArray();
		argument.endStructure();
		return argument;
	}
	
	QDBusArgument &operator<< (QDBusArgument &argument, const IPv4Config &data) {
		argument.beginStructure();
		argument << (quint32) data.getFlags() << (quint32) data.getOverwriteFlags()
			<< data.m_static_ip
			<< data.m_static_netmask
			<< data.m_static_gateway
			<< data.m_static_dnsservers
			<< data.m_gateway_metric;
		
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, IPv4Config &data) {
		argument.beginStructure();
		quint32 flags, oFlags;
		argument >> flags >> oFlags;
		data.m_flags = (IPv4Config::Flags) flags;
		data.m_overwriteFlags = (IPv4Config::OverwriteFlags) oFlags;
		argument
			>> data.m_static_ip
			>> data.m_static_netmask
			>> data.m_static_gateway
			>> data.m_static_dnsservers
			>> data.m_gateway_metric;
		argument.endStructure();
		return argument;
	}
	
	QDBusArgument &operator<< (QDBusArgument &argument, const IPv4UserConfig &data) {
		argument.beginStructure();
		argument << data.m_ip << data.m_netmask << data.m_gateway << data.m_dnsservers;
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, IPv4UserConfig &data) {
		argument.beginStructure();
		argument >> data.m_ip >> data.m_netmask >> data.m_gateway >> data.m_dnsservers;
		argument.endStructure();
		return argument;
	}
	
	Config::Config() {
	}
	
	Config::~Config() {
		if (!m_isCopy) {
			foreach(DeviceConfig* dc, m_devConfigs)
				delete dc;
			m_devConfigs.clear();
		}
	}
	
	DeviceConfig::DeviceConfig()
	: m_noAutoStart(false), m_isRegExp(false), m_gateway_metric(-1) {
	}
	
	DeviceConfig::~DeviceConfig() {
		if (!m_isCopy) {
			foreach(EnvironmentConfig* ec, m_environments)
				delete ec;
			m_environments.clear();
		}
	}
	
	EnvironmentConfig::EnvironmentConfig(const QString &name)
	: m_name(name) {
	}
	EnvironmentConfig::~EnvironmentConfig() {
		if (!m_isCopy) {
			foreach(IPv4Config *ic, m_ipv4Interfaces)
				delete ic;
			m_ipv4Interfaces.clear();
		}
	}
	
	IPv4Config::IPv4Config(int flags, int overwriteFlags)
	: m_gateway_metric(-1), m_flags(flags), m_overwriteFlags(overwriteFlags), m_timeout(0), m_continue_dhcp(false) {
	}

	// called by common.cpp: init()
	void config_init() {
		qRegisterMetaType< Config >("libnutcommon::Config");
		qRegisterMetaType< DeviceConfig >("libnutcommon::DeviceConfig");
		qRegisterMetaType< SelectResult >("libnutcommon::SelectResult");
		qRegisterMetaType< QVector< SelectResult > >("QVector<libnutcommon::SelectRule>");
		qRegisterMetaType< SelectRule >("libnutcommon::SelectRule");
		qRegisterMetaType< SelectConfig >("libnutcommon::SelectConfig");
		qRegisterMetaType< EnvironmentConfig >("libnutcommon::EnvironmentConfig");
		qRegisterMetaType< IPv4Config >("libnutcommon::IPv4Config");
		qRegisterMetaType< IPv4UserConfig >("libnutcommon::IPv4UserConfig");
	
		qDBusRegisterMetaType< Config >();
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
