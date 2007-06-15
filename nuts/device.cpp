
#include "device.h"
#include <QMutableListIterator>

namespace nuts {
	void DeviceManager::gotCarrier(int) {
	}
	void DeviceManager::lostCarrier(int) {
	}
	
	Device::Device(const QString &name, int interfaceIndex)
	: name(name), interfaceIndex(interfaceIndex) { }
	
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
	
	Environment::Environment(Device *device)
	: device(device) { }
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
};
