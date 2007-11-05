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
		argument << data.m_wpaConfigFile;
		argument << data.m_wpaDriver;
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
			<< data.m_static_dnsservers;
		
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
			>> data.m_static_dnsservers;
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
			foreach(DeviceConfig* dc, m_devices)
				delete dc;
			m_devices.clear();
		}
	}
	
	DeviceConfig::DeviceConfig()
	: m_noAutoStart(false) {
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
	: m_flags(flags), m_overwriteFlags(overwriteFlags) {
	}
}
