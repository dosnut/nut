
#include "device.h"
#include "log.h"
#include <QMutableListIterator>
#include "dhcppacket.h"

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
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h>
#include <linux/sysctl.h>
};


namespace nuts {
	DeviceManager::DeviceManager(const QString &configFile)
	: config(new Config(configFile)) {
		connect(&hwman, SIGNAL(gotCarrier(const QString&, int )), SLOT(gotCarrier(const QString &, int )));
		connect(&hwman, SIGNAL(lostCarrier(const QString&)), SLOT(lostCarrier(const QString &)));
		 /* Wait for 400 ms before deliver carrier events
		   There are 2 reasons:
		    - a kernel bug: dhcp messages cannot be sent immediately
		      after carrier event, 100 ms wait was not enough. (they simply
		      don't reach the hardware, local packet sniffers get them).
		    - in the case of lostCarrier, we want ignore short breaks.
		*/
		carrier_timer.setInterval(400);
		connect(&carrier_timer, SIGNAL(timeout()), SLOT(ca_timer()));
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
	
	void DeviceManager::ca_timer() {
		log << "ca_timer" << endl;
		if (!ca_evts.empty()) {
			log << "ca_timer takeFirst()" << endl;
			struct ca_evt e = ca_evts.takeFirst();
			if (e.up)
				devices[e.ifName]->gotCarrier(e.ifIndex);
			else
				devices[e.ifName]->lostCarrier();
		}
//		if (!ca_evts.empty())
//			carrier_timer.start();
		if (ca_evts.empty())
			carrier_timer.stop();
	}
	
	void DeviceManager::gotCarrier(const QString &ifName, int ifIndex) {
		QMutableLinkedListIterator<struct ca_evt> i(ca_evts);
		while(i.hasNext()) {
			if (i.next().ifName == ifName) {
				i.remove();
				if (ca_evts.empty())
					carrier_timer.stop();
				return;
			}
		}
		struct ca_evt e;
		e.ifName = ifName;
		e.ifIndex = ifIndex;
		e.up = true;
		ca_evts.push_back(e);
		// do not restart the timer if a item is already in the queue
		if (!carrier_timer.isActive())
			carrier_timer.start();
	}
	void DeviceManager::lostCarrier(const QString &ifName) {
		QMutableLinkedListIterator<struct ca_evt> i(ca_evts);
		while(i.hasNext()) {
			if (i.next().ifName == ifName) {
				i.remove();
				if (ca_evts.empty())
					carrier_timer.stop();
				return;
			}
		}
		struct ca_evt e;
		e.ifName = ifName;
		e.ifIndex = 0;
		e.up = false;
		ca_evts.push_back(e);
		// do not restart the timer if a item is already in the queue
		if (!carrier_timer.isActive())
			carrier_timer.start();
	}
	
	Device::Device(DeviceManager* dm, const QString &name, DeviceConfig *config)
	: dm(dm), name(name), interfaceIndex(-1), config(config), activeEnv(-1), nextEnv(-1), enabled(false), dhcp_client_socket(-1) {
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
		if (dhcp_client_socket >= 0)
			closeDHCPClientSocket();
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
		macAddress = dm->hwman.getMacAddress(interfaceIndex);
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
	bool Device::registerXID(quint32 xid, Interface_IPv4 *iface) {
		if (!xid) return false;
		if (dhcp_xid_iface.contains(xid)) return false;
		dhcp_xid_iface.insert(xid, iface);
		if (dhcp_client_socket < 0)
			setupDHCPClientSocket();
		return true;
	}
	void Device::unregisterXID(quint32 xid) {
		if (!xid) return;
		dhcp_xid_iface.remove(xid);
		if (dhcp_xid_iface.count() == 0 && dhcp_client_socket >= 0)
			closeDHCPClientSocket();
	}
	void Device::sendDHCPClientPacket(DHCPPacket *packet) {
		dhcp_write_buf.push_back(packet->msgdata);
		dhcp_write_nf->setEnabled(true);
	}
	void Device::setupDHCPClientSocket() {
		if (interfaceIndex < 0) {
			log << "Interface index invalid" << endl;
			return;
		}
		if ((dhcp_client_socket = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0) {
			log << "Couldn't open rawsocket for dhcp client" << endl;
			dhcp_client_socket = -1;
			return;
		}
		const char MAC_BCAST_ADDR[] = "\xff\xff\xff\xff\xff\xff";
		struct sockaddr_ll sock;
		memset(&sock, 0, sizeof(sock));
		sock.sll_family = AF_PACKET;
		sock.sll_protocol = htons(ETH_P_IP);
		sock.sll_ifindex = interfaceIndex;
		sock.sll_halen = 6;
		memcpy(sock.sll_addr, MAC_BCAST_ADDR, 6);
		if (bind(dhcp_client_socket, (struct sockaddr *) &sock, sizeof(sock)) < 0) {
			log << "Couldn't bind socket for dhcp client" << endl;
			close(dhcp_client_socket);
			dhcp_client_socket = -1;
		}
		dhcp_read_nf = new QSocketNotifier(dhcp_client_socket, QSocketNotifier::Read);
		dhcp_write_nf = new QSocketNotifier(dhcp_client_socket, QSocketNotifier::Write);
		dhcp_write_nf->setEnabled(!dhcp_write_buf.empty());
		connect(dhcp_read_nf, SIGNAL(activated(int)), SLOT(readDHCPClientSocket()));
		connect(dhcp_write_nf, SIGNAL(activated(int)), SLOT(writeDHCPClientSocket()));
	}
	void Device::closeDHCPClientSocket() {
		delete dhcp_read_nf; dhcp_read_nf = 0;
		delete dhcp_write_nf; dhcp_write_nf = 0;
		close(dhcp_client_socket);
		dhcp_client_socket = -1;
	}
	void Device::readDHCPClientSocket() {
		struct sockaddr_ll sock;
		socklen_t slen = sizeof(sock);
		quint32 xid;
		Interface_IPv4 * iface;
		QByteArray buf;
		int read, msgsize = 256 + 1;
		do {
			msgsize = (msgsize << 1) - 1;
			buf.resize(msgsize);
			read = recvfrom(dhcp_client_socket, buf.data(), msgsize, MSG_PEEK, (struct sockaddr *)&sock, &slen);
			if (read == 0) return;
		} while (read == msgsize);
		buf.resize(read);
		recvfrom(dhcp_client_socket, buf.data(), read, 0, (struct sockaddr *)&sock, &slen);
		DHCPPacket *packet = DHCPPacket::parseRaw(buf);
		if (!packet) return;
		// check mac
		if (packet->getClientMac() != macAddress)
			goto cleanup;
		xid = packet->getXID();
		iface = dhcp_xid_iface.value(xid);
		if (!iface)
			goto cleanup;
		iface->dhcpReceived(packet);
	cleanup:
		delete packet;
	}
	
	void Device::writeDHCPClientSocket() {
		log << "writeDHCPClientSocket" << endl;
		if (!dhcp_write_buf.empty()) {
			QByteArray msgdata = dhcp_write_buf.takeFirst();
			// raw_packet(&packet, INADDR_ANY, CLIENT_PORT, INADDR_BROADCAST,
			// SERVER_PORT, MAC_BCAST_ADDR, client_config.ifindex);
			const char MAC_BCAST_ADDR[] = "\xff\xff\xff\xff\xff\xff";
			struct sockaddr_ll sock;
			memset(&sock, 0, sizeof(sock));
			sock.sll_family = AF_PACKET;
			sock.sll_protocol = htons(ETH_P_IP);
			sock.sll_ifindex = interfaceIndex;
			sock.sll_halen = 6;
			memcpy(sock.sll_addr, MAC_BCAST_ADDR, 6);
			sendto(dhcp_client_socket, msgdata.data(), msgdata.size(), 0, (struct sockaddr *) &sock, sizeof(sock));
		}
		dhcp_write_nf->setEnabled(!dhcp_write_buf.empty());
	}
	
	MacAddress Device::getMacAddress() {
		return macAddress;
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
	: device(device), config(config), envIsUp(false), envStart(false) {
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
		bool ifsUp = (ifUpStatus.count(true) == ifUpStatus.size());
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
		if (dhcp_xid) {
			env->device->unregisterXID(dhcp_xid);
			dhcp_xid = 0;
		}
	}
	
	void Interface_IPv4::startDHCP() {
		log << "waiting (work around kernel bug)" << endl;
//		sleep(1);
		log << "start dhcp" << endl;
		DHCPClientPacket cp(this);
		cp.doDHCPDiscover();
		cp.check();
		log << "send dhcp" << endl;
		cp.send();
		log << "sent dhcp" << endl;
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
		if (config->flags & IPv4Config::DO_DHCP)
			startDHCP();
		else
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
	
	bool Interface_IPv4::registerXID(quint32 xid) {
		if (dhcp_xid) {
			env->device->unregisterXID(dhcp_xid);
			dhcp_xid = 0;
		}
		if (env->device->registerXID(xid, this)) {
			dhcp_xid = xid;
			return true;
		}
		return false;
	}
	
	void Interface_IPv4::dhcpReceived(DHCPPacket *packet) {
		switch (packet->getMessageType()) {
			case DHCP_OFFER:
				log << "dhcpReceived: DHCP OFFER" << endl;
				break;
			case DHCP_ACK:
				log << "dhcpReceived: DHCP ACK" << endl;
				break;
			default:
				log << "dhcpReceived: unexpected dhcp message type" << endl;
		}
	}

	
	inline int read_hexdigit(char c) {
		if ('0' <= c && c <= '9')
			return c - '0';
		if ('a' <= c && c <= 'f')
			return c - 'a' + 10;
		if ('A' <= c && c <= 'F')
			return c - 'A' + 10;
		return -1;
	}
	
	inline char* hex2quint8(char* msg, quint8 &val) {
		int i;
		val = 0;
		if (!msg || !*msg) return msg;
		if ((i = read_hexdigit(*msg)) == -1)
			return msg;
		msg++;
		if (!*msg) return msg;
		val = ((quint8) i) << 4;
		if ((i = read_hexdigit(*msg)) == -1)
			return msg;
		msg++;
		val |= i;
		return msg;
	}
	
	MacAddress::MacAddress() {
		for (int i = 0; i < 6; i++)
			data[i] = 0;
	}
	MacAddress::MacAddress(const QString &str) {
		QByteArray buf = str.toUtf8();
		char *s = buf.data();
		quint8 val;
		char *s2;
		for (int i = 0; i < 6; i++) {
			if (!*s) return;
			if ((s2 = hex2quint8(s, val)) == s) return;
			s = s2;
			if (*s && *s != ':') return;
			data[i] = val;
		}
	}
	
	MacAddress::MacAddress(const quint8 *d) {
		if (d == 0) {
			MacAddress();
		} else {
			for (int i = 0; i < 6; i++)
				data[i] = d[i];
		}
	}
}
