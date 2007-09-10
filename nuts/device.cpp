
#include "device.h"
#include "log.h"
#include <QMutableListIterator>
#include <QProcess>
#include <QStringList>
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
#include <linux/route.h>
void perror(const char *s); 
};

static inline void setSockaddrIPv4(struct sockaddr &s, quint32 host = 0, quint16 port = 0) {
	struct sockaddr_in *sin = (struct sockaddr_in *) &s;
	sin->sin_family = AF_INET;
	sin->sin_port = htons(port);
	sin->sin_addr.s_addr = htonl(host);
}

namespace nuts {
	DeviceManager::DeviceManager(const QString &configFile)
	: configParser(configFile), config(configParser.getConfig()) {
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
		QHashIterator<QString, nut::DeviceConfig*> i(config->getDevices());
		while (i.hasNext()) {
			i.next();
			Device *d = new Device(this, i.key(), i.value());
			devices.insert(i.key(), d);
			d->enable(true);
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
	
	Device::Device(DeviceManager* dm, const QString &name, nut::DeviceConfig *config)
	: dm(dm), name(name), interfaceIndex(-1), config(config), activeEnv(-1), nextEnv(-1), m_state(libnut::DS_DEACTIVATED), dhcp_client_socket(-1) {
		int i = 0;
		foreach(nut::EnvironmentConfig *ec, config->getEnvironments())
			envs.push_back(new Environment(this, ec, i++));
	}
	
	Device::~Device() {
		disable();
		QMutableListIterator<Environment*> i(envs);
		while (i.hasNext()) {
			delete i.next();
			i.remove();
		}
		if (dhcp_client_socket >= 0)
			closeDHCPClientSocket();
	}
	
	void Device::setState(libnut::DeviceState state) {
		libnut::DeviceState ostate = m_state;
		m_state = state;
		emit stateChanged(m_state, ostate);
	}
	
	void Device::envUp(Environment* env) {
		if (envs[activeEnv] != env) return;
		setState(libnut::DS_UP);
		log << "Device(" << name << ") is up!" << endl;
	}
	void Device::envDown(Environment* env) {
		if (envs[activeEnv] != env) return;
		setState(libnut::DS_CARRIER);
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
		setState(libnut::DS_CARRIER);
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
		setState(libnut::DS_ACTIVATED);
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
		int nread, msgsize = 256 + 1;
		do {
			msgsize = (msgsize << 1) - 1;
			buf.resize(msgsize);
			nread = recvfrom(dhcp_client_socket, buf.data(), msgsize, MSG_PEEK, (struct sockaddr *)&sock, &slen);
			if (nread < 0) {
//				perror("Device::readDHCPClientSocket: recvfrom");
				closeDHCPClientSocket();
			}
			if (nread == 0) return;
		} while (nread == msgsize);
		buf.resize(nread);
		recvfrom(dhcp_client_socket, buf.data(), nread, 0, (struct sockaddr *)&sock, &slen);
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
	
	nut::MacAddress Device::getMacAddress() {
		return macAddress;
	}

	QString Device::getName() {
		return name;
	}
	
	int Device::getEnvironment() {
		return activeEnv;
	}
	void Device::setEnvironment(int env) {
		if (env < 0 || env >= envs.size()) {
			err << QString("Device::setEnvironment: environment %1 does not exist.")
					.arg(env)
				<< endl;
			return;
		}
		if (activeEnv == -1) {
			activeEnv = env;
			envs[env]->start();
		} else {
			if (nextEnv == -1)
				envs[activeEnv]->stop();
			nextEnv = env;
		}
	}
	
	void Device::enable(bool force) {
		if (m_state == libnut::DS_DEACTIVATED) {
			setState(libnut::DS_ACTIVATED);
			dm->hwman.controlOn(name, force);
		}
	}
	void Device::disable() {
		if (m_state != libnut::DS_DEACTIVATED) {
			nextEnv = -1;
			if (activeEnv != -1) {
				envs[activeEnv]->stop();
				activeEnv = -1;
			}
			interfaceIndex = -1;
			dm->hwman.controlOff(name);
			setState(libnut::DS_DEACTIVATED);
		}
	}
	
	Environment::Environment(Device *device, nut::EnvironmentConfig *config, int id)
	: device(device), config(config), envIsUp(false), envStart(false), m_id(id) {
		foreach (nut::IPv4Config *ic, config->getIPv4Interfaces())
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
		ifUpStatus[i->m_index] = true;
		checkStatus();
	}
	void Environment::ifDown(Interface* i) {
		ifUpStatus[i->m_index] = false;
		checkStatus();
	}
	
	Interface::Interface(int index) : m_index(index) { }
	Interface::~Interface() { }
	
	Interface_IPv4::Interface_IPv4(Environment *env, int index, nut::IPv4Config *config)
	: Interface(index), env(env), dm(env->device->dm), dhcpstate(DHCPS_OFF), config(config) {
	}
	
	Interface_IPv4::~Interface_IPv4() {
		if (dhcp_xid) {
			env->device->unregisterXID(dhcp_xid);
			dhcp_xid = 0;
		}
	}
	
	void Interface_IPv4::dhcp_send_discover() {
		// send discover
		DHCPClientPacket cp(this);
		cp.doDHCPDiscover();
		cp.check();
		cp.send();
	}
	
	void Interface_IPv4::dhcp_send_request(DHCPPacket *offer) {
		DHCPClientPacket cp(this);
		cp.doDHCPRequest(*offer);
		cp.check();
		cp.send();
	}
	
	void Interface_IPv4::dhcp_setup_interface(DHCPPacket *ack) {
		ip = ack->getYourIP();
		netmask = ack->getOptionAddress(DHCP_SUBNET);
		gateway = ack->getOptionAddress(DHCP_ROUTER);
		dnsserver = ack->getOptionAddresses(DHCP_DNS_SERVER);
		localdomain = ack->getOptionString(DHCP_DOMAIN_NAME);
		dhcp_server_identifier = ack->getOption(DHCP_SERVER_ID);
		dhcp_lease_time = ntohl(ack->getOptionData<quint32>(DHCP_LEASE_TIME, -1));
		systemUp();
		env->ifUp(this);
	}
	
	void Interface_IPv4::dhcpAction(DHCPPacket *source) {
		for (;;) {
			switch (dhcpstate) {
				case DHCPS_OFF:
				case DHCPS_FAILED:
					return;
				case DHCPS_INIT:
					dhcp_send_discover();
					dhcpstate = DHCPS_SELECTING;
					break;
				case DHCPS_SELECTING:
					if (source) {
						if (source->getMessageType() == DHCP_OFFER)  {
							dhcp_send_request(source);
							dhcpstate = DHCPS_REQUESTING;
						}
					} else {
						// TODO: setup timeout
						return;
					}
				case DHCPS_REQUESTING:
					if (source) {
						switch (source->getMessageType()) {
							case DHCP_ACK:
								dhcp_setup_interface(source);
								dhcpstate = DHCPS_BOUND;
								break;
							case DHCP_NAK:
								dhcpstate = DHCPS_INIT;
								break;
							default:
								break;
						}
					} else {
						// TODO: setup timeout
					}
					return;
				case DHCPS_BOUND:
					// TODO: setup timeout
					return;
				default:
					log << "Unhandled dhcp state" << endl;
					return;
			}
			source = 0;
		}
	}
	
	void Interface_IPv4::startDHCP() {
		dhcpstate = DHCPS_INIT;
		dhcpAction();
	}
	void Interface_IPv4::startZeroconf() {
	}
	void Interface_IPv4::startStatic() {
		ip = config->getStaticIP();
		netmask = config->getStaticNetmask();
		gateway = config->getStaticGateway();
		dnsserver = config->getStaticDNS();
		systemUp();
		env->ifUp(this);
	}

	void Interface_IPv4::start() {
		log << "Interface_IPv4::start" << endl;
		if (config->getFlags() & nut::IPv4Config::DO_DHCP)
			startDHCP();
		else
			startStatic();
	}
	
	void Interface_IPv4::stop() {
		log << "Interface_IPv4::stop" << endl;
		systemDown();
		env->ifDown(this);
	}
	
	inline int getPrefixLen(const QHostAddress &netmask) {
		quint32 val = netmask.toIPv4Address();
		int i = 32;
		while (val && !(val & 0x1)) {
			i--;
			val >>= 1;
		}
		return i;
	}
	
	inline struct nl_addr* getNLAddr(const QHostAddress &addr) {
		quint32 i = htonl(addr.toIPv4Address());
		return nl_addr_build(AF_INET, &i, sizeof(i));
	}
	
	inline struct nl_addr* getNLAddr(const QHostAddress &addr, const QHostAddress &netmask) {
		quint32 i = htonl(addr.toIPv4Address());
		struct nl_addr* a = nl_addr_build(AF_INET, &i, sizeof(i));
		if (!netmask.isNull())
			nl_addr_set_prefixlen(a, getPrefixLen(netmask));
		return a;
	}
	
	void Interface_IPv4::systemUp() {
		struct nl_addr *local = getNLAddr(ip, netmask);
		char buf[32];
		log << nl_addr2str (local, buf, 32) << endl;
		struct rtnl_addr *addr = rtnl_addr_alloc();
		rtnl_addr_set_ifindex(addr, env->device->interfaceIndex);
		rtnl_addr_set_family(addr, AF_INET);
		rtnl_addr_set_local(addr, local);
		log << "systemUp: addr_add = " << rtnl_addr_add(dm->hwman.getNLHandle(), addr, 0) << endl;
		rtnl_addr_put(addr);
		nl_addr_put(local);
		// Gateway
		if (!gateway.isNull()) {
			log << "Try setting gateway" << endl;
			struct rtentry rt;
			memset(&rt, 0, sizeof(rt));
			rt.rt_flags = RTF_UP | RTF_GATEWAY;
			setSockaddrIPv4(rt.rt_dst);
			setSockaddrIPv4(rt.rt_gateway, gateway.toIPv4Address());
			setSockaddrIPv4(rt.rt_genmask);
			QByteArray buf = env->device->name.toUtf8();
			rt.rt_dev = buf.data();
			int skfd = socket(AF_INET, SOCK_DGRAM, 0);
			write(3, &rt, sizeof(rt));
			ioctl(skfd, SIOCADDRT, &rt);
			close(skfd);
		}
		// Resolvconf
		if (!dnsserver.empty()) {
			QProcess *proc = new QProcess();
			QStringList arguments;
			arguments << "-a" << QString("%1_%2").arg(env->device->name).arg(m_index);
			proc->start("/sbin/resolvconf", arguments);
			QTextStream ts(proc);
			if (!localdomain.isEmpty())
				ts << "domain " << localdomain << endl;
			foreach(QHostAddress ha, dnsserver) {
				ts << "nameserver " << ha.toString() << endl;
			}
			proc->closeWriteChannel();
			proc->waitForFinished(-1);
			delete proc; // waits for process
		}
	}
	void Interface_IPv4::systemDown() {
		// Resolvconf
		QProcess *proc = new QProcess();
		QStringList arguments;
		arguments << "-d" << QString("%1_%2").arg(env->device->name).arg(m_index);
		proc->start("/sbin/resolvconf", arguments);
		proc->closeWriteChannel();
		proc->waitForFinished(-1);
		delete proc; // waits for process
		// Gateway
		if (!gateway.isNull()) {
			struct rtentry rt;
			memset(&rt, 0, sizeof(rt));
			rt.rt_flags = RTF_UP | RTF_GATEWAY;
			setSockaddrIPv4(rt.rt_dst);
			setSockaddrIPv4(rt.rt_gateway, gateway.toIPv4Address());
			setSockaddrIPv4(rt.rt_genmask);
			QByteArray buf = env->device->name.toUtf8();
			rt.rt_dev = buf.data();
			int skfd = socket(AF_INET, SOCK_DGRAM, 0);
			ioctl(skfd, SIOCDELRT, &rt);
			close(skfd);
		}
		struct nl_addr *local = getNLAddr(ip, netmask);
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
		dhcpAction(packet);
	}
}
