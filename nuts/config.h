
#ifndef _NUTS_CONFIG_H
#define _NUTS_CONFIG_H

#include <QStack>
#include <common/config.h>

namespace nuts {
	class ConfigParser;
}

namespace nuts {
	class ConfigParser {
		private:
			struct local_env_config {
				bool m_hasdhcp, no_def_dhcp;
				
				local_env_config()
				: m_hasdhcp(false), no_def_dhcp(false) { }
			};
		
			bool failed;
			QString m_configFile;
		
			nut::Config *m_config;
			nut::DeviceConfig *m_curdevconfig;
			nut::EnvironmentConfig *m_curenvconfig;
			nut::IPv4Config *m_curipv4config;
			QStack<size_t> m_selBlocks;
			
			local_env_config *m_cur_env, *m_def_env;
			
			void selectAdd(const nut::SelectRule &rule);
		
			bool finishEnvironment(nut::EnvironmentConfig *envc, local_env_config *l_envc);

		public:
			ConfigParser(const QString &configFile);
			~ConfigParser();
			nut::Config *getConfig() { return m_config; }
			
		// internal functions for config parsing
		// didn't get protected/friend working with extern "C"
			void parseError(int lineNum, const QString &msg);
			
			bool newDevice(const QString &name);
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
			
			bool envStatic();
			bool finishStatic();
			
			bool staticUser();
			
			bool staticIP(const QHostAddress &addr);
			bool staticNetmak(const QHostAddress &addr);
			bool staticGateway(const QHostAddress &addr);
			bool staticDNS(const QHostAddress &addr);

			bool selectAndBlock();
			bool selectOrBlock();
			
			bool selectUser();
			bool selectARP(const QHostAddress &addr);
			bool selectARP(const QHostAddress &addr, const nut::MacAddress &mac);
			bool selectESSID(const QString &essid);
	};
}

#endif
