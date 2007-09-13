//
// C++ Implementation: config
//
// Description: 
//
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "config.h"

namespace nut {
	QDBusArgument &operator<< (QDBusArgument &argument, const Config &data) {
		argument.beginStructure();
		argument.beginMap( QVariant::String, qMetaTypeId<DeviceConfig>() );
		QHashIterator<QString, DeviceConfig*> i(data.m_devices);
		while (i.hasNext()) {
			i.next();
			argument.beginMapEntry();
			argument << i.key() << *i.value();
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
			data.m_devices.insert(name, dc);
		}
		argument.endMap();
		argument.endStructure();
		return argument;
	}
	
	QDBusArgument &operator<< (QDBusArgument &argument, const DeviceConfig &data) {
		argument.beginStructure();
		argument << data.m_noAutoStart;
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
	
	QDBusArgument &operator<< (QDBusArgument &argument, const SelectRule &data) {
		argument.beginStructure();
		argument << (quint8) data.selType << data.block << data.essid << data.ipAddr << data.macAddr;
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, SelectRule &data) {
		argument.beginStructure();
		quint8 selType;
		argument >> selType >> data.block >> data.essid >> data.ipAddr >> data.macAddr;
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
		argument << data.m_name << data.m_canUserSelect << data.m_noDefaultDHCP << data.m_noDefaultZeroconf << data.m_select;
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
		argument >> data.m_name >> data.m_canUserSelect >> data.m_noDefaultDHCP >> data.m_noDefaultZeroconf >> data.m_select;
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
		argument << (quint32) data.getFlags() << (quint32) data.getOverwriteFlags();
		argument.endStructure();
		return argument;
	}
	const QDBusArgument &operator>> (const QDBusArgument &argument, IPv4Config &data) {
		argument.beginStructure();
		quint32 flags, oFlags;
		argument >> flags >> oFlags;
		data.m_flags = (IPv4Config::Flags) flags;
		data.m_overwriteFlags = (IPv4Config::OverwriteFlags) oFlags;
		argument.endStructure();
		return argument;
	}
	
	Config::Config()
	: m_isCopy(false) {
	}
	Config::Config(const Config &other)
	: m_isCopy(true), m_devices(other.m_devices) {
	}
	
	Config::~Config() {
		if (!m_isCopy)
			foreach(DeviceConfig* dc, m_devices)
				delete dc;
		m_devices.clear();
	}
	
	DeviceConfig* Config::getDevice(const QString &deviceName) {
		if (!m_devices.contains(deviceName)) {
			return 0;
		} else {
			return m_devices[deviceName];
		}
	}

	DeviceConfig::DeviceConfig()
	: m_isCopy(false), m_noAutoStart(false) {
	}
	
	DeviceConfig::DeviceConfig(const DeviceConfig &other)
	: m_isCopy(true), m_environments(other.m_environments), m_noAutoStart(other.m_noAutoStart) {
	}

	
	DeviceConfig::~DeviceConfig() {
		if (!m_isCopy)
			foreach(EnvironmentConfig* ec, m_environments)
				delete ec;
		m_environments.clear();
	}
	
	EnvironmentConfig::EnvironmentConfig(const QString &name)
	: m_isCopy(false), m_name(name), m_canUserSelect(true), m_noDefaultDHCP(false), m_noDefaultZeroconf(false), m_dhcp(0), m_zeroconf(0) {
	}
	EnvironmentConfig::EnvironmentConfig(const EnvironmentConfig& other)
	: m_isCopy(true), m_name(other.m_name), m_canUserSelect(other.m_canUserSelect), m_noDefaultDHCP(other.m_noDefaultDHCP), m_noDefaultZeroconf(other.m_noDefaultZeroconf), m_dhcp(0), m_zeroconf(0) {
	}
	
	EnvironmentConfig::~EnvironmentConfig() {
		if (!m_isCopy)
			foreach(IPv4Config *ic, m_ipv4Interfaces)
				delete ic;
		m_ipv4Interfaces.clear();
	}
	
	IPv4Config::IPv4Config(int flags, int overwriteFlags)
	: m_flags(flags), m_overwriteFlags(overwriteFlags), m_canUserEnable(false) {
	}
}
