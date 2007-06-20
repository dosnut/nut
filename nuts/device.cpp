
#include "device.h"
#include "log.h"
#include <QMutableListIterator>

namespace nuts {
	DeviceManager::DeviceManager(const QString &configFile)
	: config(new Config(configFile)) {
		QHashIterator<QString, DeviceConfig*> i(config->devices);
		while (i.hasNext()) {
			i.next();
			Device *d = new Device(i.key(), i.value());
			devices.insert(i.key(), d);
		}
	}
	
	DeviceManager::~DeviceManager() {
		delete config;
		foreach (Device* d, devices)
			delete d;
		devices.clear();
	}
	
	void DeviceManager::gotCarrier(const QString &ifName, int ifIndex) {
		devices[ifName]->gotCarrier(ifIndex);
	}
	void DeviceManager::lostCarrier(const QString &ifName) {
		devices[ifName]->lostCarrier();
	}
	
	Device::Device(const QString &name, DeviceConfig *config)
	: name(name), interfaceIndex(-1), config(config) {
	}
	
	Device::~Device() {
		QMutableListIterator<Environment*> i(envs);
		while (i.hasNext()) {
			delete i.next();
			i.remove();
		}
	}
	
	void Device::envUp(Environment*) {
	}
	void Device::envDown(Environment*) {
	}
	void Device::gotCarrier(int ifIndex) {
		interfaceIndex = ifIndex;
		log << "Device(" << name << ") gotCarrier" << endl;
	}
	void Device::lostCarrier() {
		log << "Device(" << name << ") lostCarrier" << endl;
		interfaceIndex = -1;
	}

	QString Device::getName() {
		return name;
	}
	
	int Device::getCurrent() {
		return activeEnv;
	}
	void Device::setCurrent(int i) {
		activeEnv = i;
	}
	
	bool Device::getEnabled() {
		return enabled;
	}
	void Device::setEnabled(bool b) {
		enabled = b;
	}
	
	Environment::Environment(Device *device, EnvironmentConfig *config)
	: device(device), config(config) { }
	Environment::~Environment() {
		QMutableListIterator<Interface*> i(ifs);
		while (i.hasNext()) {
			delete i.next();
			i.remove();
		}
	}
	
	Device* Environment::getDevice() {
		return device;
	}
	
	const QList<Interface*>& Environment::getInterfaces() {
		return ifs;
	}
	
	Interface::Interface() { }
	Interface::~Interface() { }
	
	Interface_IPv4::Interface_IPv4(IPv4Config *config)
	: config(config) {
	}
	
	Interface_IPv4::~Interface_IPv4() {
	}

	void Interface_IPv4::start() {
	}
	
	MacAddress::MacAddress(const QString &str) {
	}
};
