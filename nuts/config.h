
#ifndef _NUTS_CONFIG_H
#define _NUTS_CONFIG_H

#include <QStack>
#include <libnutcommon/config.h>

namespace nuts {
	class ConfigParser;
}

namespace nuts {
	class ConfigParser {
		private:
			struct local_env_config {
				bool m_hasdhcp, m_haszeroconf, no_def_dhcp;
				
				local_env_config()
				: m_hasdhcp(false), m_haszeroconf(false), no_def_dhcp(false) { }
			};
		
			bool failed;
			QString m_configFile;
		
			libnutcommon::Config *m_config;
			libnutcommon::DeviceConfig *m_curdevconfig;
			libnutcommon::EnvironmentConfig *m_curenvconfig;
			libnutcommon::IPv4Config *m_curipv4config;
			bool m_curisfallback;
			
			QStack<quint32> m_selBlocks;
			
			local_env_config *m_cur_env, *m_def_env; //TODO:Whats that?
			
			void selectAdd(const libnutcommon::SelectRule &rule);
		
			bool finishEnvironment(libnutcommon::EnvironmentConfig *envc, local_env_config *l_envc);

		public:
			ConfigParser(const QString &configFile);
			~ConfigParser();
			libnutcommon::Config *getConfig() { return m_config; }
			
		// internal functions for config parsing
		// didn't get protected/friend working with extern "C"
			void parseError(int lineNum, const QString &msg);
			
			bool newDevice(const QString &name, bool regexp=false);
			bool finishDevice();
			
			bool devEnvironment(const QString &name);
			bool finishEnvironment();
			
			bool devDefaultEnvironment();
			bool devNoAutoStart();
			bool devWPASuppConfig(const QString &driver, const QString &config);
			
			bool envNoDHCP();
			
			bool envSelect();
			bool finishSelect();
			
			bool envDHCP();
			bool finishDHCP();
			
			bool envFallback();
			bool finishFallback();
			bool envFallbackTimeout(int timeout);
			bool envFallbackContinueDhcp(bool con);
			
			bool envZeroconf();
			bool finishZeroconf();
			
			bool envStatic();
			bool finishStatic();
			
			bool staticUser();
			
			bool staticIP(const QHostAddress &addr);
			bool staticNetmask(const QHostAddress &addr);
			bool staticGateway(const QHostAddress &addr);
			bool staticDNS(const QHostAddress &addr);

			bool selectAndBlock();
			bool selectOrBlock();
			bool selectBlockEnd();
			
			bool selectUser();
			bool selectARP(const QHostAddress &addr);
			bool selectARP(const QHostAddress &addr, const libnutcommon::MacAddress &mac);
			bool selectESSID(const QString &essid);
	};
}

#endif
