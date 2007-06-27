
#include "device.h"
#include "log.h"
#include <QMutableListIterator>

extern "C" {
#include <asm/types.h>
// socket, AF_INET, SOCK_RAW
#include <sys/socket.h>
// IPPROTO_RAW
#include <arpa/inet.h>
// fcntl, F_SETFD, FD_CLOEXEC
#include <fcntl.h>
#include <sys/ioctl.h>

#include <netlink/netlink.h>
#include <netlink/msg.h>
#include <netlink/route/addr.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/ethtool.h>
#include <linux/sysctl.h>
};


namespace nuts {
	DeviceManager::DeviceManager(const QString &configFile)
	: config(new Config(configFile)) {
		connect(&hwman, SIGNAL(gotCarrier(const QString&, int )), SLOT(gotCarrier(const QString &, int )));
		connect(&hwman, SIGNAL(lostCarrier(const QString&)), SLOT(lostCarrier(const QString &)));
		QHashIterator<QString, DeviceConfig*> i(config->devices);
		while (i.hasNext()) {
			i.next();
			Device *d = new Device(this, i.key(), i.value());
			devices.insert(i.key(), d);
			d->setEnabled(true, true);
		}
	}
	
	DeviceManager::~DeviceManager() {
		delete config;
		foreach(Device* d, devices)
			delete d;
		devices.clear();
	}
	
	void DeviceManager::gotCarrier(const QString &ifName, int ifIndex) {
		devices[ifName]->gotCarrier(ifIndex);
	}
	void DeviceManager::lostCarrier(const QString &ifName) {
		devices[ifName]->lostCarrier();
	}
	
	Device::Device(DeviceManager* dm, const QString &name, DeviceConfig *config)
	: dm(dm), name(name), interfaceIndex(-1), config(config), activeEnv(-1), nextEnv(-1), enabled(false) {
		foreach(EnvironmentConfig *ec, config->environments)
			envs.push_back(new Environment(this, ec));
	}
	
	Device::~Device() {
		if (enabled) setEnabled(false);
		QMutableListIterator<Environment*> i(envs);
		while (i.hasNext()) {
			delete i.next();
			i.remove();
		}
	}
	
	void Device::envUp(Environment* env) {
		if (envs[activeEnv] != env) return;
		emit deviceUp();
		log << "Device(" << name << ") is up!" << endl;
	}
	void Device::envDown(Environment* env) {
		if (envs[activeEnv] != env) return;
		emit deviceDown();
		log << "Device(" << name << ") is down!" << endl;
		activeEnv = nextEnv;
		nextEnv = -1;
		if (activeEnv != -1)
			envs[activeEnv]->start();
	}
	void Device::gotCarrier(int ifIndex) {
		interfaceIndex = ifIndex;
		log << "Device(" << name << ") gotCarrier" << endl;
		activeEnv = 0;
		envs[activeEnv]->start();
	}
	void Device::lostCarrier() {
		log << "Device(" << name << ") lostCarrier" << endl;
		nextEnv = -1;
		if (activeEnv != -1) {
			envs[activeEnv]->stop();
			activeEnv = -1;
		}
		interfaceIndex = -1;
	}

	QString Device::getName() {
		return name;
	}
	
	int Device::getCurrent() {
		return activeEnv;
	}
	void Device::setCurrent(int i) {
		if (i < 0 || i >= envs.size()) {
			err << QString("Device::setCurrent: environment %1 does not exist.")
					.arg(i)
				<< endl;
			return;
		}
		if (activeEnv == -1) {
			activeEnv = i;
			envs[i]->start();
		} else {
			if (nextEnv == -1)
				envs[activeEnv]->stop();
			nextEnv = i;
		}
	}
	
	bool Device::getEnabled() {
		return enabled;
	}
	void Device::setEnabled(bool b, bool force) {
		if (enabled != b) {
			if ((enabled = b) == true) {
				dm->hwman.controlOn(name, force);
			} else {
				dm->hwman.controlOff(name);
				nextEnv = -1;
				if (activeEnv != -1) {
					envs[activeEnv]->stop();
					activeEnv = -1;
				}
				interfaceIndex = -1;
			}
		}
	}
	
	Environment::Environment(Device *device, EnvironmentConfig *config)
	: device(device), config(config) {
		if (config->dhcp)
			ifs.push_back(new Interface_IPv4(this, ifs.size(), config->dhcp));
		if (config->zeroconf)
			ifs.push_back(new Interface_IPv4(this, ifs.size(), config->zeroconf));
		foreach (IPv4Config *ic, config->ipv4Interfaces)
			ifs.push_back(new Interface_IPv4(this, ifs.size(), ic));
		ifUpStatus.fill(false, ifs.size());
	}
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
	
	void Environment::checkStatus() {
		bool ifsUp = (ifUpStatus.count() == ifUpStatus.size());
		if (envIsUp != ifsUp) {
			envIsUp = ifsUp;
			if (envIsUp)
				device->envUp(this);
			else
				device->envDown(this);
		}
	}

	void Environment::start() {
		if (envStart) return;
		envStart = true;
		foreach (Interface* i, ifs)
			i->start();
		checkStatus();
	}
	void Environment::stop() {
		if (!envStart) return;
		envStart = false;
		foreach (Interface* i, ifs)
			i->stop();
		checkStatus();
	}
	
	void Environment::ifUp(Interface* i) {
		ifUpStatus[i->index] = true;
		checkStatus();
	}
	void Environment::ifDown(Interface* i) {
		ifUpStatus[i->index] = false;
		checkStatus();
	}
	
	Interface::Interface(int index) : index(index) { }
	Interface::~Interface() { }
	
	Interface_IPv4::Interface_IPv4(Environment *env, int index, IPv4Config *config)
	: Interface(index), env(env), dm(env->device->dm), config(config) {
	}
	
	Interface_IPv4::~Interface_IPv4() {
	}
	
	void Interface_IPv4::startDHCP() {
	}
	void Interface_IPv4::startZeroconf() {
	}
	void Interface_IPv4::startStatic() {
		ip = config->static_ip;
		netmask = config->static_netmask;
		gateway = config->static_gateway;
		dnsserver = config->static_dnsservers;
		systemUp();
		env->ifUp(this);
	}

	void Interface_IPv4::start() {
		log << "Interface_IPv4::start" << endl;
		startStatic();
	}
	
	void Interface_IPv4::stop() {
		log << "Interface_IPv4::stop" << endl;
		systemDown();
		env->ifDown(this);
	}
	
	struct nl_addr* getNLAddr(const QHostAddress &addr) {
		quint32 i = htonl(addr.toIPv4Address());
		return nl_addr_build(AF_INET, &i, sizeof(i));
//		QByteArray buf = addr.toString().toUtf8();
//		return nl_addr_parse(buf.constData(), AF_INET);
	}
	
	void Interface_IPv4::systemUp() {
		struct nl_addr *local = getNLAddr(ip);
		char buf[32];
		log << nl_addr2str (local, buf, 32) << endl;
		struct rtnl_addr *addr = rtnl_addr_alloc();
		rtnl_addr_set_ifindex(addr, env->device->interfaceIndex);
		rtnl_addr_set_family(addr, AF_INET);
		log << env->device->interfaceIndex << endl;
		log << rtnl_addr_set_local(addr, local) << endl;
		log << "systemUp: addr_add = " << rtnl_addr_add(dm->hwman.getNLHandle(), addr, 0) << endl;
		rtnl_addr_put(addr);
		nl_addr_put(local);
	}
	void Interface_IPv4::systemDown() {
		struct nl_addr *local = getNLAddr(ip);
		char buf[32];
		log << nl_addr2str (local, buf, 32) << endl;
		struct rtnl_addr *addr = rtnl_addr_alloc();
		rtnl_addr_set_ifindex(addr, env->device->interfaceIndex);
		rtnl_addr_set_family(addr, AF_INET);
		rtnl_addr_set_local(addr, local);
		log << "systemUp: addr_delete = " << rtnl_addr_delete(dm->hwman.getNLHandle(), addr, 0) << endl;
		rtnl_addr_put(addr);
		nl_addr_put(local);
	}
	
	MacAddress::MacAddress(const QString &str) {
	}
};
