
#include "device.h"
#include <QMutableListIterator>

namespace nuts {
	Device::Device(const QString &name, int interfaceIndex)
	: p_name(name), interfaceIndex(interfaceIndex) { }
	
	Device::~Device() {
		QMutableListIterator<Environment*> i(envs);
		while (i.hasNext()) {
			delete i.next();
			i.remove();
		}
	}
	
	QString Device::name() {
		return p_name;
	}
	
	Environment::Environment() { }
	Environment::~Environment() {
		QMutableListIterator<Interface*> i(ifs);
		while (i.hasNext()) {
			delete i.next();
			i.remove();
		}
	}
	
	Interface::Interface() { }
	Interface::~Interface() { }
};
