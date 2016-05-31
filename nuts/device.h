#ifndef _NUTS_DEVICE_H
#define _NUTS_DEVICE_H

#pragma once

#include <QObject>
#include <QBasicTimer>
#include <QString>
#include <QList>
#include <QHostAddress>
#include <QBitArray>
#include <QSocketNotifier>
#include <QLinkedList>
#include <QTimer>
#include <QProcess>

#include "libnutcommon/common.h"

namespace nuts {
	class ProcessManager;

	class DBusDeviceManager;

	class DeviceManager;
	class Device;
	class Environment;
	class Interface;
	class Interface_IPv4;

	class ARP;
	class DHCPPacket;
	class DHCPClientPacket;
}

#include "hardware.h"
#include "arp.h"
#include "events.h"

namespace nuts {
	/** @brief The DeviceManager keeps track of all hardware devices.

		On creation, the DeviceManager reads the config file and then tries to
		find every device specified in it.

		If devices are created later (or are removed) by the kernel, it emits
		signals if they are managed by the DeviceManager, i.e. are in the config file.
	 */
	class DeviceManager final : public QObject {
		Q_OBJECT
	public:
		/**
		 * @brief Constructs the DeviceManager
		 * @param configFile Path to the configfile.
		 */
		explicit DeviceManager(const QString &configFile, ProcessManager* processManager);
		~DeviceManager();

		void shutdown();

		/**
		 * @brief Gets all managed devices.
		 * @return Hash of devices; the keys in the hash represent the device name in the kernel.
		 */
		const QHash<QString, Device*>& getDevices() { return m_devices; }
		/**
		 * @brief Get the config.
		 * @return Config
		 */
		const libnutcommon::Config& getConfig() { return m_config; }

	signals:
		/**
		 * Emmitted when a device is added after construction.
		 * @param devName Device name in kernel
		 * @param dev Pointer to Device
		 */
		void deviceAdded(QString devName, Device *dev);
		/**
		 * Emmited when a device is removed.
		 * @param devName Device name in kernel
		 * @param dev Pointer to Device
		 */
		void deviceRemoved(QString devName, Device *dev);

	private slots:
		void ca_timer();
		void gotCarrier(const QString &ifName, int ifIndex, const QString &essid);
		void lostCarrier(const QString &ifName);
		void newDevice(const QString &ifName, int ifIndex);
		void delDevice(const QString &ifname);

	private:
		Events m_events;
		libnutcommon::Config m_config;
		QTimer m_carrier_timer;
		/// Internal structure for delaying carrier events.
		struct ca_evt {
			QString ifName;
			int ifIndex;
			bool up;
		};
		QLinkedList<struct ca_evt> m_ca_evts;

		QHash<QString, Device*> m_devices;

		void addDevice(const QString &ifname, std::shared_ptr<libnutcommon::DeviceConfig> dc);

		friend class Device;
		friend class Interface_IPv4;

		HardwareManager m_hwman;
	};

	/** @brief A Device manages the state of a hardware device from the kernel.
		@author Stefan Bühler <stbuehler@web.de>

		If a Device gets activated (either by configuration default or by user),
		it waits for a "carrier", i.e. WLAN selection (by wpa_supplicant) or a cable plugged in.

		It then tries to find the right environment, based on the common::SelectConfig; only one environment
		can be activated.
	 */
	class Device : public QObject {
		Q_OBJECT
	private:
		explicit Device(DeviceManager* dm, const QString &name, std::shared_ptr<libnutcommon::DeviceConfig> config, bool hasWLAN);
		~Device();

	public slots:
		/* Properties (activeEnvironment not used here) */
		const libnutcommon::DeviceProperties& getProperties() const { return m_properties; }
		QString getName() const { return m_properties.name; } //!< Name of the device in the kernel, e.g. "eth0"
		libnutcommon::DeviceType getType() const { return m_properties.type; } //!< Name of the device in the kernel, e.g. "eth0"
		int getEnvironment() { return m_activeEnv; } //!< Active environment, or -1
		libnutcommon::DeviceState getState() const { return m_properties.state; }
		QString getEssid() { return m_properties.essid; }
		libnutcommon::MacAddress getMacAddress() { return m_properties.macAddress; }

		bool isAir() const { return libnutcommon::DeviceType::AIR == m_properties.type; }

		const libnutcommon::DeviceConfig& getConfig() const { return *m_config; }

		const QList<Environment*>& getEnvironments() { return m_envs; }

		int getUserPreferredEnvironment() { return m_userEnv; }
		void setUserPreferredEnvironment(int env);

		/**
		 * Enable device.
		 * @param force Force the use of a kernel interface, even if it is already up and perhaps in use.
		 *	nuts will takeover the interface.
		 * @return
		 */
		bool enable(bool force = false);
		void disable(); //!< Disable device

	signals:
		/* activeEnvironment not maintained in properties, separate signal below */
		void propertiesChanged(libnutcommon::DeviceProperties properties);
		void activeEnvironmentChanged(int environment);

	private:
		void setState(libnutcommon::DeviceState state);

		// only false on failed startup, not on "unused"
		bool startWPASupplicant();
		void stopWPASupplicant();

		// Called from Environment
		void envUp(Environment*);
		void envDown(Environment*);
		void envNeedUserSetup(Environment*);
		void selectDone(Environment*);

		void setEnvironment(int env); //!< Select specific environment
		void checkEnvironment();
		void startEnvSelect();

		void gotCarrier(int ifIndex, const QString &essid = "");
		void lostCarrier();

		// DHCP
		bool registerXID(quint32 xid, Interface_IPv4 *iface);
		void unregisterXID(quint32 xid);
		bool sendDHCPClientPacket(DHCPPacket *packet);
		bool setupDHCPClientSocket();
		void closeDHCPClientSocket();

	private slots:
		void readDHCPClientSocket();
		void writeDHCPClientSocket();

	private: /* vars */
		friend class DeviceManager;
		friend class Environment;
		friend class Interface_IPv4;
		friend class DHCPPacket;
		friend class DHCPClientPacket;
		friend class ARP;

		ARP m_arp;
		DeviceManager *m_dm = nullptr;
		int m_interfaceIndex = -1;
		std::shared_ptr<libnutcommon::DeviceConfig> m_config;

		int m_activeEnv = -1;
		int m_nextEnv = -1;
		int m_userEnv = -1;
		int m_waitForEnvSelects = 0;

		QList<Environment*> m_envs;

		// DHCP
		QHash< quint32, Interface_IPv4* > m_dhcp_xid_iface;
		int m_dhcp_client_socket = -1;
		QSocketNotifier *m_dhcp_read_nf = nullptr;
		QSocketNotifier *m_dhcp_write_nf = nullptr;
		QLinkedList< QByteArray > m_dhcp_write_buf;

		// Device properties
		libnutcommon::DeviceProperties m_properties;

		// WPA
		QProcess *m_wpa_supplicant = nullptr;
	};

	class Environment : public QObject {
		Q_OBJECT
		Q_PROPERTY(Device* device READ getDevice)
	public:
		explicit Environment(Device *device, std::shared_ptr<libnutcommon::EnvironmentConfig> config, int id);
		~Environment();

		Device* getDevice() { return m_device; }

		libnutcommon::EnvironmentProperties const& getProperties() const { return m_properties; }
		QString getName() const { return m_properties.name; }
		int getID() const { return m_properties.id; }
		bool isActive() const { return m_properties.active; }
		libnutcommon::SelectResult getSelectResult() const { return m_properties.selectResult; }
		QVector<libnutcommon::SelectResult> const& getSelectResults() const { return m_properties.selectResults; }

		bool selectionDone() const { return (m_selArpWaiting == -1); }

		libnutcommon::EnvironmentConfig const& getConfig() const { return *m_config; }

		QList<Interface*> const& getInterfaces() const { return m_ifs; }

	signals:
		/* select result changes might get delayed until all are ready */
		void propertiesChanged(libnutcommon::EnvironmentProperties properties);

	private:
		void checkStatus();

		void start();
		void stop();

		void ifUp(Interface*);
		void ifDown(Interface*);

		bool startSelect();
		void checkSelectState();

		void updateNeedUserSetup();

	private slots:
		void selectArpRequestTimeout(QHostAddress ip);
		void selectArpRequestFoundMac(libnutcommon::MacAddress mac, QHostAddress ip);

	private: /* vars */
		friend class Device;
		friend class Interface;
		friend class Interface_IPv4;
		friend class DHCPPacket;
		friend class DHCPClientPacket;

		Device* m_device = nullptr;
		QList<Interface*> m_ifs;

		libnutcommon::EnvironmentProperties m_properties;

		std::shared_ptr<libnutcommon::EnvironmentConfig> m_config;
		QBitArray m_ifUpStatus;
		bool m_envIsUp = false;

		int m_selArpWaiting = 0;
		bool m_needUserSetup = false;
	};

	class Interface : public QObject {
		Q_OBJECT
	public:
		explicit Interface(Environment *env, int index);
		~Interface();

		Environment* getEnvironment() { return m_env; }

		int getIndex() const { return m_index; }
		
		virtual bool needUserSetup() const = 0;

		virtual void start() = 0;
		virtual void stop() = 0;

	protected: /* vars */
		friend class Environment;

		Environment* const m_env = nullptr;
		int const m_index = -1;
	};

	class Interface_IPv4 : public Interface {
		Q_OBJECT
	public:
		explicit Interface_IPv4(Environment *env, int index, std::shared_ptr<libnutcommon::IPv4Config> config);
		~Interface_IPv4();

		void start() override;
		void stop() override;

		libnutcommon::InterfaceProperties const& getProperties() const { return m_properties; }
		libnutcommon::InterfaceState getState() const { return m_properties.state; }
		QHostAddress const& getIP() const { return m_properties.ip; }
		QHostAddress const& getNetmask() const { return m_properties.netmask; }
		QHostAddress const& getGateway() const { return m_properties.gateway; }
		QList<QHostAddress> const& getDnsServers() const { return m_properties.dnsServers; }
		int gatewayMetric() const { return m_properties.gatewayMetric; }
		bool needUserSetup() const override { return m_properties.needUserSetup; }

		QString getLocalDomain() const { return m_localdomain; }

		libnutcommon::IPv4Config const& getConfig() const { return *m_config; }

		bool setUserConfig(const libnutcommon::IPv4UserConfig &userConfig);
		libnutcommon::IPv4UserConfig const& getUserConfig() const { return m_userConfig; }

	signals:
		void propertiesChanged(libnutcommon::InterfaceProperties properties);
		void userConfigChanged(libnutcommon::IPv4UserConfig userConfig);

	private:
		void timerEvent(QTimerEvent *event) override;

		void dhcp_send_discover();
		void dhcp_send_request(DHCPPacket *offer);
		void dhcp_send_renew();
		void dhcp_send_rebind();
		void dhcp_send_release();
		void dhcp_setup_interface(DHCPPacket *ack, bool renewing = false);
		void dhcpAction(DHCPPacket *source = 0);

		void zeroconf_setup_interface();
		void zeroconfProbe();   // select new ip and start probe it
		void zeroconfFree();    // free ARPProbe, ARPAnnounce and ARPWatch
		void zeroconfWatch();   // setup ARPWatch
		void zeroconfAnnounce();// setup ARPAnnounce
		void zeroconfAction();  // automaton

		void startDHCP();
		void stopDHCP();
		void startZeroconf();
		void startStatic();
		void startUserStatic();

		void startFallback();
		void checkFallbackRunning();

		// both call setState() and therefore checkPropertiesUpdate()
		void systemUp(libnutcommon::InterfaceState new_state);
		void systemDown(libnutcommon::InterfaceState new_state);

		bool setupUnicastDHCP(bool temporary = false);
		void closeUnicastDHCP();
		bool sendUnicastDHCP(DHCPPacket *packet);
		bool registerXID(quint32 xid);
		bool registerUnicastXID(quint32 &xid);
		void releaseXID();
		void dhcpReceived(DHCPPacket *packet);

		// make sure to call checkPropertiesUpdate() after each modification
		// to m_properties
		void checkPropertiesUpdate();
		// setState() also calls checkPropertiesUpdate() (or some inlined shortcut)
		void setState(libnutcommon::InterfaceState state);

	private slots:
		void readDHCPUnicastClientSocket();

		void zc_probe_conflict();
		void zc_probe_ready();
		void zc_announce_ready();
		void zc_watch_conflict();

	private: /* vars */
		friend class DHCPPacket;
		friend class DHCPClientPacket;
		friend class Environment;
		friend class Device;

		QBasicTimer m_dhcp_timer;
		int m_dhcp_retry = -1; // count retries
		QBasicTimer m_fallback_timer;

		enum class dhcp_state {
			OFF,
			INIT_START,  // reset dhcp_retry, -> dhcp_state::INIT
			INIT,        // (++dhcp_retry >= 5) -> failed, Discover all -> selecting
			SELECTING,   // Waiting for offer; request a offer -> requesting, timeout -> init
			REQUESTING,  // Requested a offer, waiting for ack -> bound, nak -> init
			BOUND,       // bound, on timeout request -> renew
			RENEWING,    // wait for ack -> bound, on timeout request -> rebind, on nak -> init
			REBINDING,   // wait for ack -> bound, timeout/nak -> init
			INITREBOOT,  // request last ip -> rebooting
			REBOOTING,   // wait for ack -> bound, timeout/nak -> init
		};
		enum class zeroconf_state {
			OFF,           // No zeroconf activity
			START,         // Start Probing with address based on MAC (set last to 0)
			PROBE,         // Probe a new address (if last is 0, take hash of the mac)
			PROBING,       // Startet ARPProbes
			RESERVING,     // Probing was successful, now reserve address (more Probes) until
			               // we really want to use it (wait for DHCP e.g.)
			BIND,          // Bind address and announce it.
			BOUND,         // Listen for conflicting ARP Packets.
			CONFLICT,      // Conflict detected, select new address and restart Probing.
		};

		DeviceManager* const m_dm;

		quint32 m_dhcp_xid = 0;
		bool m_dhcp_xid_unicast = false;
		int m_dhcp_unicast_socket = -1;
		QSocketNotifier* m_dhcp_unicast_read_nf = nullptr;
		dhcp_state m_dhcpstate = dhcp_state::OFF;
		QHostAddress m_dhcp_server_ip;
		QVector<quint8> m_dhcp_server_identifier;
		quint32 m_dhcp_lease_time = 0;

		zeroconf_state m_zc_state = zeroconf_state::OFF;
		QHostAddress m_zc_probe_ip;
		std::unique_ptr<ARPProbe> m_zc_arp_probe;
		std::unique_ptr<ARPWatch> m_zc_arp_watch;
		std::unique_ptr<ARPAnnounce> m_zc_arp_announce;

		std::shared_ptr<libnutcommon::IPv4Config> const m_config;
		libnutcommon::InterfaceProperties m_properties, m_last_notified_properties;
		QString m_localdomain;
		libnutcommon::IPv4UserConfig m_userConfig;
	};
}

#endif /* _NUTS_DEVICE_H */
