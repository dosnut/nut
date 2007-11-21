
#include "config.h"
#include "configparser_yacc.h"

extern "C" {
#include <stdio.h>
}

#include "exception.h"
#include "log.h"

void configparserparse(nuts::ConfigParser *cp);
extern FILE *configparserin;

/* sub configuration structures will
 * be deleted by ~Config() !
 */

namespace nuts {
	ConfigParser::ConfigParser(const QString &configFile)
	: m_configFile(configFile), m_config(new libnutcommon::Config()) {
		failed = false;
		configparserin = fopen(m_configFile.toUtf8().constData(), "r");
		if (!configparserin)
			throw Exception(QString("Couldn't open config file '%1'").arg(m_configFile));
		configparserparse(this);
		fclose(configparserin);
		if (failed)
			throw Exception(QString("Invalid config file"));
	}
	
	ConfigParser::~ConfigParser() {
		delete m_config;
	}
	
	void ConfigParser::parseError(int lineNum, const QString &msg) {
		failed = true;
		err << QString("%1:%2: %3").arg(m_configFile).arg(lineNum).arg(msg) << endl;
	}
	
	bool ConfigParser::newDevice(const QString &name) {
		m_def_env = 0; m_curdevconfig = 0;
		if (!m_config->m_devices.contains(name)) {
			m_curdevconfig = new libnutcommon::DeviceConfig();
			m_config->m_devices.insert(name, m_curdevconfig);
			m_curdevconfig->m_environments.push_back(new libnutcommon::EnvironmentConfig(""));
			m_def_env = new local_env_config();
			return true;
		}
		return false;
	}
	
	bool ConfigParser::finishDevice() {
		if (!m_curdevconfig) return true;
		if (!m_curdevconfig->m_environments.count() > 0) return false;
		finishEnvironment(m_curdevconfig->m_environments[0], m_def_env);
		delete m_def_env;
		m_def_env = 0; m_curdevconfig = 0;
		return true;
	}
	
	bool ConfigParser::finishEnvironment(libnutcommon::EnvironmentConfig *envc, local_env_config *l_envc) {
		if ((envc->m_ipv4Interfaces.size() == 0) && (!l_envc->no_def_dhcp)) {
			envc->m_ipv4Interfaces.push_back(new libnutcommon::IPv4Config());
		}
		// Append "select user;" if no select config was given.
		if (envc->m_select.filters.size() == 0) {
			envc->m_select.filters.append(libnutcommon::SelectRule());
		}
		return true;
	}
	
	bool ConfigParser::devDefaultEnvironment() {
		if (!m_curdevconfig) return false;
		if (!m_curdevconfig->m_environments.count() > 0) return false;
		m_curenvconfig = m_curdevconfig->m_environments[0];
		m_cur_env = m_def_env;
		return true;
	}
	
	bool ConfigParser::devEnvironment(const QString &name) {
		m_cur_env = 0; m_curenvconfig = 0;
		if (!m_curdevconfig) return false;
		m_curenvconfig = new libnutcommon::EnvironmentConfig(name);
		m_curdevconfig->m_environments.push_back(m_curenvconfig);
		m_cur_env = new local_env_config();
		return true;
	}
	
	bool ConfigParser::finishEnvironment() {
		if (!m_curenvconfig) return true;
		finishEnvironment(m_curenvconfig, m_cur_env);
		delete m_cur_env;
		m_cur_env = 0; m_curenvconfig = 0;
		return true;
	}
	
	bool ConfigParser::devNoAutoStart() {
		if (!m_curdevconfig) return false;
		m_curdevconfig->m_noAutoStart = true;
		return true;
	}
	
	bool ConfigParser::devWPASuppConfig(const QString &driver, const QString &config) {
		if (!m_curdevconfig) return false;
		m_curdevconfig->m_wpaDriver = driver;
		m_curdevconfig->m_wpaConfigFile = config;
		return true;
	}
	
	bool ConfigParser::envNoDHCP() {
		if (!m_curenvconfig) return false;
		m_cur_env->no_def_dhcp = true;
		return true;
	}
	
	bool ConfigParser::envSelect() {
		if (!m_curenvconfig) return false;
		// Only one select block per environment
		if (m_curenvconfig->m_select.filters.count() != 0) return false;
		m_selBlocks.clear();
		return true;
	}
	
	bool ConfigParser::finishSelect() {
		return true;
	}
	
	bool ConfigParser::envDHCP() {
		m_curipv4config = 0;
		if (!m_curenvconfig) return false;
		if (m_cur_env->m_hasdhcp) return false;
		m_curipv4config = new libnutcommon::IPv4Config();
		return true;
	}
	
	bool ConfigParser::finishDHCP() {
		if (!m_curenvconfig) return false;
		if (!m_curipv4config) return false;
		m_cur_env->m_hasdhcp = true;
		m_curenvconfig->m_ipv4Interfaces.push_back(m_curipv4config);
		m_curipv4config = 0;
		return true;
	}
	
	bool ConfigParser::envZeroconf() {
		m_curipv4config = 0;
		if (!m_curenvconfig) return false;
		if (m_cur_env->m_haszeroconf) return false;
		m_curipv4config = new libnutcommon::IPv4Config(libnutcommon::IPv4Config::DO_ZEROCONF);
		return true;
	}
	
	bool ConfigParser::finishZeroconf() {
		if (!m_curenvconfig) return false;
		if (!m_curipv4config) return false;
		m_cur_env->m_haszeroconf = true;
		m_curenvconfig->m_ipv4Interfaces.push_back(m_curipv4config);
		m_curipv4config = 0;
		return true;
	}
	
	bool ConfigParser::envStatic() {
		m_curipv4config = 0;
		if (!m_curenvconfig) return false;
		m_curipv4config = new libnutcommon::IPv4Config(libnutcommon::IPv4Config::DO_STATIC);
		return true;
	}
	
	bool ConfigParser::finishStatic() {
		if (!m_curenvconfig) return false;
		if (!m_curipv4config) return false;
		m_curenvconfig->m_ipv4Interfaces.push_back(m_curipv4config);
		m_curipv4config = 0;
		return true;
	}
	
	bool ConfigParser::staticUser() {
		if (!m_curipv4config) return false;
		m_curipv4config->m_flags = libnutcommon::IPv4Config::DO_USERSTATIC;
		return true;
	}
	
	bool ConfigParser::staticIP(const QHostAddress &addr) {
		if (!m_curipv4config) return false;
		if (!m_curipv4config->m_static_ip.isNull()) return false;
		m_curipv4config->m_static_ip = addr;
		return true;
	}
	
	bool ConfigParser::staticNetmak(const QHostAddress &addr) {
		if (!m_curipv4config) return false;
		if (!m_curipv4config->m_static_netmask.isNull()) return false;
		m_curipv4config->m_static_netmask = addr;
		return true;
	}
	
	bool ConfigParser::staticGateway(const QHostAddress &addr) {
		if (!m_curipv4config) return false;
		if (!m_curipv4config->m_static_gateway.isNull()) return false;
		m_curipv4config->m_static_gateway = addr;
		return true;
	}
	
	bool ConfigParser::staticDNS(const QHostAddress &addr) {
		if (!m_curipv4config) return false;
		m_curipv4config->m_static_dnsservers.push_back(addr);
		return true;
	}
	
	void ConfigParser::selectAdd(const libnutcommon::SelectRule &rule) {
		m_curenvconfig->m_select.filters.append(rule);
		if (!m_curenvconfig->m_select.blocks.isEmpty()) {
			quint32 filterid = m_curenvconfig->m_select.filters.count() - 1;
			m_curenvconfig->m_select.blocks[m_selBlocks.top()].append(filterid);
		}
	}
	
	bool ConfigParser::selectAndBlock() {
		if (!m_curenvconfig) return false;
		quint32 blockid = m_curenvconfig->m_select.blocks.size();
		selectAdd(libnutcommon::SelectRule(blockid, libnutcommon::SelectRule::SEL_AND_BLOCK));
		m_curenvconfig->m_select.blocks.append(QVector<quint32>());
		m_selBlocks.push(blockid);
		return true;
	}
	
	bool ConfigParser::selectOrBlock() {
		if (!m_curenvconfig) return false;
		quint32 blockid = m_curenvconfig->m_select.blocks.size();
		selectAdd(libnutcommon::SelectRule(blockid, libnutcommon::SelectRule::SEL_OR_BLOCK));
		m_curenvconfig->m_select.blocks.append(QVector<quint32>());
		m_selBlocks.push(blockid);
		return true;
	}
	
	bool ConfigParser::selectBlockEnd() {
		if (!m_curenvconfig) return false;
		m_selBlocks.pop();
		return true;
	}
	
	bool ConfigParser::selectUser() {
		if (!m_curenvconfig) return false;
		selectAdd(libnutcommon::SelectRule());
		return true;
	}
	
	bool ConfigParser::selectARP(const QHostAddress &addr) {
		if (!m_curenvconfig) return false;
		selectAdd(libnutcommon::SelectRule(addr));
		return true;
	}
	
	bool ConfigParser::selectARP(const QHostAddress &addr, const libnutcommon::MacAddress &mac) {
		if (!m_curenvconfig) return false;
		selectAdd(libnutcommon::SelectRule(addr, mac));
		return true;
	}
	
	bool ConfigParser::selectESSID(const QString &essid) {
		if (!m_curenvconfig) return false;
		selectAdd(libnutcommon::SelectRule(essid));
		return true;
	}
};
