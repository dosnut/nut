
#include "configparser.h"
#include "device.h"
#include "log.h"
#include <QMutableListIterator>
#include <QProcess>
#include <QStringList>
#include <QFile>
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
#include <unistd.h>

void perror(const char *s);
}

static inline void setSockaddrIPv4(struct sockaddr &s, quint32 host = 0, quint16 port = 0) {
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = htonl(host);
	memcpy(&s, &sa, sizeof(struct sockaddr_in));
}

namespace nuts {
	using namespace libnutcommon;

	DeviceManager::DeviceManager(const QString &configFile, ProcessManager* processManager)
	: m_events(processManager), m_config(parseConfig(configFile)) {
		connect(&m_hwman, &HardwareManager::gotCarrier, this, &DeviceManager::gotCarrier);
		connect(&m_hwman, &HardwareManager::lostCarrier, this, &DeviceManager::lostCarrier);
		connect(&m_hwman, &HardwareManager::newDevice, this, &DeviceManager::newDevice);
		connect(&m_hwman, &HardwareManager::delDevice, this, &DeviceManager::delDevice);

		connect(this, &DeviceManager::deviceAdded, &m_events, &Events::deviceAdded);
		connect(this, &DeviceManager::deviceRemoved, &m_events, &Events::deviceRemoved);

		 /* Wait for 400 ms before deliver carrier events
		   There are 2 reasons:
		    - a kernel bug: dhcp messages cannot be sent immediately
		      after carrier event, 100 ms wait was not enough. (they simply
		      don't reach the hardware, local packet sniffers get them).
		    - in the case of lostCarrier, we want ignore short breaks.
		*/
		m_carrier_timer.setInterval(400);
		connect(&m_carrier_timer, &QTimer::timeout, this, &DeviceManager::ca_timer);
		addDevices();
	}

	void DeviceManager::addDevices() {
		foreach(QString real_dev, m_hwman.get_ifNames()) {
			auto devConfig = m_config.lookup(real_dev);
			if (devConfig) {
				addDevice(real_dev, devConfig);
			}
		}
	}

	void DeviceManager::addDevice(const QString &ifName, std::shared_ptr<DeviceConfig> dc) {
		Device *d = new Device(this, ifName, dc, m_hwman.hasWLAN(ifName));
		m_devices.insert(ifName, d);
		emit deviceAdded(ifName, d);
		if (!dc->noAutoStart) d->enable(true);
	}

	DeviceManager::~DeviceManager() {
		// shutdown() should be used to cleanup before

		// manually deleting devices so HardwareManager is available for their destruction;
		// in ~QObject  (deleteChildren) it is too late for them.
		foreach (Device* device, m_devices)
			delete device;
		m_devices.clear();
	}

	void DeviceManager::shutdown()
	{
		// no new hardware anymore
		disconnect(&m_hwman, &HardwareManager::newDevice, 0, 0);

		for (QString const& ifName: m_devices.keys()) {
			Device* device = m_devices.value(ifName, nullptr);
			if (device) delete device;
		}
	}

	void DeviceManager::ca_timer() {
		if (!m_ca_evts.empty()) {
			struct ca_evt e = m_ca_evts.takeFirst();
			Device * dev = m_devices.value(e.ifName,0);
			if (dev) {
				if (e.up)
					dev->gotCarrier(e.ifIndex);
				else
					dev->lostCarrier();
			}
		}
		if (m_ca_evts.empty())
			m_carrier_timer.stop();
	}

	void DeviceManager::gotCarrier(const QString &ifName, int ifIndex, const QString &essid) {
		if (!essid.isEmpty()) {
			m_devices[ifName]->gotCarrier(ifIndex, essid);
			return;
		}
		QMutableLinkedListIterator<struct ca_evt> i(m_ca_evts);
		while(i.hasNext()) {
			if (i.next().ifName == ifName) {
				i.remove();
				if (m_ca_evts.empty())
					m_carrier_timer.stop();
				return;
			}
		}
		struct ca_evt e;
		e.ifName = ifName;
		e.ifIndex = ifIndex;
		e.up = true;
		m_ca_evts.push_back(e);
		// do not restart the timer if a item is already in the queue
		if (!m_carrier_timer.isActive())
			m_carrier_timer.start();
	}
	void DeviceManager::lostCarrier(const QString &ifName) {
		Device *dev = m_devices.value(ifName, 0);
		if (!dev) return;
		if (dev->isAir()) {
			dev->lostCarrier();
			return;
		}
		QMutableLinkedListIterator<struct ca_evt> i(m_ca_evts);
		while(i.hasNext()) {
			if (i.next().ifName == ifName) {
				i.remove();
				if (m_ca_evts.empty())
					m_carrier_timer.stop();
				return;
			}
		}
		struct ca_evt e;
		e.ifName = ifName;
		e.ifIndex = 0;
		e.up = false;
		m_ca_evts.push_back(e);
		// do not restart the timer if a item is already in the queue
		if (!m_carrier_timer.isActive())
			m_carrier_timer.start();
	}

	void DeviceManager::newDevice(const QString &ifName, int) {
		Device *d = m_devices.value(ifName, 0);
		if (d) return;

		auto devConfig = m_config.lookup(ifName);
		if (devConfig) {
			addDevice(ifName, devConfig);
		}
	}

	void DeviceManager::delDevice(const QString &ifName) {
		Device *d = m_devices.value(ifName, 0);
		if (!d) return;
//		log << QString("delDevice(%1)").arg(ifName) << endl;
		d->disable();
		emit deviceRemoved(ifName, d);
		d->deleteLater();
		m_devices.remove(ifName);
	}

	//Initialize with default values, add environments, connect to dm-events
	Device::Device(DeviceManager* dm, const QString &name, std::shared_ptr<DeviceConfig> config, bool hasWLAN)
	: QObject(dm), m_arp(this), m_dm(dm), m_interfaceIndex(-1), m_config(config), m_activeEnv(-1), m_nextEnv(-1), m_userEnv(-1), m_waitForEnvSelects(0), m_dhcp_client_socket(-1), m_wpa_supplicant(0) {
		m_properties.name = name;
		m_properties.type = hasWLAN ? DeviceType::AIR : DeviceType::ETH;
		m_properties.state = DeviceState::DEACTIVATED;

		int i = 0;
		for (auto const& ec: m_config->environments) {
			m_envs.push_back(new Environment(this, ec, i++));
		}
	}

	Device::~Device() {
		disable();
		if (m_dhcp_client_socket >= 0)
			closeDHCPClientSocket();
		foreach (Environment* env, m_envs)
			delete env;
		m_envs.clear();
	}

	void Device::setState(DeviceState state) {
		auto ostate = m_properties.state;
		if (ostate == state) return;
		m_properties.state = state;
		switch (m_properties.state) {
		case DeviceState::DEACTIVATED:
		case DeviceState::ACTIVATED:
			m_properties.essid.clear();
			// mac address usually doesn't change - not clearing it.
			// m_properties.macAddress.clear();
			m_arp.stop();
			break;
		case DeviceState::CARRIER:
		case DeviceState::UNCONFIGURED:
		case DeviceState::UP:
			m_arp.start();
			break;
		}

		m_dm->m_events.stateChanged(state, ostate, this);
		emit propertiesChanged(m_properties);
	}

	void Device::envUp(Environment* env) {
		if (m_envs[m_activeEnv] != env) return;
		setState(DeviceState::UP);
		log << "Device(" << m_properties.name << ") is up!" << endl;
	}
	void Device::envDown(Environment* env) {
		if (m_activeEnv < 0 || m_envs[m_activeEnv] != env) return;
		setState(DeviceState::CARRIER);
		log << "Device(" << m_properties.name << ") is down!" << endl;
		m_activeEnv = m_nextEnv;
		m_nextEnv = -1;
		if (m_activeEnv != -1)
			m_envs[m_activeEnv]->start();
		emit activeEnvironmentChanged(m_activeEnv);
	}
	void Device::envNeedUserSetup(Environment* env) {
		if (m_envs[m_activeEnv] != env) return;
		setState(DeviceState::UNCONFIGURED);
		log << "Device(" << m_properties.name << ") needs user configuration!" << endl;
	}

	void Device::gotCarrier(int ifIndex, const QString &essid) {
		m_interfaceIndex = ifIndex;
		m_properties.essid = essid;
		auto hasWLAN = (QFile::exists(QString("/sys/class/net/%1/wireless").arg(m_properties.name))) || !essid.isEmpty();
		m_properties.type = hasWLAN ? DeviceType::AIR : DeviceType::ETH;
		m_properties.macAddress = m_dm->m_hwman.getMacAddress(m_properties.name);
		if (m_properties.macAddress.zero()) log << "Device(" << m_properties.name << "): couldn't get MacAddress" << endl;
		log << "Device(" << m_properties.name << ") gotCarrier" << endl;
		if (hasWLAN) log << "ESSID: " << essid << endl;
		setState(DeviceState::CARRIER);
		startEnvSelect();
	}
	void Device::lostCarrier() {
		log << "Device(" << m_properties.name << ") lostCarrier" << endl;
		m_nextEnv = -1;
		if (m_activeEnv != -1) {
			m_envs[m_activeEnv]->stop();
			m_activeEnv = -1;
			emit activeEnvironmentChanged(m_activeEnv);
		}
		m_interfaceIndex = -1;
		setState(DeviceState::ACTIVATED);
	}
	bool Device::registerXID(quint32 xid, Interface_IPv4 *iface) {
		if (!xid) return false;
		if (m_dhcp_xid_iface.contains(xid)) return false;
		m_dhcp_xid_iface.insert(xid, iface);
		if (m_dhcp_client_socket < 0)
			setupDHCPClientSocket();
		return true;
	}
	void Device::unregisterXID(quint32 xid) {
		if (!xid) return;
		m_dhcp_xid_iface.remove(xid);
		if (m_dhcp_xid_iface.count() == 0 && m_dhcp_client_socket >= 0)
			closeDHCPClientSocket();
	}
	bool Device::sendDHCPClientPacket(DHCPPacket *packet) {
		if (m_dhcp_client_socket < 0) {
			err << "Cannot send DHCP packet" << endl;
			return false;
		}
		m_dhcp_write_buf.push_back(packet->msgdata);
		m_dhcp_write_nf->setEnabled(true);
		return true;
	}
	bool Device::setupDHCPClientSocket() {
		if (m_interfaceIndex < 0) {
			log << "Interface index invalid" << endl;
			return false;
		}
		if ((m_dhcp_client_socket = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0) {
			log << "Couldn't open rawsocket for dhcp client" << endl;
			m_dhcp_client_socket = -1;
			return false;
		}
		const char MAC_BCAST_ADDR[] = "\xff\xff\xff\xff\xff\xff";
		struct sockaddr_ll sock;
		memset(&sock, 0, sizeof(sock));
		sock.sll_family = AF_PACKET;
		sock.sll_protocol = htons(ETH_P_IP);
		sock.sll_ifindex = m_interfaceIndex;
		sock.sll_halen = 6;
		memcpy(sock.sll_addr, MAC_BCAST_ADDR, 6);
		if (bind(m_dhcp_client_socket, (struct sockaddr *) &sock, sizeof(sock)) < 0) {
			log << "Couldn't bind socket for dhcp client" << endl;
			close(m_dhcp_client_socket);
			m_dhcp_client_socket = -1;
			return false;
		}

		m_dhcp_read_nf = new QSocketNotifier(m_dhcp_client_socket, QSocketNotifier::Read);
		m_dhcp_write_nf = new QSocketNotifier(m_dhcp_client_socket, QSocketNotifier::Write);
		m_dhcp_write_nf->setEnabled(!m_dhcp_write_buf.empty());
		connect(m_dhcp_read_nf, &QSocketNotifier::activated, this, &Device::readDHCPClientSocket);
		connect(m_dhcp_write_nf, &QSocketNotifier::activated, this, &Device::writeDHCPClientSocket);
		return true;
	}
	void Device::closeDHCPClientSocket() {
		delete m_dhcp_read_nf; m_dhcp_read_nf = 0;
		delete m_dhcp_write_nf; m_dhcp_write_nf = 0;
		close(m_dhcp_client_socket);
		m_dhcp_client_socket = -1;
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
			nread = recvfrom(m_dhcp_client_socket, buf.data(), msgsize, MSG_PEEK, (struct sockaddr *)&sock, &slen);
			if (nread < 0) {
//				perror("Device::readDHCPClientSocket: recvfrom");
				closeDHCPClientSocket();
			}
			if (nread == 0) return;
		} while (nread == msgsize);
		buf.resize(nread);
		recvfrom(m_dhcp_client_socket, buf.data(), nread, 0, (struct sockaddr *)&sock, &slen);
		DHCPPacket *packet = DHCPPacket::parseRaw(buf);
		if (!packet) return;
		// check mac
		if (packet->getClientMac() != m_properties.macAddress)
			goto cleanup;
		xid = packet->getXID();
		iface = m_dhcp_xid_iface.value(xid);
		if (!iface)
			goto cleanup;
		iface->dhcpReceived(packet);
	cleanup:
		delete packet;
	}

	void Device::writeDHCPClientSocket() {
//		log << "writeDHCPClientSocket" << endl;
		if (!m_dhcp_write_buf.empty()) {
			QByteArray msgdata = m_dhcp_write_buf.takeFirst();
			// raw_packet(&packet, INADDR_ANY, CLIENT_PORT, INADDR_BROADCAST,
			// SERVER_PORT, MAC_BCAST_ADDR, client_config.ifindex);
			const char MAC_BCAST_ADDR[] = "\xff\xff\xff\xff\xff\xff";
			struct sockaddr_ll sock;
			memset(&sock, 0, sizeof(sock));
			sock.sll_family = AF_PACKET;
			sock.sll_protocol = htons(ETH_P_IP);
			sock.sll_ifindex = m_interfaceIndex;
			sock.sll_halen = 6;
			memcpy(sock.sll_addr, MAC_BCAST_ADDR, 6);
			sendto(m_dhcp_client_socket, msgdata.data(), msgdata.size(), 0, (struct sockaddr *) &sock, sizeof(sock));
		}
		m_dhcp_write_nf->setEnabled(!m_dhcp_write_buf.empty());
	}

	void Device::selectDone(Environment*) {
		m_waitForEnvSelects--;
		checkEnvironment();
	}

	void Device::setEnvironment(int env) {
		if (m_activeEnv == env || m_nextEnv == env) return;
		log << QString("Set next environment %1").arg(env) << endl;
		if (env < 0 || env >= m_envs.size()) {
			m_nextEnv = -1;
			if (m_activeEnv != -1) {
				m_envs[m_activeEnv]->stop();
			}
		} else if (m_activeEnv == -1) {
			m_activeEnv = env;
			m_envs[m_activeEnv]->start();
			emit activeEnvironmentChanged(m_activeEnv);
		} else if (m_nextEnv == -1) {
			m_nextEnv = env;
			m_envs[m_activeEnv]->stop();
		} else {
			m_nextEnv = env;
		}
	}

	void Device::checkEnvironment() {
		if (m_userEnv >= 0 && m_envs[m_userEnv]->selectionDone()) {
			auto user_res = m_envs[m_userEnv]->getSelectResult();
			if (user_res == SelectResult::True || user_res == SelectResult::User) {
				setEnvironment(m_userEnv);
				return;
			}
		}
		if (m_waitForEnvSelects != 0) return;
		// every environment has now selectionDone() == true
		// => m_userEnv is either -1 or m_userEnv cannot be selected.
		if ((m_activeEnv >= 0) && (m_envs[m_activeEnv]->getSelectResult() == SelectResult::True)) return;
		// => select first with SelectResult::True
		foreach (Environment* env, m_envs) {
			if (env->getSelectResult() == SelectResult::True) {
				setEnvironment(env->m_properties.id);
				return;
			}
		}
		setEnvironment(0);
	}

	void Device::startEnvSelect() {
		m_waitForEnvSelects = m_envs.size();
		foreach (Environment* env, m_envs)
			env->startSelect();
	}

	void Device::setUserPreferredEnvironment(int env) {
		if (env < 0 || env >= m_envs.size()) {
			m_userEnv = -1;
		} else {
			m_userEnv = env;
		}
		checkEnvironment();
	}

	bool Device::enable(bool force) {
		if (DeviceState::DEACTIVATED == m_properties.state) {
			if (!m_dm->m_hwman.controlOn(m_properties.name, force))
				return false;
			if (!startWPASupplicant())
				return false;
			setState(DeviceState::ACTIVATED);
		}
		return true;
	}
	void Device::disable() {
		if (DeviceState::DEACTIVATED != m_properties.state) {
			m_nextEnv = -1;
			if (m_activeEnv != -1) {
				m_envs[m_activeEnv]->stop();
				m_activeEnv = -1;
				emit activeEnvironmentChanged(m_activeEnv);
			}
			m_interfaceIndex = -1;
			stopWPASupplicant();
			m_dm->m_hwman.controlOff(m_properties.name);
			setState(DeviceState::DEACTIVATED);
		}
	}

	bool Device::startWPASupplicant() {
		if (!m_config->wpaDriver.isEmpty()) {
			m_wpa_supplicant = new QProcess(this);
			QStringList arguments;
			arguments << "-i" << m_properties.name;
			arguments << "-D" << m_config->wpaDriver;
			arguments << "-c" << m_config->wpaConfigFile;
			m_wpa_supplicant->start("wpa_supplicant", arguments);
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


	Environment::Environment(Device *device, std::shared_ptr<EnvironmentConfig> config, int id)
	: QObject{device}, m_device{device}, m_config{config}, m_envIsUp{false}, m_selArpWaiting{0}, m_needUserSetup{false} {
		m_properties.name = m_config->name;
		m_properties.id = id;
		m_properties.active = false;
		m_properties.selectResults.fill({}, config->select.filters.size());

		for(auto const& ic: m_config->ipv4Interfaces) {
			m_ifs.push_back(new Interface_IPv4(this, m_ifs.size(), ic));
		}
		m_ifUpStatus.fill(false, m_ifs.size());
		updateNeedUserSetup();
	}
	Environment::~Environment() {
		foreach (Interface* iface, m_ifs)
			delete iface;
		m_ifs.clear();
	}

	void Environment::checkStatus() {
//		log << QString("checkStatus: %1/%2").arg(ifUpStatus.count(true)).arg(ifUpStatus.size()) << endl;
		if (m_properties.active) {
			if (!m_envIsUp && m_ifUpStatus.count(true) == m_ifUpStatus.size()) {
				m_envIsUp = true;
				m_device->envUp(this);
			}
		} else {
			if (m_envIsUp && m_ifUpStatus.count(true) == 0) {
				m_envIsUp = false;
				m_device->envDown(this);
			}
		}
	}

	void Environment::start() {
		if (m_properties.active) return;
		m_properties.active = true;
		m_envIsUp = false;
		m_needUserSetup = false;
		foreach (Interface* i, m_ifs) {
			i->start();
			if (i->needUserSetup()) {
				m_needUserSetup = true;
				m_device->envNeedUserSetup(this);
				break;
			}
		}
		emit propertiesChanged(m_properties);
		checkStatus();
	}
	void Environment::stop() {
		if (!m_properties.active) return;
		m_properties.active = false;
		m_envIsUp = true;
		foreach (Interface* i, m_ifs)
			i->stop();
		emit propertiesChanged(m_properties);
		checkStatus();
	}

	void Environment::ifUp(Interface* i) {
//		log << QString("Interface %1 up").arg(i->m_index) << endl;
		m_ifUpStatus[i->m_index] = true;
		checkStatus();
	}
	void Environment::ifDown(Interface* i) {
//		log << QString("Interface %1 down").arg(i->m_index) << endl;
		m_ifUpStatus[i->m_index] = false;
		checkStatus();
	}

	bool Environment::startSelect() {
		m_selArpWaiting = 0;

		ARPRequest *r;

		const QVector<SelectRule> &filters = m_config->select.filters;
		int c = filters.size();
		for (int i = 0; i < c; i++) {
			switch (filters[i].selType) {
			case SelectType::USER:
				m_properties.selectResults[i] = SelectResult::User;
				break;
			case SelectType::ARP:
				r = m_device->m_arp.requestIPv4(QHostAddress((quint32) 0), filters[i].ipAddr);
				if (!r) return false;
				r->disconnect(this);
				connect(r, &ARPRequest::timeout, this, &Environment::selectArpRequestTimeout);
				connect(r, &ARPRequest::foundMac, this, &Environment::selectArpRequestFoundMac);
				m_selArpWaiting++;
				break;
			case SelectType::ESSID:
				m_properties.selectResults[i] = (filters[i].essid == m_device->getEssid()) ? SelectResult::True : SelectResult::False;
				break;
			case SelectType::AND_BLOCK:
			case SelectType::OR_BLOCK:
				break;
			}
		}
		checkSelectState();
		return true;
	}

	void Environment::checkSelectState() {
		if (m_selArpWaiting > 0) return;
		if (m_selArpWaiting == -1) {
			err << QString("Environment::checkSelectState called after selection was done") << endl;
			return;
		}
		m_selArpWaiting = -1;
		const QVector<SelectRule> &filters = m_config->select.filters;
		const QVector< QVector<quint32> > &blocks = m_config->select.blocks;
		int c = filters.size();
		if (!c) {
//			qDebug() << QString("Nothing to select") << endl;
			m_properties.selectResult = SelectResult::User;
			m_device->selectDone(this);
			emit propertiesChanged(m_properties);
			return;
		}
		for (int i = c-1; i >= 0; i--) {
			if (filters[i].selType == SelectType::AND_BLOCK) {
				const QVector<quint32> &block = blocks[filters[i].block];
				auto tmp = SelectResult::True;
				foreach (quint32 j, block) {
					tmp = tmp && m_properties.selectResults[j];
					if (tmp == SelectResult::False) break;
				}
				m_properties.selectResults[i] = tmp;
			} else if (filters[i].selType == SelectType::OR_BLOCK) {
				const QVector<quint32> &block = blocks[filters[i].block];
				auto tmp = SelectResult::False;
				foreach (quint32 j, block) {
					tmp = tmp || m_properties.selectResults[j];
					if (tmp == SelectResult::True) break;
				}
				m_properties.selectResults[i] = tmp;
			}
		}
		m_properties.selectResult = m_properties.selectResults[0];
		m_device->selectDone(this);
		emit propertiesChanged(m_properties);
//		qDebug() << QString("Select Result: %1").arg((qint8) m_properties.selectResults[0]) << endl;
	}

	void Environment::selectArpRequestTimeout(QHostAddress ip) {
//		qDebug() << QString("arp timeout: %1").arg(ip.toString()) << endl;
		const QVector<SelectRule> &filters = m_config->select.filters;
		int c = filters.size();
		for (int i = 0; i < c; i++) {
			if (filters[i].selType == SelectType::ARP && filters[i].ipAddr == ip) {
				m_properties.selectResults[i] = SelectResult::False;
				m_selArpWaiting--;
			}
		}
		checkSelectState();
	}

	void Environment::selectArpRequestFoundMac(MacAddress mac, QHostAddress ip) {
//		qDebug() << QString("arp found: %1 -> %2").arg(ip.toString()).arg(mac.toString()) << endl;
		const QVector<SelectRule> &filters = m_config->select.filters;
		int c = filters.size();
		for (int i = 0; i < c; i++) {
			if (filters[i].selType == SelectType::ARP && filters[i].ipAddr == ip) {
				if ((!filters[i].macAddr.zero()) && (filters[i].macAddr != mac)) {
					m_properties.selectResults[i] = SelectResult::False;
				} else {
					m_properties.selectResults[i] = SelectResult::True;
				}
				m_selArpWaiting--;
			}
		}
		checkSelectState();
	}

	void Environment::updateNeedUserSetup() {
		m_needUserSetup = false;
		foreach (Interface *iface, m_ifs) {
			if (iface->needUserSetup()) {
				m_needUserSetup = true;
				break;
			}
		}
	}

	Interface::Interface(Environment* env, int index)
	: QObject(env), m_env(env), m_index(index) { }
	Interface::~Interface() { }

	Interface_IPv4::Interface_IPv4(Environment *env, int index, std::shared_ptr<IPv4Config> config)
	: Interface(env, index), m_dm(env->m_device->m_dm), m_config(config) {
		m_properties.state = InterfaceState::OFF;
		m_properties.needUserSetup = m_config->flags & IPv4ConfigFlag::USERSTATIC;
		m_properties.gatewayMetric = (-1 != m_config->gatewayMetric) ? m_config->gatewayMetric : m_env->getDevice()->getConfig().gatewayMetric;
		m_last_notified_properties = m_properties;
	}

	Interface_IPv4::~Interface_IPv4() {
		m_dhcp_timer.kill(this);
		releaseXID();
	}

	void Interface_IPv4::checkPropertiesUpdate() {
		if (m_last_notified_properties != m_properties) {
			m_last_notified_properties = m_properties;
			emit propertiesChanged(m_properties);
		}
	}

	void Interface_IPv4::setState(InterfaceState state) {
		if (m_properties.state == state) {
			checkPropertiesUpdate();
			return;
		}
		m_properties.state = state;
		m_last_notified_properties = m_properties;
		m_dm->m_events.interfaceStatusChanged(state, this);
		if (InterfaceState::OFF == state || InterfaceState::WAITFORCONFIG == state) {
			/* kept for for m_dm->m_events, now clear it */
			m_properties.ip.clear();
			m_properties.netmask.clear();
			m_properties.gateway.clear();
			m_properties.dnsServers.clear();
		}
		emit propertiesChanged(m_properties);
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
		DHCPClientPacket cp(this, m_dhcp_server_ip);
		cp.doDHCPRenew(m_properties.ip);
		cp.check();
		cp.send();
	}

	void Interface_IPv4::dhcp_send_rebind() {
		DHCPClientPacket cp(this);
		cp.doDHCPRebind(m_properties.ip);
		cp.check();
		cp.send();
	}

	void Interface_IPv4::dhcp_send_release() {
		DHCPClientPacket cp(this, m_dhcp_server_ip);
		cp.doDHCPRelease(m_properties.ip, m_dhcp_server_identifier);
		cp.check();
		cp.send();
	}

	void Interface_IPv4::dhcp_setup_interface(DHCPPacket *ack, bool renewing) {
		//stop fallback
		m_fallback_timer.kill(this);
		if (renewing) {
			m_dhcp_lease_time = ntohl(ack->getOptionData<quint32>(DHCP_LEASE_TIME, 0xffffffffu));
		} else {
			m_properties.ip = ack->getYourIP();
			m_properties.netmask = ack->getOptionAddress(DHCP_SUBNET);
			m_properties.gateway = ack->getOptionAddress(DHCP_ROUTER);
			m_properties.dnsServers = ack->getOptionAddresses(DHCP_DNS_SERVER);
			m_localdomain = ack->getOptionString(DHCP_DOMAIN_NAME);
			m_dhcp_server_identifier = ack->getOption(DHCP_SERVER_ID);
			m_dhcp_server_ip = QHostAddress(ntohl(ack->headers.ip.saddr));
			m_dhcp_lease_time = ntohl(ack->getOptionData<quint32>(DHCP_LEASE_TIME, 0xffffffffu));
				// T1: 0.5 * dhcp_lease_time
				// T2: 0.875 * dhcp_lease_time ( 7/8 )
			systemUp(InterfaceState::DHCP);
		}
	}

	void Interface_IPv4::zeroconf_setup_interface() {
		if (m_properties.state != InterfaceState::OFF) return;
		m_properties.ip = m_zc_probe_ip;
		m_properties.netmask = QHostAddress((quint32) 0xFFFF0000);
		m_properties.gateway = QHostAddress((quint32) 0);
		m_properties.dnsServers.clear();
		systemUp(InterfaceState::ZEROCONF);
	}

	quint16 hashMac(const MacAddress &addr) {
		auto w0 = qFromBigEndian<quint16>(addr.data.bytes);
		auto w1 = qFromBigEndian<quint16>(addr.data.bytes + 2);
		auto w2 = qFromBigEndian<quint16>(addr.data.bytes + 4);
		return w0 ^ w1 ^ w2;
	}

	void Interface_IPv4::zeroconfProbe() {   // select new ip and start probe it
		quint32 lastip = m_zc_probe_ip.toIPv4Address();
		const quint32 baseip = 0xA9FE0000; // 169.254.0.0
		const quint32 mask = 0xFFFF;
		quint32 ip = baseip | hashMac(m_env->m_device->getMacAddress());
		while (ip == lastip) {
			quint32 rnd;
			do {
				rnd = getRandomUInt32() & mask;
			} while ((rnd >> 8) == 0 || (rnd >> 8) == 0xFF);
			ip = baseip | rnd;
		}
		m_zc_probe_ip = QHostAddress(ip);
		m_zc_arp_probe = m_env->m_device->m_arp.probeIPv4(m_zc_probe_ip);
		if (!m_zc_arp_probe || m_zc_arp_probe->getState() != ARPProbe::PROBING) {
			err << "ARPProbe failed" << endl;
			m_zc_state =ZCS_OFF;
			return;
		}
		// This did lead to segfaults... ;_)
		// m_zc_arp_probe->setReserve(m_config->flags & IPv4ConfigFlag::DHCP);
		connect(m_zc_arp_probe, &ARPProbe::conflict, this, &Interface_IPv4::zc_probe_conflict);
		connect(m_zc_arp_probe, &ARPProbe::ready, this, &Interface_IPv4::zc_probe_ready);
	}
	void Interface_IPv4::zeroconfWatch() {
		m_zc_arp_watch = m_env->m_device->m_arp.watchIPv4(m_zc_probe_ip);
		connect(m_zc_arp_watch, &ARPWatch::conflict, this, &Interface_IPv4::zc_watch_conflict);
	}
	void Interface_IPv4::zeroconfAnnounce() {
		m_zc_arp_announce = m_env->m_device->m_arp.announceIPv4(m_zc_probe_ip);
		connect(m_zc_arp_announce, &ARPAnnounce::ready, this, &Interface_IPv4::zc_announce_ready);
	}

	void Interface_IPv4::zeroconfFree() {    // free ARPProbe and ARPWatch
		if (m_zc_arp_probe) {
			delete m_zc_arp_probe;
			m_zc_arp_probe = 0;
		}
		if (m_zc_arp_announce) {
			delete m_zc_arp_announce;
			m_zc_arp_announce = 0;
		}
		if (m_zc_arp_watch) {
			delete m_zc_arp_watch;
			m_zc_arp_watch = 0;
		}
	}

	void Interface_IPv4::zeroconfAction() {
		if (!(m_config->flags & IPv4ConfigFlag::ZEROCONF))
			return;
		bool hasDHCP = (m_config->flags & IPv4ConfigFlag::DHCP);
		if (m_properties.state == InterfaceState::DHCP)
			m_zc_state = ZCS_OFF;
		for (;;) {
			switch (m_zc_state) {
			case ZCS_OFF:
				zeroconfFree();
				return;
			case ZCS_START:
				m_zc_probe_ip = QHostAddress((quint32) 0);
				m_zc_state = ZCS_PROBE;
			case ZCS_PROBE:
				zeroconfProbe();
				m_zc_state = ZCS_PROBING;
			case ZCS_PROBING:
				return;
			case ZCS_RESERVING:
				if (!hasDHCP || m_dhcp_retry > 3) {
					if (m_zc_arp_probe != 0){
						delete m_zc_arp_probe;
						m_zc_arp_probe = 0;
					}
					m_zc_state = ZCS_ANNOUNCE;
				} else {
					return;
				}
			case ZCS_ANNOUNCE:
				zeroconfWatch();
				zeroconfAnnounce();
				m_zc_state = ZCS_ANNOUNCING;
			case ZCS_ANNOUNCING:
				return;
			case ZCS_BIND:
				zeroconf_setup_interface();
				m_zc_state = ZCS_BOUND;
			case ZCS_BOUND:
				return;
			case ZCS_CONFLICT:
				zeroconfFree();
				m_zc_state = ZCS_PROBE;
				break;
			}
		}
	}

	void Interface_IPv4::zc_probe_conflict() {
		m_zc_arp_probe = 0; // Deletes itself
		m_zc_state = ZCS_CONFLICT;
		zeroconfAction();
	}
	void Interface_IPv4::zc_probe_ready() {
		if (!m_zc_arp_probe->getReserve())
			m_zc_arp_probe = 0; // Deletes itself
		m_zc_state = ZCS_RESERVING;
		zeroconfAction();
	}
	void Interface_IPv4::zc_announce_ready() {
		m_zc_arp_announce = 0; // Deletes itself
		m_zc_state = ZCS_BIND;
		zeroconfAction();
	}
	void Interface_IPv4::zc_watch_conflict() {
		m_zc_arp_watch = 0; // Deletes itself
		if (m_properties.state == InterfaceState::ZEROCONF) systemDown(InterfaceState::OFF);
		m_zc_state = ZCS_CONFLICT;
		zeroconfAction();
	}

	void Interface_IPv4::dhcpAction(DHCPPacket *source) {
		for (;;) {
			switch (m_dhcpstate) {
			case DHCPS_OFF:
				releaseXID();
				return;
			case DHCPS_INIT_START:
				m_dhcp_retry = 0;
				// fall through:
				// dhcpstate = DHCPS_INIT;
			case DHCPS_INIT:
				checkFallbackRunning();
				dhcp_send_discover();
				m_dhcpstate = DHCPS_SELECTING;
				break;
			case DHCPS_SELECTING:
				if (source) {
					if (source->getMessageType() == DHCP_OFFER)  {
						m_dhcp_timer.kill(this);
						dhcp_send_request(source);
						m_dhcpstate = DHCPS_REQUESTING;
					}
				} else {
					if (m_dhcp_retry < 5) {
						m_dhcp_retry++;
						// send 6 packets with short interval
						// 0, 500, 2500, 4500, 6500, 8500 [ms]
						m_dhcp_timer.set_timeout(this, 500 + (m_dhcp_retry-1) * 2000);
					} else {
						// 1 min
						m_dhcp_timer.set_timeout(this, 1 * 60 * 1000);
					}
					return;
				}
			case DHCPS_REQUESTING:
				if (source) {
					switch (source->getMessageType()) {
					case DHCP_ACK:
						m_dhcp_timer.kill(this);
						dhcp_setup_interface(source);
						m_dhcpstate = DHCPS_BOUND;
						break;
					case DHCP_NAK:
						m_dhcp_timer.kill(this);
						m_dhcpstate = DHCPS_INIT;
						break;
					default:
						break;
					}
					break;
				} else {
					m_dhcp_timer.set_timeout(this, 4000);
					return;
				}
			case DHCPS_BOUND:
				releaseXID();
				// 0.5 * 1000 (msecs)
				m_dhcp_timer.set_timeout(this, 500 * m_dhcp_lease_time);
				return;
			case DHCPS_RENEWING:
				if (source) {
					switch (source->getMessageType()) {
					case DHCP_ACK:
						m_dhcp_timer.kill(this);
						dhcp_setup_interface(source, true);
						m_dhcpstate = DHCPS_BOUND;
						break;
					case DHCP_NAK:
						m_dhcp_timer.kill(this);
						m_dhcpstate = DHCPS_INIT_START;
						break;
					default:
						break;
					}
					break;
				} else {
					dhcp_send_renew();
					// T2 - T1 * 1000 (msecs) = (7 / 8 - 1 / 2) * 1000 = 3 / 8 * 1000 = 375
					m_dhcp_timer.set_timeout(this, 375 * m_dhcp_lease_time);
					return;
				}
			case DHCPS_REBINDING:
				if (source) {
					switch (source->getMessageType()) {
					case DHCP_ACK:
						m_dhcp_timer.kill(this);
						dhcp_setup_interface(source, true);
						m_dhcpstate = DHCPS_BOUND;
						break;
					case DHCP_NAK:
						m_dhcp_timer.kill(this);
						m_dhcpstate = DHCPS_INIT_START;
						break;
					default:
						break;
					}
					break;
				} else {
					dhcp_send_rebind();
					// T - (T2) = 1/8 -> 125
					m_dhcp_timer.set_timeout(this, 125 * m_dhcp_lease_time);
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
		if (m_fallback_timer.match(tevt)) {
			startFallback();
		}
		else if (m_dhcp_timer.match(tevt)) {
			m_dhcp_timer.kill(this);
			switch (m_dhcpstate) {
			case DHCPS_SELECTING:
				m_dhcpstate = DHCPS_INIT;
				break;
			case DHCPS_REQUESTING:
				m_dhcpstate = DHCPS_INIT;
				break;
			case DHCPS_BOUND:
				m_dhcpstate = DHCPS_RENEWING;
				break;
			case DHCPS_RENEWING:
				m_dhcpstate = DHCPS_REBINDING;
				break;
			case DHCPS_REBINDING:
				m_dhcpstate = DHCPS_INIT_START;
				break;
			default:
				break;
			}
			dhcpAction();
		}
		else {
			err << "Unrequested timer Event: " << tevt->timerId() << endl;
		}
	}

	void Interface_IPv4::startDHCP() {
		//Start timer for fallback timeout:
		checkFallbackRunning();
		m_dhcpstate = DHCPS_INIT_START;
		dhcpAction();
	}
	void Interface_IPv4::stopDHCP() {
		m_fallback_timer.kill(this);
		switch (m_dhcpstate) {
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
		m_dhcp_timer.kill(this);
	}

	void Interface_IPv4::startZeroconf() {
		m_zc_state = ZCS_START;
		zeroconfAction();
	}
	void Interface_IPv4::startStatic() {
		m_properties.ip = m_config->static_ip;
		m_properties.netmask = m_config->static_netmask;
		m_properties.gateway = m_config->static_gateway;
		m_properties.dnsServers = m_config->static_dnsservers;
		systemUp(InterfaceState::STATIC);
	}
	void Interface_IPv4::startUserStatic() {
		m_properties.ip = m_userConfig.ip;
		m_properties.netmask = m_userConfig.netmask;
		m_properties.gateway = m_userConfig.gateway;
		m_properties.dnsServers = m_userConfig.dnsServers;
		systemUp(InterfaceState::STATIC);
	}

	void Interface_IPv4::startFallback() {
		auto is_up = m_env->m_ifUpStatus.at(m_index);
		/* we should never get here if the interface is up or DHCP was successful; check anyway. */
		if (InterfaceState::DHCP != m_properties.state && !is_up) {
			if (m_config->flags == (IPv4ConfigFlag::DHCP | IPv4ConfigFlag::ZEROCONF)) {
				stopDHCP();
				startZeroconf();
			}
			else if (m_config->flags == (IPv4ConfigFlag::DHCP | IPv4ConfigFlag::STATIC)) {
				stopDHCP();
				startStatic();
			}
		}
	}


	void Interface_IPv4::checkFallbackRunning() {
		if ((!m_fallback_timer.active() && m_config->timeout > 0) && (
			( m_config->flags == (IPv4ConfigFlag::DHCP | IPv4ConfigFlag::ZEROCONF)) ||
			( m_config->flags == (IPv4ConfigFlag::DHCP | IPv4ConfigFlag::STATIC)) ) ) {
			m_fallback_timer.set_timeout(this, m_config->timeout*1000);
		}
	}

	void Interface_IPv4::start() {
		log << "Interface_IPv4::start" << endl;
		if (m_config->flags & IPv4ConfigFlag::DHCP) {
			startDHCP();
		} else if (m_config->flags & IPv4ConfigFlag::USERSTATIC) {
			//Check if user config is set otherwise let user set:
			// both setState() and startUserState() end in checkPropertiesUpdate().
			m_properties.needUserSetup = !m_userConfig.valid();
			m_env->updateNeedUserSetup();
			if (m_properties.needUserSetup) {
				setState(InterfaceState::WAITFORCONFIG);
			} else {
				startUserStatic();
			}
		} else if (m_config->flags & IPv4ConfigFlag::ZEROCONF) {
			startZeroconf();
		} else {
			startStatic();
		}
	}

	void Interface_IPv4::stop() {
		log << "Interface_IPv4::stop" << endl;
		if (m_dhcpstate != DHCPS_OFF)
			stopDHCP();
		if (m_zc_state != ZCS_OFF) {
			m_zc_state = ZCS_OFF;
			zeroconfAction();
		}
		systemDown(InterfaceState::OFF);
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

	void Interface_IPv4::systemUp(InterfaceState new_state) {
		auto local = getNLAddr(m_properties.ip, m_properties.netmask);
		auto bcast = getNLBroadcast(m_properties.ip, m_properties.netmask);
//		char buf[32];
//		log << nl_addr2str (local, buf, 32) << endl;
		struct rtnl_addr *addr = rtnl_addr_alloc();
		rtnl_addr_set_ifindex(addr, m_env->m_device->m_interfaceIndex);
		rtnl_addr_set_family(addr, AF_INET);
		rtnl_addr_set_local(addr, local);
		rtnl_addr_set_broadcast(addr, bcast);
		if (m_properties.state == InterfaceState::ZEROCONF) {
			rtnl_addr_set_scope(addr, RT_SCOPE_LINK);
			rtnl_addr_set_flags(addr, IFA_F_PERMANENT);
		}
#if 0
		log << "systemUp: addr_add = " << rtnl_addr_add(dm->hwman.getNLHandle(), addr, 0) << endl;
#else
		rtnl_addr_add(m_dm->m_hwman.getNLHandle(), addr, 0);
#endif
		rtnl_addr_put(addr);
		nl_addr_put(local);
		nl_addr_put(bcast);
		// Gateway
		if (!m_properties.gateway.isNull()) {
//			log << "Try setting gateway" << endl;
			struct rtentry rt;
			memset(&rt, 0, sizeof(rt));
			rt.rt_flags = RTF_UP | RTF_GATEWAY;
			setSockaddrIPv4(rt.rt_dst);
			setSockaddrIPv4(rt.rt_gateway, m_properties.gateway.toIPv4Address());
			setSockaddrIPv4(rt.rt_genmask);
			if (-1 != m_properties.gatewayMetric) {
				rt.rt_metric = m_properties.gatewayMetric + 1;
			}

			QByteArray buf = m_env->m_device->getName().toUtf8();
			rt.rt_dev = buf.data();
			int skfd = socket(AF_INET, SOCK_DGRAM, 0);
			write(3, &rt, sizeof(rt));
			ioctl(skfd, SIOCADDRT, &rt);
			close(skfd);
		}
		// Resolvconf
		if (!m_properties.dnsServers.empty()) {
			QProcess *proc = new QProcess();
			QStringList arguments;
			arguments << "-a" << QString("%1_%2").arg(m_env->m_device->getName()).arg(m_index);
			proc->start("/sbin/resolvconf", arguments);
			QTextStream ts(proc);
			if (!m_localdomain.isEmpty())
				ts << "domain " << m_localdomain << endl;
			foreach(QHostAddress ha, m_properties.dnsServers) {
				ts << "nameserver " << ha.toString() << endl;
			}
			proc->closeWriteChannel();
			proc->waitForFinished(-1);
			delete proc; // waits for process
		}
		setState(new_state);
		m_env->ifUp(this);
	}
	void Interface_IPv4::systemDown(InterfaceState new_state) {
		// Resolvconf
		QProcess *proc = new QProcess();
		QStringList arguments;
		arguments << "-d" << QString("%1_%2").arg(m_env->m_device->getName()).arg(m_index);
		proc->start("/sbin/resolvconf", arguments);
		proc->closeWriteChannel();
		proc->waitForFinished(-1);
		delete proc; // waits for process
		// Gateway
		if (!m_properties.gateway.isNull()) {
			struct rtentry rt;
			memset(&rt, 0, sizeof(rt));
			rt.rt_flags = RTF_UP | RTF_GATEWAY;
			setSockaddrIPv4(rt.rt_dst);
			setSockaddrIPv4(rt.rt_gateway, m_properties.gateway.toIPv4Address());
			setSockaddrIPv4(rt.rt_genmask);
			QByteArray buf = m_env->m_device->getName().toUtf8();
			rt.rt_dev = buf.data();
			int skfd = socket(AF_INET, SOCK_DGRAM, 0);
			ioctl(skfd, SIOCDELRT, &rt);
			close(skfd);
		}
		auto local = getNLAddr(m_properties.ip, m_properties.netmask);
//		char buf[32];
//		log << nl_addr2str (local, buf, 32) << endl;
		struct rtnl_addr *addr = rtnl_addr_alloc();
		rtnl_addr_set_ifindex(addr, m_env->m_device->m_interfaceIndex);
		rtnl_addr_set_family(addr, AF_INET);
		rtnl_addr_set_local(addr, local);
#if 0
		log << "systemDown: addr_delete = " << rtnl_addr_delete(dm->hwman.getNLHandle(), addr, 0) << endl;
#else
		rtnl_addr_delete(m_dm->m_hwman.getNLHandle(), addr, 0);
#endif
		rtnl_addr_put(addr);
		nl_addr_put(local);
		setState(new_state);
		m_env->ifDown(this);
	}

	bool Interface_IPv4::registerXID(quint32 xid) {
		releaseXID();
		if (m_env->m_device->registerXID(xid, this)) {
			m_dhcp_xid_unicast = false;
			m_dhcp_xid = xid;
			return true;
		}
		return false;
	}

	bool Interface_IPv4::registerUnicastXID(quint32 &xid) {
		releaseXID();
		xid = 0;
		if (!setupUnicastDHCP()) return false;
		xid = getRandomUInt32();
		if (!xid) xid++;
		m_dhcp_xid_unicast = true;
		m_dhcp_xid = xid;
		return true;
	}

	void Interface_IPv4::releaseXID() {
		if (m_dhcp_xid) {
			if (m_dhcp_xid_unicast) {
				closeUnicastDHCP();
			} else{
				m_env->m_device->unregisterXID(m_dhcp_xid);
			}
			m_dhcp_xid = 0;
		}
	}

	void Interface_IPv4::dhcpReceived(DHCPPacket *packet) {
		dhcpAction(packet);
	}

	bool Interface_IPv4::setupUnicastDHCP(bool temporary) {
		m_dhcp_unicast_socket = socket(PF_INET, SOCK_DGRAM, 0);
		if (m_dhcp_unicast_socket == -1) {
			err << "Couldn't create UDP socket" << endl;
			return false;
		}
		{
			int opt = 1;
			setsockopt(m_dhcp_unicast_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		}
		struct sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_port = htons(68);
		sin.sin_addr.s_addr = htonl(m_properties.ip.toIPv4Address());
		if (-1 == bind(m_dhcp_unicast_socket, (struct sockaddr*) &sin, sizeof(sin))) {
			err << "Couldn't bind on local dhcp client socket" << endl;
			close(m_dhcp_unicast_socket);
			m_dhcp_unicast_socket = -1;
			return false;
		}
		m_dhcp_unicast_read_nf = 0;
		if (!temporary) {
			m_dhcp_unicast_read_nf = new QSocketNotifier(m_dhcp_unicast_socket, QSocketNotifier::Read);
			connect(m_dhcp_unicast_read_nf, &QSocketNotifier::activated, this, &Interface_IPv4::readDHCPUnicastClientSocket);
		}
		return true;
	}

	void Interface_IPv4::closeUnicastDHCP() {
		if (m_dhcp_unicast_read_nf) {
			delete m_dhcp_unicast_read_nf;
			m_dhcp_unicast_read_nf = 0;
		}
		close(m_dhcp_unicast_socket);
		m_dhcp_unicast_socket = -1;
	}

	bool Interface_IPv4::sendUnicastDHCP(DHCPPacket *packet) {
		bool tmpSocket = !m_dhcp_xid || !m_dhcp_xid_unicast;
		if (tmpSocket && !setupUnicastDHCP(true)) return false;
		struct sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_port = htons(67);
		sin.sin_addr.s_addr = htonl(packet->unicast_addr.toIPv4Address());
		sendto(m_dhcp_unicast_socket, packet->msgdata.data() + sizeof(packet->headers), packet->msgdata.length() - sizeof(packet->headers), 0, (struct sockaddr*) &sin, sizeof(sin));
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
			nread = recvfrom(m_dhcp_unicast_socket, buf.data(), msgsize, MSG_PEEK, (struct sockaddr *)&sin, &slen);
			if (nread < 0) {
//				perror("Device::readDHCPClientSocket: recvfrom");
				closeUnicastDHCP();
			}
			if (nread == 0) return;
		} while (nread == msgsize);
		buf.resize(nread);
		recvfrom(m_dhcp_unicast_socket, buf.data(), nread, 0, (struct sockaddr *)&sin, &slen);
		DHCPPacket *packet = DHCPPacket::parseData(buf, sin);
		if (!packet) return;
		dhcpAction(packet);
		delete packet;
	}

	bool Interface_IPv4::setUserConfig(const IPv4UserConfig &userConfig) {
		if (!(m_config->flags & IPv4ConfigFlag::USERSTATIC)) return false;
		m_userConfig = userConfig;
		bool tmp = m_properties.needUserSetup;
		m_properties.needUserSetup = !m_userConfig.valid();
		m_env->updateNeedUserSetup();
		if (m_properties.needUserSetup) {
			if (tmp) return true;
			systemDown(InterfaceState::WAITFORCONFIG);
		} else {
			//check if environment is active:
			if (m_env->getID() == m_env->m_device->getEnvironment()) {
				startUserStatic();
			} else {
				// systemDown / startUserStatic do this on their own
				checkPropertiesUpdate();
			}
		}
		return true;
	}
}
