
#include "device.h"
#include "dbus.h"
#include "log.h"
#include <QMutableListIterator>
#include <QProcess>
#include <QStringList>
#include <QTimerEvent>
#include "dhcppacket.h"
#include "random.h"

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
		connect(&hwman, SIGNAL(gotCarrier(const QString&, int, const QString)), SLOT(gotCarrier(const QString &, int, const QString)));
		connect(&hwman, SIGNAL(lostCarrier(const QString&)), SLOT(lostCarrier(const QString &)));
		connect(&hwman, SIGNAL(newDevice(const QString&, int)), SLOT(newDevice(const QString &, int)));
		connect(&hwman, SIGNAL(delDevice(const QString&)), SLOT(delDevice(const QString&)));
		
		connect(this, SIGNAL(deviceAdded(QString, Device*)), &m_events, SLOT(deviceAdded(QString, Device*)));
		connect(this, SIGNAL(deviceRemoved(QString, Device*)), &m_events, SLOT(deviceRemoved(QString, Device*)));
		
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
			if (hwman.ifExists(i.key()))
				addDevice(i.key(), i.value());
		}
	}
	
	void DeviceManager::addDevice(const QString &ifName, nut::DeviceConfig *dc) {
		Device *d = new Device(this, ifName, dc, hwman.hasWLAN(ifName));
		devices.insert(ifName, d);
		emit deviceAdded(ifName, d);
		if (!dc->noAutoStart())
			d->enable(true);
	}
	
	DeviceManager::~DeviceManager() {
		dbus_devMan->stopDBus();
		// manually deleting devices so HardwareManager is available for their destruction;
		// in ~QObject  (deleteChildren) it is too late for them.
		foreach (Device* device, devices)
			delete device;
		devices.clear();
	}
	
	void DeviceManager::ca_timer() {
		if (!ca_evts.empty()) {
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
	
	void DeviceManager::gotCarrier(const QString &ifName, int ifIndex, const QString &essid) {
		if (!essid.isEmpty()) {
			devices[ifName]->gotCarrier(ifIndex, essid);
			return;
		}
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
		Device *dev = devices.value(ifName, 0);
		if (!dev) return;
		if (dev->hasWLAN()) {
			dev->lostCarrier();
			return;
		}
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
	
	void DeviceManager::newDevice(const QString &ifName, int) {
		Device *d = devices.value(ifName, 0);
		if (d) return;
		nut::DeviceConfig *dc = config->getDevices().value(ifName, 0);
		if (!dc) return;
		addDevice(ifName, dc);
//		log << QString("newDevice(%1)").arg(ifName) << endl;
	}
	
	void DeviceManager::delDevice(const QString &ifName) {
		Device *d = devices.value(ifName, 0);
		if (!d) return;
//		log << QString("delDevice(%1)").arg(ifName) << endl;
		d->disable();
		emit deviceRemoved(ifName, d);
		d->deleteLater();
		devices.remove(ifName);
	}
	
	Device::Device(DeviceManager* dm, const QString &name, nut::DeviceConfig *config, bool hasWLAN)
	: QObject(dm), m_arp(this), dm(dm), name(name), interfaceIndex(-1), config(config), activeEnv(-1), nextEnv(-1), m_userEnv(-1), m_waitForEnvSelects(0), m_state(libnut::DS_DEACTIVATED), dhcp_client_socket(-1), m_hasWLAN(hasWLAN), m_wpa_supplicant(0) {
		connect(this, SIGNAL(stateChanged(libnut::DeviceState, libnut::DeviceState, Device*)), &dm->m_events, SLOT(stateChanged(libnut::DeviceState, libnut::DeviceState, Device*)));
		
		int i = 0;
		foreach(nut::EnvironmentConfig *ec, config->getEnvironments())
			envs.push_back(new Environment(this, ec, i++));
	}
	
	Device::~Device() {
		disable();
		if (dhcp_client_socket >= 0)
			closeDHCPClientSocket();
		foreach (Environment* env, envs)
			delete env;
		envs.clear();
	}
	
	void Device::setState(libnut::DeviceState state) {
		libnut::DeviceState ostate = m_state;
		m_state = state;
		if (ostate == m_state) return;
		switch (m_state) {
			case libnut::DS_DEACTIVATED:
			case libnut::DS_ACTIVATED:
				m_arp.stop();
				break;
			case libnut::DS_CARRIER:
			case libnut::DS_UNCONFIGURED:
			case libnut::DS_UP:
				m_arp.start();
				break;
		}
		emit stateChanged(m_state, ostate, this);
	}
	
	void Device::envUp(Environment* env) {
		if (envs[activeEnv] != env) return;
		setState(libnut::DS_UP);
		log << "Device(" << name << ") is up!" << endl;
	}
	void Device::envDown(Environment* env) {
		if (activeEnv < 0 || envs[activeEnv] != env) return;
		setState(libnut::DS_CARRIER);
		log << "Device(" << name << ") is down!" << endl;
		activeEnv = nextEnv;
		nextEnv = -1;
		if (activeEnv != -1)
			envs[activeEnv]->start();
	}
	void Device::envNeedUserSetup(Environment* env) {
		if (envs[activeEnv] != env) return;
		setState(libnut::DS_UNCONFIGURED);
		log << "Device(" << name << ") needs user configuration!" << endl;
	}
	
	void Device::gotCarrier(int ifIndex, const QString &essid) {
		interfaceIndex = ifIndex;
		m_essid = essid;
		m_hasWLAN = !essid.isEmpty();
		nut::MacAddress mAddr = dm->hwman.getMacAddress(name);
		if (mAddr.valid()) macAddress = mAddr;
		if (mAddr.zero()) log << "Device(" << name << "): couldn't get MacAddress from hardware:" << mAddr.toString() << endl;
		if (macAddress.zero()) log << "Device(" << name << "): couldn't get MacAddress" << endl;
		log << "Device(" << name << ") gotCarrier" << endl;
		if (m_hasWLAN) log << "ESSID: " << essid << endl;
		setState(libnut::DS_CARRIER);
		startEnvSelect();
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
	bool Device::sendDHCPClientPacket(DHCPPacket *packet) {
		if (dhcp_client_socket < 0) {
			err << "Cannot send DHCP packet" << endl;
			return false;
		}
		dhcp_write_buf.push_back(packet->msgdata);
		dhcp_write_nf->setEnabled(true);
		return true;
	}
	bool Device::setupDHCPClientSocket() {
		if (interfaceIndex < 0) {
			log << "Interface index invalid" << endl;
			return false;
		}
		if ((dhcp_client_socket = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0) {
			log << "Couldn't open rawsocket for dhcp client" << endl;
			dhcp_client_socket = -1;
			return false;
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
			return false;
		}
		
		dhcp_read_nf = new QSocketNotifier(dhcp_client_socket, QSocketNotifier::Read);
		dhcp_write_nf = new QSocketNotifier(dhcp_client_socket, QSocketNotifier::Write);
		dhcp_write_nf->setEnabled(!dhcp_write_buf.empty());
		connect(dhcp_read_nf, SIGNAL(activated(int)), SLOT(readDHCPClientSocket()));
		connect(dhcp_write_nf, SIGNAL(activated(int)), SLOT(writeDHCPClientSocket()));
		return true;
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
//		log << "writeDHCPClientSocket" << endl;
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
	
	void Device::selectDone(Environment*) {
		m_waitForEnvSelects--;
		checkEnvironment();
	}
	
	void Device::setEnvironment(int env) {
		if (activeEnv == env || nextEnv == env) return;
		log << QString("Set next environment %1").arg(env) << endl;
		if (env < 0 || env >= envs.size()) {
			nextEnv = -1;
			if (activeEnv != -1)
				envs[activeEnv]->stop();
		} else if (activeEnv == -1) {
			activeEnv = env;
			envs[activeEnv]->start();
		} else if (nextEnv == -1) {
			nextEnv = env;
			envs[activeEnv]->stop();
		} else {
			nextEnv = env;
		}
	}
	
	void Device::checkEnvironment() {
		nut::SelectResult user_res;
		if (m_userEnv >= 0) user_res = envs[m_userEnv]->getSelectResult();
		if ((m_userEnv >= 0) && (envs[m_userEnv]->selectionDone())) {
			if (user_res == nut::SelectResult::True || user_res == nut::SelectResult::User) {
				setEnvironment(m_userEnv);
				return;
			}
		}
		if (m_waitForEnvSelects != 0) return;
		// every environment has now selectionDone() == true
		// => m_userEnv is either -1 or m_userEnv cannot be selected.
		if ((activeEnv >= 0) && (envs[activeEnv]->getSelectResult() == nut::SelectResult::True)) return;
		// => select first with SelectResult::True
		foreach (Environment* env, envs) {
			if (env->getSelectResult() == nut::SelectResult::True) {
				setEnvironment(env->m_id);
				return;
			}
		}
		setEnvironment(0);
	}
	
	void Device::startEnvSelect() {
		m_waitForEnvSelects = envs.size();
		foreach (Environment* env, envs)
			env->startSelect();
	}
	
	void Device::setUserPreferredEnvironment(int env) {
		if (env < 0 || env >= envs.size()) {
			m_userEnv = -1;
		} else {
			m_userEnv = env;
		}
		checkEnvironment();
	}
	
	bool Device::enable(bool force) {
		if (m_state == libnut::DS_DEACTIVATED) {
			if (!dm->hwman.controlOn(name, force))
				return false;
			if (!startWPASupplicant())
				return false;
			setState(libnut::DS_ACTIVATED);
		}
		return true;
	}
	void Device::disable() {
		if (m_state != libnut::DS_DEACTIVATED) {
			nextEnv = -1;
			if (activeEnv != -1) {
				envs[activeEnv]->stop();
				activeEnv = -1;
			}
			interfaceIndex = -1;
			stopWPASupplicant();
			dm->hwman.controlOff(name);
			setState(libnut::DS_DEACTIVATED);
		}
	}
	
	bool Device::startWPASupplicant() {
		if (!config->wpaDriver().isEmpty()) {
			m_wpa_supplicant = new QProcess(this);
			QStringList arguments;
			arguments << "-i" << name;
			arguments << "-D" << config->wpaDriver();
			arguments << "-c" << config->wpaConfigFile();
			m_wpa_supplicant->start("/sbin/wpa_supplicant", arguments);
			if (m_wpa_supplicant->waitForStarted(-1)) return true;
			log << "Couldn't start wpa_supplicant" << endl;
			delete m_wpa_supplicant;
			m_wpa_supplicant = 0;
			return false;
		}
		return true;
	}
	
	void Device::stopWPASupplicant() {
		if (m_wpa_supplicant) {
			m_wpa_supplicant->terminate();
			m_wpa_supplicant->waitForFinished(2000);
			m_wpa_supplicant->kill();
			delete m_wpa_supplicant;
			m_wpa_supplicant = 0;
		}
	}

	
	Environment::Environment(Device *device, nut::EnvironmentConfig *config, int id)
	: QObject(device), device(device), config(config), envIsUp(false), envStart(false), m_id(id), selArpWaiting(0), m_needUserSetup(false) {
		foreach (nut::IPv4Config *ic, config->getIPv4Interfaces())
			ifs.push_back(new Interface_IPv4(this, ifs.size(), ic));
		ifUpStatus.fill(false, ifs.size());
		updateNeedUserSetup();
	}
	Environment::~Environment() {
		foreach (Interface* iface, ifs)
			delete iface;
		ifs.clear();
	}
	
	void Environment::checkStatus() {
//		log << QString("checkStatus: %1/%2").arg(ifUpStatus.count(true)).arg(ifUpStatus.size()) << endl;
		if (envStart) {
			if (!envIsUp && ifUpStatus.count(true) == ifUpStatus.size()) {
				envIsUp = true;
				device->envUp(this);
			}
		} else {
			if (envIsUp && ifUpStatus.count(true) == 0) {
				envIsUp = false;
				device->envDown(this);
			}
		}
	}

	void Environment::start() {
		if (envStart) return;
		envStart = true; envIsUp = false;
		bool needUserSetup = false;
		foreach (Interface* i, ifs) {
			i->start();
			needUserSetup |= i->needUserSetup();
		}
		if (needUserSetup)
			device->envNeedUserSetup(this);
		checkStatus();
	}
	void Environment::stop() {
		if (!envStart) return;
		envStart = false; envIsUp = true;
		foreach (Interface* i, ifs)
			i->stop();
		checkStatus();
	}
	
	void Environment::ifUp(Interface* i) {
//		log << QString("Interface %1 up").arg(i->m_index) << endl;
		ifUpStatus[i->m_index] = true;
		checkStatus();
	}
	void Environment::ifDown(Interface* i) {
//		log << QString("Interface %1 down").arg(i->m_index) << endl;
		ifUpStatus[i->m_index] = false;
		checkStatus();
	}
	
	bool Environment::startSelect() {
		selArpWaiting = 0;
		
		ARPRequest *r;
		
		const QVector<nut::SelectRule> &filters = config->getSelect().filters;
		int c = filters.size();
		m_selectResults.resize(c);
		for (int i = 0; i < c; i++) {
			switch (filters[i].selType) {
				case nut::SelectRule::SEL_USER:
					m_selectResults[i] = nut::SelectResult::User;
					break;
				case nut::SelectRule::SEL_ARP:
					r = device->m_arp.requestIPv4(QHostAddress((quint32) 0), filters[i].ipAddr);
					if (!r) return false;
					r->disconnect(this);
					connect(r, SIGNAL(timeout(QHostAddress)), this, SLOT(selectArpRequestTimeout(QHostAddress)));
					connect(r, SIGNAL(foundMac(nut::MacAddress, QHostAddress)), this, SLOT(selectArpRequestFoundMac(nut::MacAddress, QHostAddress)));
					selArpWaiting++;
					break;
				case nut::SelectRule::SEL_ESSID:
					m_selectResults[i] = (filters[i].essid == device->essid()) ? nut::SelectResult::True : nut::SelectResult::False;
					break;
				case nut::SelectRule::SEL_AND_BLOCK:
				case nut::SelectRule::SEL_OR_BLOCK:
					break;
			}
		}
		checkSelectState();
		return true;
	}
	
	void Environment::checkSelectState() {
		if (selArpWaiting > 0) return;
		if (selArpWaiting == -1) {
			err << QString("Environment::checkSelectState called after selection was done") << endl;
			return;
		}
		selArpWaiting = -1;
		const QVector<nut::SelectRule> &filters = config->getSelect().filters;
		const QVector< QVector<quint32> > &blocks = config->getSelect().blocks;
		int c = filters.size();
		if (!c) {
//			qDebug() << QString("Nothing to select") << endl;
			m_selectResult = nut::SelectResult::User;
			device->selectDone(this);
			return;
		}
		for (int i = c-1; i >= 0; i--) {
			if (filters[i].selType == nut::SelectRule::SEL_AND_BLOCK) {
				const QVector<quint32> &block = blocks[filters[i].block];
				nut::SelectResult tmp = nut::SelectResult::True;
				foreach (quint32 j, block) {
					tmp = tmp && m_selectResults[j];
					if (tmp == nut::SelectResult::False) break;
				}
				m_selectResults[i] = tmp;
			} else if (filters[i].selType == nut::SelectRule::SEL_OR_BLOCK) {
				const QVector<quint32> &block = blocks[filters[i].block];
				nut::SelectResult tmp = nut::SelectResult::False;
				foreach (quint32 j, block) {
					tmp = tmp || m_selectResults[j];
					if (tmp == nut::SelectResult::True) break;
				}
				m_selectResults[i] = tmp;
			}
		}
		m_selectResult = m_selectResults[0];
		device->selectDone(this);
//		qDebug() << QString("Select Result: %1").arg((qint8) m_selectResults[0]) << endl;
	}
	
	void Environment::selectArpRequestTimeout(QHostAddress ip) {
//		qDebug() << QString("arp timeout: %1").arg(ip.toString()) << endl;
		const QVector<nut::SelectRule> &filters = config->getSelect().filters;
		int c = filters.size();
		for (int i = 0; i < c; i++) {
			if (filters[i].selType == nut::SelectRule::SEL_ARP && filters[i].ipAddr == ip) {
				m_selectResults[i] = nut::SelectResult::False;
				selArpWaiting--;
			}
		}
		checkSelectState();
	}
	
	void Environment::selectArpRequestFoundMac(nut::MacAddress mac, QHostAddress ip) {
//		qDebug() << QString("arp found: %1 -> %2").arg(ip.toString()).arg(mac.toString()) << endl;
		const QVector<nut::SelectRule> &filters = config->getSelect().filters;
		int c = filters.size();
		for (int i = 0; i < c; i++) {
			if (filters[i].selType == nut::SelectRule::SEL_ARP && filters[i].ipAddr == ip) {
				if ((!filters[i].macAddr.zero()) && (filters[i].macAddr != mac)) {
					m_selectResults[i] = nut::SelectResult::False;
				} else {
					m_selectResults[i] = nut::SelectResult::True;
				}
				selArpWaiting--;
			}
		}
		checkSelectState();
	}
	
	void Environment::updateNeedUserSetup() {
//		bool prev = m_needUserSetup;
		m_needUserSetup = false;
		foreach (Interface *iface, ifs) {
			if (iface->needUserSetup()) {
				m_needUserSetup = true;
				break;
			}
		}
//		if (prev != m_needUserSetup)
			// TODO ?
	}
	
	Interface::Interface(Environment *env, int index) : QObject(env), m_env(env), m_index(index) { }
	Interface::~Interface() { }
	
	void Interface::updateNeedUserSetup(bool needUserSetup) {
		m_needUserSetup = needUserSetup;
		m_env->updateNeedUserSetup();
	}
	
	Interface_IPv4::Interface_IPv4(Environment *env, int index, nut::IPv4Config *config)
	: Interface(env, index), dhcp_timer_id(-1), dm(env->device->dm), dhcp_xid(0), dhcpstate(DHCPS_OFF), zc_state(ZCS_OFF), zc_arp_probe(0), m_config(config), m_ifstate(libnut::IFS_OFF) {
		m_needUserSetup = config->getFlags() & nut::IPv4Config::DO_USERSTATIC;
		connect(this, SIGNAL(statusChanged(libnut::InterfaceState, Interface_IPv4*)), &m_env->device->dm->m_events, SLOT(interfaceStatusChanged(libnut::InterfaceState, Interface_IPv4*)));
	}
	
	Interface_IPv4::~Interface_IPv4() {
		dhcp_set_timeout(-1);
		releaseXID();
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
	
	void Interface_IPv4::dhcp_send_renew() {
		DHCPClientPacket cp(this, dhcp_server_ip);
		cp.doDHCPRenew(ip);
		cp.check();
		cp.send();
	}
	
	void Interface_IPv4::dhcp_send_rebind() {
		DHCPClientPacket cp(this);
		cp.doDHCPRebind(ip);
		cp.check();
		cp.send();
	}
	
	void Interface_IPv4::dhcp_send_release() {
		DHCPClientPacket cp(this, dhcp_server_ip);
		cp.doDHCPRelease(ip, dhcp_server_identifier);
		cp.check();
		cp.send();
	}
	
	void Interface_IPv4::dhcp_setup_interface(DHCPPacket *ack, bool renewing) {
		if (renewing) {
			dhcp_lease_time = ntohl(ack->getOptionData<quint32>(DHCP_LEASE_TIME, -1));
		} else {
			ip = ack->getYourIP();
			netmask = ack->getOptionAddress(DHCP_SUBNET);
			gateway = ack->getOptionAddress(DHCP_ROUTER);
			dnsserver = ack->getOptionAddresses(DHCP_DNS_SERVER);
			localdomain = ack->getOptionString(DHCP_DOMAIN_NAME);
			dhcp_server_identifier = ack->getOption(DHCP_SERVER_ID);
			dhcp_server_ip = QHostAddress(ntohl(ack->headers.ip.saddr));
			dhcp_lease_time = ntohl(ack->getOptionData<quint32>(DHCP_LEASE_TIME, -1));
				// T1: 0.5 * dhcp_lease_time
				// T2: 0.875 * dhcp_lease_time ( 7/8 )
			m_ifstate = libnut::IFS_DHCP;
			systemUp();
		}
	}
	
	void Interface_IPv4::dhcp_set_timeout(int msec) {
		if (dhcp_timer_id != -1) killTimer(dhcp_timer_id);
		if (msec != -1)
			dhcp_timer_id = startTimer(msec);
		else
			dhcp_timer_id = -1;
	}
	
	void Interface_IPv4::zeroconf_setup_interface() {
		if (m_ifstate != libnut::IFS_OFF) return;
		ip = zc_probe_ip;
		netmask = QHostAddress((quint32) 0xFFFF0000);
		gateway = QHostAddress((quint32) 0);
		dnsserver.clear();
		m_ifstate = libnut::IFS_ZEROCONF;
		systemUp();
	}
	
	quint16 hashMac(const nut::MacAddress &addr) {
		quint16 *ptr = (quint16*) addr.data;
		return (quint16) (ptr[0] ^ ptr[1] ^ ptr[2]);
	}
	
	void Interface_IPv4::zeroconfProbe() {   // select new ip and start probe it
		zeroconfFree();
		quint32 lastip = zc_probe_ip.toIPv4Address();
		const quint32 baseip = 0xA9FE0000; // 169.254.0.0
		const quint32 mask = 0xFFFF;
		quint32 rnd = hashMac(m_env->device->getMacAddress()), ip;
		do {
			while ((rnd >> 8) == 0 || (rnd >> 8) == 0xFF)
				rnd = getRandomUInt32() & mask;
			ip = baseip | rnd;
		} while (ip == lastip);
		zc_probe_ip = QHostAddress(ip);
		zc_arp_probe = m_env->device->m_arp.probeIPv4(zc_probe_ip);
		if (zc_arp_probe->getState() != ARPProbe::PROBING) {
			err << "ARPProbe failed" << endl;
			return;
		}
		zc_arp_probe->setReserve(m_config->getFlags() & nut::IPv4Config::DO_DHCP);
		connect(zc_arp_probe, SIGNAL(conflict(QHostAddress, nut::MacAddress)), SLOT(zc_conflict()));
		connect(zc_arp_probe, SIGNAL(ready(QHostAddress)), SLOT(zc_ready()));
	}
	
	void Interface_IPv4::zeroconfFree() {    // free ARPProbe and ARPWatch
		if (zc_arp_probe) {
			delete zc_arp_probe;
			zc_arp_probe = 0;
		}
	}
	
	void Interface_IPv4::zeroconfAction() {
		if (!(m_config->getFlags() & nut::IPv4Config::DO_ZEROCONF))
			return;
		switch (zc_state) {
			case ZCS_OFF:
				zeroconfFree();
				break;
			case ZCS_START:
				zc_probe_ip = QHostAddress((quint32) 0);
				zeroconfProbe();
				zc_state = ZCS_PROBING;
				break;
			case ZCS_PROBING:
				break;
			case ZCS_RESERVING:
				if (dhcp_retry > 3) {
					delete zc_arp_probe;
					zc_arp_probe = 0;
				}
				break;
			case ZCS_ANNOUNCING:
				break;
			case ZCS_BOUND:
				break;
			case ZCS_CONFLICT:
				zeroconfProbe();
				zc_state = ZCS_PROBING;
				break;
		}
	}
	
	void Interface_IPv4::zc_conflict() {
		zc_arp_probe = 0;
		zc_state = ZCS_CONFLICT;
		zeroconfAction();
	}
	
	void Interface_IPv4::zc_ready() {
		if (m_config->getFlags() & nut::IPv4Config::DO_DHCP) {
			zc_state = ZCS_RESERVING;
			zeroconfAction();
		} else {
			zc_arp_probe = 0;
			// TODO: Announce
			zc_state = ZCS_BOUND;
			zeroconf_setup_interface();
		}
	}
	
	void Interface_IPv4::dhcpAction(DHCPPacket *source) {
		for (;;) {
			switch (dhcpstate) {
				case DHCPS_OFF:
					releaseXID();
					return;
				case DHCPS_INIT_START:
					dhcp_retry = 0;
					// fall through:
					// dhcpstate = DHCPS_INIT;
				case DHCPS_INIT:
					dhcp_send_discover();
					dhcpstate = DHCPS_SELECTING;
					break;
				case DHCPS_SELECTING:
					if (source) {
						if (source->getMessageType() == DHCP_OFFER)  {
							killTimer(dhcp_timer_id);
							dhcp_send_request(source);
							dhcpstate = DHCPS_REQUESTING;
						}
					} else {
						if (dhcp_retry < 5) {
							dhcp_retry++;
							// send 6 packets with short interval
							// 0, 500, 2500, 4500, 6500, 8500 [ms]
							dhcp_set_timeout(500 + (dhcp_retry-1) * 2000);
						} else {
							// 1 min
							dhcp_set_timeout(1 * 60 * 1000);
						}
						return;
					}
				case DHCPS_REQUESTING:
					if (source) {
						switch (source->getMessageType()) {
							case DHCP_ACK:
								killTimer(dhcp_timer_id);
								dhcp_setup_interface(source);
								dhcpstate = DHCPS_BOUND;
								break;
							case DHCP_NAK:
								killTimer(dhcp_timer_id);
								dhcpstate = DHCPS_INIT;
								break;
							default:
								break;
						}
						break;
					} else {
						dhcp_set_timeout(4000);
						return;
					}
				case DHCPS_BOUND:
					releaseXID();
					// 0.5 * 1000 (msecs)
					dhcp_set_timeout(500 * dhcp_lease_time);
					return;
				case DHCPS_RENEWING:
					if (source) {
						switch (source->getMessageType()) {
							case DHCP_ACK:
								killTimer(dhcp_timer_id);
								dhcp_setup_interface(source, true);
								dhcpstate = DHCPS_BOUND;
								break;
							case DHCP_NAK:
								killTimer(dhcp_timer_id);
								dhcpstate = DHCPS_INIT_START;
								break;
							default:
								break;
						}
						break;
					} else {
						dhcp_send_renew();
						// T2 - T1 * 1000 (msecs) = (7 / 8 - 1 / 2) * 1000 = 3 / 8 * 1000 = 375
						dhcp_set_timeout(375 * dhcp_lease_time);
						return;
					}
				case DHCPS_REBINDING:
					if (source) {
						switch (source->getMessageType()) {
							case DHCP_ACK:
								killTimer(dhcp_timer_id);
								dhcp_setup_interface(source, true);
								dhcpstate = DHCPS_BOUND;
								break;
							case DHCP_NAK:
								killTimer(dhcp_timer_id);
								dhcpstate = DHCPS_INIT_START;
								break;
							default:
								break;
						}
						break;
					} else {
						dhcp_send_rebind();
						// T - (T2) = 1/8 -> 125
						dhcp_set_timeout(125 * dhcp_lease_time);
						return;
					}
				default:
					log << "Unhandled dhcp state" << endl;
					return;
			}
			source = 0;
		}
	}
	
	void Interface_IPv4::timerEvent(QTimerEvent *tevt) {
		if (tevt->timerId() != dhcp_timer_id) {
			err << "Unrequested timer Event: " << tevt->timerId() << endl;
//			return;
		}
		dhcp_set_timeout(-1);
		switch (dhcpstate) {
			case DHCPS_SELECTING:
				dhcpstate = DHCPS_INIT;
				break;
			case DHCPS_REQUESTING:
				dhcpstate = DHCPS_INIT;
				break;
			case DHCPS_BOUND:
				dhcpstate = DHCPS_RENEWING;
				break;
			case DHCPS_RENEWING:
				dhcpstate = DHCPS_REBINDING;
				break;
			case DHCPS_REBINDING:
				dhcpstate = DHCPS_INIT_START;
				break;
			default:
				break;
		}
		dhcpAction();
	}
	
	void Interface_IPv4::startDHCP() {
		dhcpstate = DHCPS_INIT_START;
		dhcpAction();
	}
	void Interface_IPv4::stopDHCP() {
		switch (dhcpstate) {
			case DHCPS_INITREBOOT:  // nothing to do ??
			case DHCPS_REBOOTING:  // nothing to do ??
			
			case DHCPS_OFF: // should not happen
			case DHCPS_INIT_START: // should not happen
			case DHCPS_INIT: // should not happen
			case DHCPS_SELECTING: // normal cleanup
				break;
			case DHCPS_REQUESTING: // release
			case DHCPS_BOUND: // release
			case DHCPS_RENEWING: // release
			case DHCPS_REBINDING: // release
				dhcp_send_release();
		}
		releaseXID();
		dhcp_set_timeout(-1);
	}
	
	void Interface_IPv4::startZeroconf() {
		zc_state = ZCS_START;
		zeroconfAction();
	}
	void Interface_IPv4::startStatic() {
		ip = m_config->getStaticIP();
		netmask = m_config->getStaticNetmask();
		gateway = m_config->getStaticGateway();
		dnsserver = m_config->getStaticDNS();
		m_ifstate = libnut::IFS_STATIC;
		systemUp();
	}
	void Interface_IPv4::startUserStatic() {
		ip = m_userConfig.ip();
		netmask = m_userConfig.netmask();
		gateway = m_userConfig.gateway();
		dnsserver = m_userConfig.dnsservers();
		m_ifstate = libnut::IFS_STATIC;
		systemUp();
	}

	void Interface_IPv4::start() {
		log << "Interface_IPv4::start" << endl;
		if (m_config->getFlags() & nut::IPv4Config::DO_DHCP) {
			startDHCP();
		} else if (m_config->getFlags() & nut::IPv4Config::DO_USERSTATIC) {
			if (m_needUserSetup) {
				m_ifstate = libnut::IFS_WAITFORCONFIG;
				emit statusChanged(m_ifstate, this);
			} else {
				startUserStatic();
			}
		} else if (m_config->getFlags() & nut::IPv4Config::DO_ZEROCONF) {
			startZeroconf();
		} else {
			startStatic();
		}
	}
	
	void Interface_IPv4::stop() {
		log << "Interface_IPv4::stop" << endl;
		if (dhcpstate != DHCPS_OFF)
			stopDHCP();
		zeroconfFree();
		systemDown();
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
	
	inline struct nl_addr* getNLBroadcast(const QHostAddress &addr, const QHostAddress &netmask) {
		quint32 nm = 0;
		if (!netmask.isNull()) nm = htonl(netmask.toIPv4Address());
		quint32 i = htonl(addr.toIPv4Address());
		quint32 bcast = (i & nm) | (~nm);
		struct nl_addr* a = nl_addr_build(AF_INET, &bcast, sizeof(bcast));
		return a;
	}
	
	inline struct nl_addr* getNLAddr(const QHostAddress &addr, const QHostAddress &netmask) {
		quint32 i = htonl(addr.toIPv4Address());
		struct nl_addr* a = nl_addr_build(AF_INET, &i, sizeof(i));
		if (!netmask.isNull())
			nl_addr_set_prefixlen(a, getPrefixLen(netmask));
		return a;
	}
	
	void Interface_IPv4::systemUp() {
		struct nl_addr *local = getNLAddr(ip, netmask), *bcast = getNLBroadcast(ip, netmask);
//		char buf[32];
//		log << nl_addr2str (local, buf, 32) << endl;
		struct rtnl_addr *addr = rtnl_addr_alloc();
		rtnl_addr_set_ifindex(addr, m_env->device->interfaceIndex);
		rtnl_addr_set_family(addr, AF_INET);
		rtnl_addr_set_local(addr, local);
		rtnl_addr_set_broadcast(addr, bcast);
#if 0
		log << "systemUp: addr_add = " << rtnl_addr_add(dm->hwman.getNLHandle(), addr, 0) << endl;
#else
		rtnl_addr_add(dm->hwman.getNLHandle(), addr, 0);
#endif
		rtnl_addr_put(addr);
		nl_addr_put(local);
		nl_addr_put(bcast);
		// Gateway
		if (!gateway.isNull()) {
//			log << "Try setting gateway" << endl;
			struct rtentry rt;
			memset(&rt, 0, sizeof(rt));
			rt.rt_flags = RTF_UP | RTF_GATEWAY;
			setSockaddrIPv4(rt.rt_dst);
			setSockaddrIPv4(rt.rt_gateway, gateway.toIPv4Address());
			setSockaddrIPv4(rt.rt_genmask);
			QByteArray buf = m_env->device->name.toUtf8();
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
			arguments << "-a" << QString("%1_%2").arg(m_env->device->name).arg(m_index);
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
		emit statusChanged(m_ifstate, this);
		m_env->ifUp(this);
	}
	void Interface_IPv4::systemDown() {
		// Resolvconf
		QProcess *proc = new QProcess();
		QStringList arguments;
		arguments << "-d" << QString("%1_%2").arg(m_env->device->name).arg(m_index);
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
			QByteArray buf = m_env->device->name.toUtf8();
			rt.rt_dev = buf.data();
			int skfd = socket(AF_INET, SOCK_DGRAM, 0);
			ioctl(skfd, SIOCDELRT, &rt);
			close(skfd);
		}
		struct nl_addr *local = getNLAddr(ip, netmask);
//		char buf[32];
//		log << nl_addr2str (local, buf, 32) << endl;
		struct rtnl_addr *addr = rtnl_addr_alloc();
		rtnl_addr_set_ifindex(addr, m_env->device->interfaceIndex);
		rtnl_addr_set_family(addr, AF_INET);
		rtnl_addr_set_local(addr, local);
#if 0
		log << "systemUp: addr_delete = " << rtnl_addr_delete(dm->hwman.getNLHandle(), addr, 0) << endl;
#else
		rtnl_addr_delete(dm->hwman.getNLHandle(), addr, 0);
#endif
		rtnl_addr_put(addr);
		nl_addr_put(local);
		m_ifstate = libnut::IFS_OFF;
		emit statusChanged(m_ifstate, this);
		m_env->ifDown(this);
	}
	
	bool Interface_IPv4::registerXID(quint32 xid) {
		releaseXID();
		if (m_env->device->registerXID(xid, this)) {
			dhcp_xid_unicast = false;
			dhcp_xid = xid;
			return true;
		}
		return false;
	}
	
	bool Interface_IPv4::registerUnicastXID(quint32 xid) {
		releaseXID();
		if (!xid) return false;
		if (setupUnicastDHCP()) {
			dhcp_xid_unicast = true;
			dhcp_xid = xid;
			return true;
		}
		return false;
	}
	
	void Interface_IPv4::releaseXID() {
		if (dhcp_xid) {
			if (dhcp_xid_unicast) {
				closeUnicastDHCP();
			} else{
				m_env->device->unregisterXID(dhcp_xid);
			}
			dhcp_xid = 0;
		}
	}
	
	void Interface_IPv4::dhcpReceived(DHCPPacket *packet) {
		dhcpAction(packet);
	}

	bool Interface_IPv4::setupUnicastDHCP(bool temporary) {
		dhcp_unicast_socket = socket(PF_INET, SOCK_DGRAM, 0);
		if (dhcp_unicast_socket == -1) {
			err << "Couldn't create UDP socket" << endl;
			return false;
		}
		struct sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_port = htons(68);
		sin.sin_addr.s_addr = htonl(ip.toIPv4Address());
		if (-1 == bind(dhcp_unicast_socket, (struct sockaddr*) &sin, sizeof(sin))) {
			err << "Couldn't bind on local dhcp client socket" << endl;
			close(dhcp_unicast_socket);
			dhcp_unicast_socket = -1;
			return false;
		}
		dhcp_unicast_read_nf = 0;
		if (!temporary) {
			dhcp_unicast_read_nf = new QSocketNotifier(dhcp_unicast_socket, QSocketNotifier::Read);
			connect(dhcp_unicast_read_nf, SIGNAL(activated(int)), SLOT(readDHCPUnicastClientSocket()));
		}
		return true;
	}
	
	void Interface_IPv4::closeUnicastDHCP() {
		if (dhcp_unicast_read_nf) {
			delete dhcp_unicast_read_nf;
			dhcp_unicast_read_nf = 0;
		}
		close(dhcp_unicast_socket);
		dhcp_unicast_socket = -1;
	}
	
	bool Interface_IPv4::sendUnicastDHCP(DHCPPacket *packet) {
		bool tmpSocket = !dhcp_xid || !dhcp_xid_unicast;
		if (tmpSocket && !setupUnicastDHCP(true)) return false;
		struct sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_port = htons(67);
		sin.sin_addr.s_addr = htonl(packet->unicast_addr.toIPv4Address());
		sendto(dhcp_unicast_socket, packet->msgdata.data() + sizeof(packet->headers), packet->msgdata.length() - sizeof(packet->headers), 0, (struct sockaddr*) &sin, sizeof(sin));
		if (tmpSocket) closeUnicastDHCP();
		return true;
	}

	void Interface_IPv4::readDHCPUnicastClientSocket() {
		struct sockaddr_in sin;
		socklen_t slen = sizeof(sin);
		QByteArray buf;
		int nread, msgsize = 256 + 1;
		do {
			msgsize = (msgsize << 1) - 1;
			buf.resize(msgsize);
			nread = recvfrom(dhcp_unicast_socket, buf.data(), msgsize, MSG_PEEK, (struct sockaddr *)&sin, &slen);
			if (nread < 0) {
//				perror("Device::readDHCPClientSocket: recvfrom");
				closeUnicastDHCP();
			}
			if (nread == 0) return;
		} while (nread == msgsize);
		buf.resize(nread);
		recvfrom(dhcp_unicast_socket, buf.data(), nread, 0, (struct sockaddr *)&sin, &slen);
		DHCPPacket *packet = DHCPPacket::parseData(buf, sin);
		if (!packet) return;
		dhcpAction(packet);
		delete packet;
	}
	
	bool Interface_IPv4::setUserConfig(const nut::IPv4UserConfig &userConfig) {
		if (!(m_config->getFlags() & nut::IPv4Config::DO_USERSTATIC)) return false;
		m_userConfig = userConfig;
		updateNeedUserSetup(m_userConfig.valid());
		return true;
	}

}
