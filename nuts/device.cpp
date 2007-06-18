
#include "device.h"
#include <QMutableListIterator>

namespace nuts {
	DeviceManager::DeviceManager(const QString &configFile)
	: config(new Config(configFile)) {
	}
	
	DeviceManager::~DeviceManager() {
		delete config;
	}
	
	void DeviceManager::gotCarrier(int) {
	}
	void DeviceManager::lostCarrier(int) {
	}
	
	Device::Device(const QString &name, int interfaceIndex, DeviceConfig *config)
	: name(name), interfaceIndex(interfaceIndex), config(config) {
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
