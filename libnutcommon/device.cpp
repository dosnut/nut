#include "common.h"
#include "enum.h"

namespace libnutcommon {
	QString toString(DeviceState state) {
		return enumArrayEntry(state, { "deactivated", "activated", "carrier", "unconfigured", "up" });
	}

	QString toString(DeviceType type) {
		return enumArrayEntry(type, { "eth", "air", "ppp" });
	}

	QString toString(InterfaceState state) {
		return enumArrayEntry(state, { "off", "static", "dhcp", "zeroconf", "waitforconfig" });
	}

	bool operator==(DeviceProperties const& a, DeviceProperties const& b) {
		return a.name              == b.name
			&& a.type              == b.type
			&& a.activeEnvironment == b.activeEnvironment
			&& a.state             == b.state
			&& a.essid             == b.essid
			&& a.macAddress        == b.macAddress;
	}
	bool operator!=(DeviceProperties const& a, DeviceProperties const& b) {
		return !(a == b);
	}

	bool operator==(EnvironmentProperties const& a, EnvironmentProperties const& b) {
		return a.name          == b.name
			&& a.id            == b.id
			&& a.active        == b.active
			&& a.selectResult  == b.selectResult
			&& a.selectResults == b.selectResults;
	}
	bool operator!=(EnvironmentProperties const& a, EnvironmentProperties const& b) {
		return !(a == b);
	}

	bool operator==(InterfaceProperties const& a, InterfaceProperties const& b) {
		return a.state         == b.state
			&& a.ip            == b.ip
			&& a.netmask       == b.netmask
			&& a.gateway       == b.gateway
			&& a.dnsServers    == b.dnsServers
			&& a.gatewayMetric == b.gatewayMetric
			&& a.needUserSetup == b.needUserSetup;
	}
	bool operator!=(InterfaceProperties const& a, InterfaceProperties const& b) {
		return !(a == b);
	}

	bool operator==(IPv4UserConfig const& a, IPv4UserConfig const& b) {
		return a.ip         == b.ip
			&& a.netmask    == b.netmask
			&& a.gateway    == b.gateway
			&& a.dnsServers == b.dnsServers;
	}
	bool operator!=(IPv4UserConfig const& a, IPv4UserConfig const& b) {
		return !(a == b);
	}

	// called by common.cpp: init()
	void device_init() {
		qRegisterMetaType<OptionalQDBusObjectPath>("libnutcommon::OptionalQDBusObjectPath");
		qRegisterMetaType<DeviceState>("libnutcommon::DeviceState");
		qRegisterMetaType<DeviceType>("libnutcommon::DeviceType");
		qRegisterMetaType<InterfaceState>("libnutcommon::InterfaceState");
		qRegisterMetaType<DeviceProperties>("libnutcommon::DeviceProperties");
		qRegisterMetaType<EnvironmentProperties>("libnutcommon::EnvironmentProperties");
		qRegisterMetaType<InterfaceProperties>("libnutcommon::InterfaceProperties");
		qRegisterMetaType<IPv4UserConfig>("libnutcommon::IPv4UserConfig");

		qDBusRegisterMetaType<OptionalQDBusObjectPath>();
		qDBusRegisterMetaType<DeviceState>();
		qDBusRegisterMetaType<DeviceType>();
		qDBusRegisterMetaType<InterfaceState>();
		qDBusRegisterMetaType<DeviceProperties>();
		qDBusRegisterMetaType<EnvironmentProperties>();
		qDBusRegisterMetaType<InterfaceProperties>();
		qDBusRegisterMetaType<IPv4UserConfig>();
	}
}
