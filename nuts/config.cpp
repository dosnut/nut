
#include "config.h"

extern "C" {
#include <stdio.h>
}

#include "exception.h"
#include "log.h"
#include <QRegExp>

int configparserparse(nuts::ConfigParser *cp);
extern FILE *configparserin;

/* sub configuration structures will
 * be deleted by ~Config() !
 */

#define FALLBACK_TIMEOUT 60

namespace nuts {
	ConfigParser::ConfigParser(const QString &configFile)
	: m_configFile(configFile), m_config(new libnutcommon::Config()), m_curisfallback(false) {
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

	bool ConfigParser::newDevice(const QString &name, bool regexp) {
		using libnutcommon::DeviceNamePattern;

		m_def_env = nullptr;
		m_curdevconfig.reset();

		bool wildcard = (!regexp && (name.contains("?") || name.contains("[") || name.contains("]") || name.contains("*")) );

		QRegExp rx(name);
		if (regexp) {
			rx.setPatternSyntax(QRegExp::RegExp);
			if (!rx.isValid()) {
				throw Exception(QString("Device name is not a valid RegExp: %1").arg(name));
			}
		}
		else {
			rx.setPatternSyntax(QRegExp::Wildcard);
			if (!rx.isValid()) {
				throw Exception(QString("Device Name is not valid: %1").arg(name));
			}
		}

		auto pattern = DeviceNamePattern { name, regexp ? DeviceNamePattern::RegExp : wildcard ? DeviceNamePattern::Wildcard : DeviceNamePattern::Plain };

		if (m_config->namedDeviceConfigs.find(pattern) != m_config->namedDeviceConfigs.end()) {
			throw Exception(QString("Duplicate device name/pattern: %1 (%2)").arg(pattern.pattern).arg(pattern.typeString()));
		}

		m_curdevconfig = m_config->create(pattern);

		m_curdevconfig->environments.push_back(std::make_shared<libnutcommon::EnvironmentConfig>(""));
		m_def_env = new local_env_config();
		return true;
	}

	bool ConfigParser::finishDevice() {
		if (!m_curdevconfig) return true;
		if (!m_curdevconfig->environments.size() > 0) return false;
		finishEnvironment(m_curdevconfig->environments[0], m_def_env);
		delete m_def_env;
		m_def_env = nullptr;
		m_curdevconfig.reset();
		return true;
	}

	bool ConfigParser::finishEnvironment(std::shared_ptr<libnutcommon::EnvironmentConfig> const& envc, local_env_config *l_envc) {
		if ((envc->ipv4Interfaces.size() == 0) && (!l_envc->no_def_dhcp)) {
			envc->ipv4Interfaces.push_back(std::make_shared<libnutcommon::IPv4Config>(libnutcommon::IPv4ConfigFlag::DHCP));
		}
		// Append "select user;" if no select config was given.
		if (envc->select.filters.size() == 0) {
			envc->select.filters.append(libnutcommon::SelectRule());
		}
		return true;
	}

	bool ConfigParser::devDefaultEnvironment() {
		if (!m_curdevconfig) return false;
		if (!m_curdevconfig->environments.size() > 0) return false;
		m_curenvconfig = m_curdevconfig->environments[0];
		m_cur_env = m_def_env;
		return true;
	}

	bool ConfigParser::devEnvironment(const QString &name) {
		m_cur_env = nullptr;
		m_curenvconfig.reset();
		if (!m_curdevconfig) return false;
		m_curenvconfig = std::make_shared<libnutcommon::EnvironmentConfig>(name);
		m_curdevconfig->environments.push_back(m_curenvconfig);
		m_cur_env = new local_env_config();
		return true;
	}

	bool ConfigParser::finishEnvironment() {
		if (!m_curenvconfig) return true;
		finishEnvironment(m_curenvconfig, m_cur_env);
		delete m_cur_env;
		m_cur_env = nullptr;
		m_curenvconfig.reset();
		return true;
	}

	bool ConfigParser::devNoAutoStart() {
		if (!m_curdevconfig) return false;
		m_curdevconfig->noAutoStart = true;
		return true;
	}

	bool ConfigParser::devWPASuppConfig(const QString &driver, const QString &config) {
		if (!m_curdevconfig) return false;
		m_curdevconfig->wpaDriver = driver;
		m_curdevconfig->wpaConfigFile = config;
		return true;
	}

	bool ConfigParser::devMetric(int metric) {
		if (!m_curdevconfig) return false;
		m_curdevconfig->gatewayMetric = metric;
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
		if (m_curenvconfig->select.filters.count() != 0) return false;
		m_selBlocks.clear();
		return true;
	}

	bool ConfigParser::finishSelect() {
		return true;
	}

	bool ConfigParser::envDHCP() {
		m_curipv4config.reset();
		if (!m_curenvconfig) return false;
		if (m_cur_env->m_hasdhcp) return false;
		m_curipv4config = std::make_shared<libnutcommon::IPv4Config>(libnutcommon::IPv4ConfigFlag::DHCP);
		return true;
	}

	bool ConfigParser::finishDHCP() {
		if (!m_curenvconfig) return false;
		if (!m_curipv4config) return false;
		m_cur_env->m_hasdhcp = true;
		m_curenvconfig->ipv4Interfaces.push_back(m_curipv4config);
		m_curipv4config.reset();
		return true;
	}
	bool ConfigParser::envFallback() {
		if (!m_curipv4config) return false;
		m_curisfallback = true;
		//Set timeout to -1 to later check if user has set time out
		m_curipv4config->timeout = -1;
		return true;;
	}
	bool ConfigParser::finishFallback() {
		m_curisfallback = false;
		if (-1 == m_curipv4config->timeout) //user did not set timeout, set default
			m_curipv4config->timeout = FALLBACK_TIMEOUT;
		//TODO:Remove nasty workaround; Priority: mid
		//Description: If fallback time-out is below 7 seconds; timer problems occure: Zerconf does not work anymore
		//(guess: 3 ARPProbe, after that no announce -> no ip is set)
		if (m_curipv4config->timeout < 10) {
			m_curipv4config->timeout = 10;
		}
		//check if interface type was defined, if not, set zeroconf
		if ( !(m_curipv4config->flags & libnutcommon::IPv4ConfigFlag::STATIC ||
			m_curipv4config->flags & libnutcommon::IPv4ConfigFlag::ZEROCONF) ) {
			m_curipv4config->flags |= libnutcommon::IPv4ConfigFlag::ZEROCONF;
		}
		return true;
	}
	bool ConfigParser::envFallbackTimeout(int timeout) {
		if (!m_curenvconfig) return false;
		if (!m_curipv4config) return false;
		if (-1 != m_curipv4config->timeout) return false; //timeout was set before
		m_curipv4config->timeout = timeout;
		return true;
	}

	bool ConfigParser::envFallbackContinueDhcp(bool con) {
		if (!m_curenvconfig) return false;
		if (!m_curipv4config) return false;
		m_curipv4config->continue_dhcp = con;
		return true;
	}

	bool ConfigParser::envZeroconf() {
		if (m_curisfallback) {
			if (!m_curenvconfig) return false;
			if (!m_curipv4config) return false;
			if (libnutcommon::IPv4ConfigFlag::DHCP != m_curipv4config->flags) return false; //Check if this is the first time fallback is configured (dhcp only)
			m_curipv4config->flags = m_curipv4config->flags | libnutcommon::IPv4ConfigFlag::ZEROCONF;
			return true;
		}
		else { //not in fallback, configuring "real" interface
			m_curipv4config.reset();
			if (!m_curenvconfig) return false;
			if (m_cur_env->m_haszeroconf) return false; //TODO:wahts happening here?
			m_curipv4config = std::make_shared<libnutcommon::IPv4Config>(libnutcommon::IPv4ConfigFlag::ZEROCONF);
			return true;
		}
	}

	bool ConfigParser::finishZeroconf() {
		if (m_curisfallback) {
			//is there anything to check?
			return true;
		}
		else {
			if (!m_curenvconfig) return false;
			if (!m_curipv4config) return false;
			m_cur_env->m_haszeroconf = true;
			m_curenvconfig->ipv4Interfaces.push_back(m_curipv4config);
			m_curipv4config.reset();
			return true;
		}
	}

	bool ConfigParser::envStatic() {
		if (m_curisfallback) {
			if (!m_curenvconfig) return false;
			if (!m_curipv4config) return false;
			if (libnutcommon::IPv4ConfigFlag::DHCP != m_curipv4config->flags) return false; //Check if this is the first time fallback is configured (dhcp only)
			m_curipv4config->flags = m_curipv4config->flags | libnutcommon::IPv4ConfigFlag::STATIC;
			return true;
		}
		else {
			m_curipv4config.reset();
			if (!m_curenvconfig) return false;
			m_curipv4config = std::make_shared<libnutcommon::IPv4Config>(libnutcommon::IPv4ConfigFlag::STATIC);
			return true;
		}
	}

	bool ConfigParser::finishStatic() {
		if (m_curisfallback) {
			//anything to check here?
			return true;
		}
		else {
			if (!m_curenvconfig) return false;
			if (!m_curipv4config) return false;
			m_curenvconfig->ipv4Interfaces.push_back(m_curipv4config);
			m_curipv4config.reset();
			return true;
		}
	}

	bool ConfigParser::staticUser() {
		if (!m_curipv4config) return false;
		m_curipv4config->flags = libnutcommon::IPv4ConfigFlag::USERSTATIC;
		return true;
	}

	bool ConfigParser::staticIP(const QHostAddress &addr) {
		if (m_curisfallback) { //TODO:Check if we have to do anything else
			if (!m_curipv4config) return false;
			if (!m_curipv4config->static_ip.isNull()) return false;
			m_curipv4config->static_ip = addr;
			return true;
		}
		else {
			if (!m_curipv4config) return false;
			if (!m_curipv4config->static_ip.isNull()) return false;
			m_curipv4config->static_ip = addr;
			return true;
		}
	}

	bool ConfigParser::staticNetmask(const QHostAddress &addr) {
		if (m_curisfallback) { //TODO:Check if we have to do anything else
			if (!m_curipv4config) return false;
			if (!m_curipv4config->static_netmask.isNull()) return false;
			m_curipv4config->static_netmask = addr;
			return true;
		}
		else {
			if (!m_curipv4config) return false;
			if (!m_curipv4config->static_netmask.isNull()) return false;
			m_curipv4config->static_netmask = addr;
			return true;
		}
	}

	bool ConfigParser::staticGateway(const QHostAddress &addr) {
		if (m_curisfallback) { //TODO:Check if we have to do anything else
			if (!m_curipv4config) return false;
			if (!m_curipv4config->static_gateway.isNull()) return false;
			m_curipv4config->static_gateway = addr;
			return true;
		}
		else {
			if (!m_curipv4config) return false;
			if (!m_curipv4config->static_gateway.isNull()) return false;
			m_curipv4config->static_gateway = addr;
			return true;
		}
	}

	bool ConfigParser::staticDNS(const QHostAddress &addr) {
		if (m_curisfallback) { //TODO:Check if we have to do anything else
			if (!m_curipv4config) return false;
			m_curipv4config->static_dnsservers.push_back(addr);
			return true;
		}
		else {
			if (!m_curipv4config) return false;
			m_curipv4config->static_dnsservers.push_back(addr);
			return true;
		}
	}

	void ConfigParser::selectAdd(const libnutcommon::SelectRule &rule) {
		m_curenvconfig->select.filters.append(rule);
		if (!m_curenvconfig->select.blocks.isEmpty()) {
			quint32 filterid = m_curenvconfig->select.filters.count() - 1;
			m_curenvconfig->select.blocks[m_selBlocks.top()].append(filterid);
		}
	}

	bool ConfigParser::selectAndBlock() {
		if (!m_curenvconfig) return false;
		quint32 blockid = m_curenvconfig->select.blocks.size();
		selectAdd(libnutcommon::SelectRule(blockid, libnutcommon::SelectType::AND_BLOCK));
		m_curenvconfig->select.blocks.append(QVector<quint32>());
		m_selBlocks.push(blockid);
		return true;
	}

	bool ConfigParser::selectOrBlock() {
		if (!m_curenvconfig) return false;
		quint32 blockid = m_curenvconfig->select.blocks.size();
		selectAdd(libnutcommon::SelectRule(blockid, libnutcommon::SelectType::OR_BLOCK));
		m_curenvconfig->select.blocks.append(QVector<quint32>());
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
}
