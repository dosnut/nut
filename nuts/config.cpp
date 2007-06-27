
#include "config.h"
#include "configparser_yacc.h"

extern "C" {
#include <stdio.h>
}

void configparserparse(nuts::Config* config);
extern FILE *configparserin;

/* sub configuration structures will
 * be deleted by ~Config() !
 */

namespace nuts {
	Config::Config(const QString &configFile)
	: configFile(configFile) {
		configparserin = fopen(configFile.toUtf8().constData(), "r");
		configparserparse(this);
		fclose(configparserin);
	}
	Config::~Config() {
		foreach(DeviceConfig* dc, devices)
			delete dc;
		devices.clear();
	}
	DeviceConfig* Config::getDevice(const QString &deviceName) {
		if (!devices.contains(deviceName)) {
			return 0;
		} else {
			return devices[deviceName];
		}
	}

	DeviceConfig* Config::createDevice(const QString &deviceName) {
		if (!devices.contains(deviceName)) {
			DeviceConfig* t = new DeviceConfig();
			devices.insert(deviceName, t);
			return t;
		} else {
			return devices[deviceName];
		}
	}

	DeviceConfig::DeviceConfig()
	: defEnv(new EnvironmentConfig()), canUserEnable(false) {
		environments.push_back(defEnv);
	}
	DeviceConfig::~DeviceConfig() {
		foreach(EnvironmentConfig* ec, environments)
			delete ec;
		environments.clear();
	}
	
	EnvironmentConfig* DeviceConfig::getDefaultEnv() {
		return defEnv;
	}
	EnvironmentConfig* DeviceConfig::createEnvironment() {
		EnvironmentConfig* t = new EnvironmentConfig();
		environments.push_back(t);
		return t;
	}
	
	EnvironmentConfig::EnvironmentConfig()
	: canUserSelect(true), noDefaultDHCP(false), noDefaultZeroconf(false), dhcp(0), zeroconf(0) {
	}
	EnvironmentConfig::~EnvironmentConfig() {
		delete dhcp;
		delete zeroconf;
	}
	IPv4Config *EnvironmentConfig::doDHCP() {
		if (!dhcp)
			dhcp = new IPv4Config(IPv4Config::DO_DHCP | IPv4Config::DO_ZEROCONF, 0);
		return dhcp;
	}
	IPv4Config *EnvironmentConfig::createStatic() {
		IPv4Config* ic = new IPv4Config(IPv4Config::DO_STATIC, 0);
		ipv4Interfaces.push_back(ic);
		return ic;
	}
	
	IPv4Config::IPv4Config(int flags, int overwriteFlags)
	: flags(flags), overwriteFlags(overwriteFlags), canUserEnable(false) {
	}
};
