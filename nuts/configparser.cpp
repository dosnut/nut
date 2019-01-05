#include "configparser_p.h"
#include "exception.h"
#include "log.h"

/*
Parse basics:

on error search for the next ';' or '}' and continue parsing; make sure
each '{' is matched by a '}' (or you reached EOF) before returning.

returning "false" means the parent parser has to do the continue handling,
returning "true" means the context should be ok, throwing an exception
terminates the parser.

member "failed" is set when any error occured.
*/

using namespace libnutcommon;
using namespace nuts;

namespace {
	QString toString(int t) {
		if (t > 0 && t < 256) return "'" + QString{(unsigned char) t} + "'";
		switch ((config_tokentype) t) {
		case INVALID_TOKEN: return "invalid input token";
		case INVALID_CHAR:  return "invalid input character";
		case ENDOFFILE:     return "end of file";
		case DEVICE:        return "'device'";
		case ENVIRONMENT:   return "'environment'";
		case REGEXP:        return "'regexp'";
		case NOAUTOSTART:   return "'no-auto-start'";
		case DEFAULT:       return "'default'";
		case DHCP:          return "'dhcp'";
		case NODHCP:        return "'no-dhcp'";
		case ZEROCONF:      return "'zeroconf'";
		case NOZEROCONF:    return "'no-zeroconf'";
		case STATIC:        return "'static'";
		case FALLBACK:      return "'fallback'";
		case TIMEOUT:       return "'timeout'";
		case CONTINUEDHCP:  return "'continue-dhcp'";
		case IP:            return "'ip'";
		case NETMASK:       return "'netmask'";
		case GATEWAY:       return "'gateway'";
		case DNSSERVER:     return "'dns-server'";
		case METRIC:        return "'metric'";
		case LABELINDEX:    return "'label-index'";
		case SELECT:        return "'select'";
		case USER:          return "'user'";
		case ARP:           return "'arp'";
		case ESSID:         return "'essid'";
		case AND:           return "'and'";
		case OR:            return "'or'";
		case WLAN:          return "'wlan'";
		case MODE:          return "'mode'";
		case WPASUPPLICANT: return "'wpa-supplicant'";
		case CONFIG:        return "'config'";
		case DRIVER:        return "'driver'";
		case STRING:        return "string value";
		case IPv4_VAL:      return "ipv4 address";
		case MACADDR:       return "mac address";
		case INTEGER:       return "integer value";
		case BOOL:          return "boolean value";
		}
		return "unknown token";
	}

	QString toString(int t, config_tokenvalue const& v) {
		auto s = toString(t);
		switch (t) {
		case INVALID_TOKEN: return s + " '" + v.str + "'";
		case INVALID_CHAR:  return s + " '" + v.str + "'";
		case STRING:        return s + " '" + v.str + "'";
		case IPv4_VAL:      return s + " " + v.addr.toString();
		case MACADDR:       return s + " " + v.macAddr.toString();
		case INTEGER:       return s + " " + QString::number(v.i);
		case BOOL:          return s + (v.b ? " true" : " false");
		}
		return s;
	}

	/* temporary environment options */
	struct local_env_config {
		local_env_config(local_env_config const&) = delete;
		local_env_config& operator=(local_env_config const&) = delete;

		std::shared_ptr<EnvironmentConfig> const real;

		/* has an explicit dhcp interface */
		bool has_dhcp = false;
		/* has an explicit zeroconf interface, or dhcp with zeroconf fallback */
		bool has_zeroconf = false;
		/* had an explicit no-dhcp option */
		bool no_def_dhcp = false;

		explicit local_env_config(QString const& name = "")
		: real(std::make_shared<EnvironmentConfig>(name)) {
		}

		~local_env_config() {
			if (!no_def_dhcp && real->ipv4Interfaces.empty()) {
				real->ipv4Interfaces.push_back(std::make_shared<IPv4Config>(IPv4ConfigFlag::DHCP));
			}
			// Append "select user;" if no select config was given.
			if (real->select.filters.empty()) {
				real->select.filters.append(libnutcommon::SelectRule{});
			}
		}

	};

	struct state {
		Config& m_config;
		QString m_configFile;
		FILE* m_strm = nullptr;
		config_tokenctx m_ctx = nullptr;

		config_lloc lloc;
		int last_token = -1, prev_token = -1;
		config_tokenvalue tv;

		bool failed = false;

		explicit state(Config& config, QString const& configFile) : m_config(config), m_configFile(configFile) {
			m_strm = fopen(m_configFile.toUtf8().constData(), "r");
			if (!m_strm)
				throw Exception(QString("Couldn't open config file '%1'").arg(m_configFile));
			configparserlex_init(&m_ctx);
			configparserset_in(m_strm, m_ctx);
		}

		~state() {
			if (m_ctx) {
				configparserlex_destroy(m_ctx);
				m_ctx = nullptr;
			}
			if (m_strm) {
				fclose(m_strm);
				m_strm = nullptr;
			}
		}

		void error(QString const& msg) {
			err << QString("%1:%2: %3\n").arg(m_configFile).arg(lloc.lineNum).arg(msg);
			failed = true;
		}

		void error_expected(QString const& msg) {
			error("expected " + msg + ", got " + toString(peek(), tv));
			pop();
		}

		int peek() {
			if (-1 == last_token) {
				last_token = configparserlex(&tv, &lloc, m_ctx);
			}
			return last_token;
		}

		void pop() {
			if (-1 == last_token) throw Exception("pop() without peek()");
			prev_token = last_token;
			// stop at EOF, otherwise consume it
			if (ENDOFFILE != last_token) last_token = -1;
		}

		bool eat(int t) {
			if (t != peek()) {
				error_expected(toString(t));
				return false;
			}
			pop();
			return true;
		}

		/* options usually need to be separated by a ';'
		 * as each option can have at most one trailing block, an option that
		 * ends in a block does not need a ';' separator
		 * additional ';' are ignored
		 */
		void searchOptionEnd(bool showError) {
			if (';' == prev_token || '}' == prev_token) {
				/* already have a separator */
				while (';' == peek()) pop(); /* optional */
				return;
			}

			switch (peek()) {
			case ENDOFFILE:
				/* bad style, but we can take it... */
				return;
			case ';':
				pop();
				while (';' == peek()) pop(); /* optional */
				return;
			case '}':
				return;
			}

			if (showError) {
				/* only print error once until next ';' */
				error_expected("';' or '}'");
			}

			/* now handle continuation after error */

			/* count nested blocks */
			unsigned int nested = 0;
			for (;;) {
				/* consume all input but the final closing '}' - so just peek */
				auto t = peek();
				if ('}' == t && 0 == nested) return; // reached final '}' for current block
				pop();
				switch (t) {
				case ENDOFFILE:
					return; // done.
				case ';':
					if (0 == nested) {
						while (';' == peek()) pop(); /* optional */
						return;
					}
					break;
				case '}':
					--nested;
					break;
				case '{':
					++nested;
					break;
				}
			}
		}

		template<typename Option>
		bool parseBlock(Option&& option) {
			switch (peek()) {
			case '{':
				pop();
				for (;;) {
					while (';' == peek()) pop();
					if ('}' == peek()) {
						pop();
						while (';' == peek()) pop();
						return true;
					}
					searchOptionEnd(option());
				}
			case ';':
				pop();
				return true;
			case ENDOFFILE:
				return true;
			default:
				searchOptionEnd(option());
				return true;
			}
		}

		template<typename Handler>
		bool tryIPv4(local_env_config& env, Handler&& handler) {
			auto i = std::make_shared<IPv4Config>(IPv4ConfigFlags{0});
			if (!handler(*i)) return false;
			env.real->ipv4Interfaces.push_back(i);
			return true;
		}

		bool parseStaticOption(IPv4Config& ipv4) {
			QHostAddress* dest;
			auto t = peek();
			switch (t) {
			case IP:
				dest = &ipv4.static_ip;
				break;
			case NETMASK:
				dest = &ipv4.static_netmask;
				break;
			case GATEWAY:
				dest = &ipv4.static_gateway;
				break;
			case DNSSERVER:
				pop();
				if (!eat(IPv4_VAL)) return false;
				ipv4.static_dnsservers.push_back(tv.addr);
				return true;
			default:
				error_expected("'ip', 'netmask', 'gateway' or 'dns-server'");
				return false;
			}
			pop();
			if (!eat(IPv4_VAL)) return false;
			if (!dest->isNull()) {
				error("already have " + toString(t) + " for this interface");
				return false;
			}
			*dest = tv.addr;
			return true;
		}

		bool parseIpv4Metric(IPv4Config& ipv4) {
			if (METRIC != peek()) return true;
			pop();
			if (!eat(INTEGER)) return false;
			if (-1 != ipv4.gatewayMetric) {
				error("metric already set for interface");
				return false;
			}
			ipv4.gatewayMetric = tv.i;
			return true;
		}

		bool parseStatic(IPv4Config& ipv4) {
			if (!eat(STATIC)) return false;
			if (!parseIpv4Metric(ipv4)) return false;
			switch (peek()) {
			case USER:
				pop();
				if (ipv4.flags != IPv4ConfigFlags{0}) {
					error("'static user' not allowed as dhcp fallback interface");
					return false;
				}
				ipv4.flags |= IPv4ConfigFlag::USERSTATIC;
				return true;
			case IP:
				ipv4.flags |= IPv4ConfigFlag::STATIC;
				pop();
				if (!eat(IPv4_VAL)) return false;
				ipv4.static_ip = tv.addr;
				return true;
			case '{':
				ipv4.flags |= IPv4ConfigFlag::STATIC;
				return parseBlock([&]() { return parseStaticOption(ipv4); });
			default:
				error_expected("'ip', 'user' or '{'");
				return false;
			}
		}

		bool parseZeroconf(local_env_config& env, IPv4Config& ipv4) {
			if (!eat(ZEROCONF)) return false;
			if (env.has_zeroconf) {
				error("already have zeroconf in this environment");
				return false;
			}
			ipv4.flags |= IPv4ConfigFlag::ZEROCONF;
			env.has_zeroconf = true;
			if (!parseIpv4Metric(ipv4)) return false;
			return true;
		}

		bool parseDHCPFallback(local_env_config& env, IPv4Config& ipv4) {
			if (!eat(FALLBACK)) return false;
			switch (peek()) {
			case '{':
				{
					bool has_timeout{false}, has_continue_dhcp{false};
					bool result = parseBlock([&]() {
						switch (peek()) {
						case TIMEOUT:
							if (has_timeout) {
								error("already have 'timeout' in this fallback");
								return false;
							}
							has_timeout = true;
							pop();
							if (!eat(INTEGER)) return false;
							ipv4.timeout = tv.i;
							return true;
						case CONTINUEDHCP:
							if (has_continue_dhcp) {
								error("already have 'continue-dhcp' in this fallback");
								return false;
							}
							has_continue_dhcp = true;
							pop();
							if (!eat(BOOL)) return false;
							ipv4.continue_dhcp = tv.b;
							return true;
						case ZEROCONF:
							if (ipv4.flags != IPv4ConfigFlag::DHCP) {
								error("already have a fallback interface");
								pop();
								return false;
							}
							return parseZeroconf(env, ipv4);
						case STATIC:
							if (ipv4.flags != IPv4ConfigFlag::DHCP) {
								error("already have a fallback interface");
								pop();
								return false;
							}
							return parseStatic(ipv4);
						}
						return false;
					});
					if (ipv4.flags == IPv4ConfigFlag::DHCP) {
						error("No fallback address configured");
						return false;
					}
					if (0 >= ipv4.timeout) {
						if (!has_continue_dhcp && !ipv4.continue_dhcp) {
							error("Need timeout if continue-dhcp is disabled");
							return false;
						}
						ipv4.continue_dhcp = true;
					}
					return result;
				}
			case INTEGER:
				pop();
				ipv4.timeout = tv.i;
				ipv4.continue_dhcp = (0 >= ipv4.timeout);
				switch (peek()) {
				case ZEROCONF:
					return parseZeroconf(env, ipv4);
				case STATIC:
					return parseStatic(ipv4);
				}
				return true;
			case ZEROCONF:
				ipv4.continue_dhcp = true;
				return parseZeroconf(env, ipv4);
			case STATIC:
				ipv4.continue_dhcp = true;
				return parseStatic(ipv4);
			default:
				error("expected '{', integer (timeout value), 'zeroconf' or 'static'");
				return false;
			}
			return true;
		}

		bool parseDHCP(local_env_config& env, IPv4Config& ipv4) {
			if (!eat(DHCP)) return false;
			if (env.has_dhcp) {
				error("already have 'dhcp' in this environment");
				return false;
			}
			env.has_dhcp = true;
			ipv4.flags |= IPv4ConfigFlag::DHCP;
			if (!parseIpv4Metric(ipv4)) return false;
			if (FALLBACK == peek() && !parseDHCPFallback(env, ipv4)) return false;
			return true;
		}

		template<typename... Args>
		void addSelectRule(local_env_config& env, QVector<quint32>& block, Args&&... args) {
			block << env.real->select.filters.size();
			env.real->select.filters << SelectRule{ std::forward<Args>(args)... };
		}

		template<typename... Args>
		QVector<quint32>& addSelectRuleBlock(local_env_config& env, QVector<quint32>& block, Args&&... args) {
			quint32 block_ndx = env.real->select.blocks.size();
			addSelectRule(env, block, block_ndx, std::forward<Args>(args)...);
			env.real->select.blocks << QVector<quint32>{};
			return env.real->select.blocks.last();
		}

		bool parseSelectBlockOption(local_env_config& env, QVector<quint32>& block) {
			if (SELECT == peek()) pop();
			switch (peek()) {
			case USER:
				pop();
				addSelectRule(env, block);
				return true;
			case ARP:
				{
					pop();
					if (!eat(IPv4_VAL)) return false;
					auto ip = tv.addr;
					switch (peek()) {
					case MACADDR:
						pop();
						addSelectRule(env, block, ip, tv.macAddr);
						return true;
					case '}':
					case ';':
						addSelectRule(env, block, ip);
						return true;
					default:
						error_expected("mac address, '}' or ';'");
						return false;
					}
				}
			case ESSID:
				pop();
				if (!eat(STRING)) return false;
				addSelectRule(env, block, tv.str);
				return true;
			case AND:
				pop();
				return parseSelectBlock(env, addSelectRuleBlock(env, block, SelectType::AND_BLOCK));
			case OR:
				pop();
				return parseSelectBlock(env, addSelectRuleBlock(env, block, SelectType::OR_BLOCK));
			default:
				error_expected("'user', 'arp', 'essid', 'and', 'or', ';' or '}'");
				return false;
			}
		}

		bool parseSelectBlock(local_env_config& env, QVector<quint32>& block) {
			return parseBlock([&]() { return parseSelectBlockOption(env, block); });
		}

		bool parseSelect(local_env_config& env) {
			if (!eat(SELECT)) return false;
			if (!env.real->select.filters.isEmpty()) {
				error("already have 'select' in this environment");
				pop();
				return false;
			}

			QVector<quint32> block;
			if (!parseSelectBlock(env, block)) return false;
			if (0 == block.size()) {
				error("'select' rules empty");
				return false;
			}
			if (1 != block.size()) {
				error("need 'and' or 'or' for outer list of select rules");
				return false;
			}
			if (0 != block[0]) throw Exception("'select' rules internal error");

			return true;
		}

		bool parseEnvironmentOption(DeviceConfig& dev, local_env_config& env) {
			switch (peek()) {
			case DHCP:
				return tryIPv4(env, [&](IPv4Config& ipv4) { return parseDHCP(env, ipv4); });
			case ZEROCONF:
				return tryIPv4(env, [&](IPv4Config& ipv4) { return parseZeroconf(env, ipv4); });
			case STATIC:
				return tryIPv4(env, [&](IPv4Config& ipv4) { return parseStatic(ipv4); });
			case SELECT:
				return parseSelect(env);
			case NODHCP:
				env.no_def_dhcp = true;
				return true;
			case METRIC:
				pop();
				if (!eat(INTEGER)) return false;
				if (-1 != env.real->metric) {
					error("metric already set for environment");
					return false;
				}
				env.real->metric = tv.i;
				return true;
			default:
				error_expected("environment option");
				return false;
			}
		}

		bool parseEnvironmentBlock(DeviceConfig& dev, local_env_config& env) {
			return parseBlock([&]() { return parseEnvironmentOption(dev, env); });
		}

		bool parseWpaSupplicant(DeviceConfig& dev) {
			if (!eat(WPASUPPLICANT)) return false;
			if (!dev.wpaConfigFile.isEmpty()) {
				error("already have 'wpa-supplicant'");
				return false;
			}
			for (;;) {
				auto t = peek();
				QString* dest = nullptr;
				switch (t) {
				case DRIVER:
					dest = &dev.wpaDriver;
					break;
				case CONFIG:
					dest = &dev.wpaConfigFile;
					break;
				}
				if (!dest) break;

				pop();
				if (!eat(STRING)) return false;
				if (!dest->isEmpty()) {
					error("already have " + toString(t));
					return false;
				}
				*dest = tv.str;
			}
			if (dev.wpaConfigFile.isEmpty()) {
				error("'config' required for 'wpa-supplicant'");
				return false;
			}
			if (dev.wpaDriver.isEmpty()) dev.wpaDriver = "wext";
			return true;
		}

		bool parseDeviceOption(DeviceConfig& dev, local_env_config& defEnv) {
			switch (peek()) {
			case ENVIRONMENT:
				pop();
				switch (peek()) {
				case STRING:
					{
						pop();
						local_env_config env(tv.str);
						dev.environments.push_back(env.real);
						return parseEnvironmentBlock(dev, env);
					}
				case DHCP:
				case ZEROCONF:
				case STATIC:
				case SELECT:
				case NODHCP:
				case METRIC:
				case '{':
					return parseEnvironmentBlock(dev, defEnv);
				default:
					error_expected("environment name, environment option or '{'");
					return false;
				}
			case WPASUPPLICANT:
				return parseWpaSupplicant(dev);
			case NOAUTOSTART:
				pop();
				dev.noAutoStart = true;
				return true;
			case METRIC:
				pop();
				if (!eat(INTEGER)) return false;
				if (-1 != dev.metric) {
					error("metric already set for device");
					return false;
				}
				dev.metric = tv.i;
				return true;
			case DHCP:
			case ZEROCONF:
			case STATIC:
			case SELECT:
			case NODHCP:
				return parseEnvironmentOption(dev, defEnv);
			default:
				error_expected("device or environment option");
				return false;
			}
		}

		bool parseDeviceConfigBlock(DeviceConfig& dev) {
			local_env_config defEnv;
			dev.environments.push_back(defEnv.real);

			return parseBlock([&]() { return parseDeviceOption(dev, defEnv); });
		}

		bool parseDevice() {
			if (!eat(DEVICE)) return false;
			DeviceNamePatternType pt = DeviceNamePatternType::Plain;
			if (REGEXP == peek()) {
				pt = DeviceNamePatternType::RegExp;
				pop();
			}
			if (!eat(STRING)) return false;
			auto name = tv.str;
			if (DeviceNamePatternType::RegExp != pt
				&& (name.contains("?") || name.contains("[") || name.contains("]") || name.contains("*"))) {
				pt = DeviceNamePatternType::Wildcard;
			}
			DeviceNamePattern dnp { name, pt };
			auto duplicate = (m_config.namedDeviceConfigs.find(dnp) != m_config.namedDeviceConfigs.end());
			if (duplicate) {
				error("duplicate device name");
			}
			auto dev = std::make_shared<DeviceConfig>();
			if (!parseDeviceConfigBlock(*dev)) return false;
			if (!duplicate) {
				m_config.namedDeviceConfigs.emplace(std::move(dnp), std::move(dev));
			}
			return true;
		}

		bool parseDevices() {
			for (;;) {
				switch (peek()) {
				case ';': /* ignore */
					pop();
					break;
				case ENDOFFILE:
					return !failed;
				case DEVICE:
					searchOptionEnd(parseDevice());
					break;
				default:
					error_expected("'device' or end of file");
					return false;
				}
			}
		}
	};
}

namespace nuts {
	Config parseConfig(QString const& filename) {
		Config config;
		state st(config, filename);
		if (!st.parseDevices()) throw Exception("Invalid config file");
		return config;
	}
}
