
#ifndef _NUTS_DEVICE_H
#define _NUTS_DEVICE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QHostAddress>
#include <QBitArray>
#include <QSocketNotifier>
#include <QLinkedList>
#include <QTimer>
#include <QProcess>

#include <common/macaddress.h>
#include <common/types.h>

namespace nuts {
	class DBusDeviceManager;
	
	class DeviceManager;
	class Device;
	class Environment;
	class Interface;
	class Interface_IPv4;
	
	class ARP;
	class DHCPPacket;
	class DHCPClientPacket;
};

#include "config.h"
#include "hardware.h"
#include "arp.h"
#include "events.h"

namespace nuts {
	/** @brief The DeviceManager keeps track of all hardware devices.
		@author Stefan Bühler <stbuehler@web.de>
		
		On creation, the DeviceManager reads the config file and then tries to
		find every device specified in it.
		
		If devices are created later (or are removed) by the kernel, it emits
		signals if they are managed by the DeviceManager, i.e. are in the config file.
	 */
	class DeviceManager : public QObject {
		Q_OBJECT
		private:
			ConfigParser configParser;
			Events m_events;
			nut::Config *config;
			QTimer carrier_timer;
			/// Internal structure for delaying carrier events.
			struct ca_evt {
				QString ifName;
				int ifIndex;
				bool up;
			};
			QLinkedList<struct ca_evt> ca_evts;
			
			QHash<QString, Device*> devices;
			
			void addDevice(const QString &ifname, nut::DeviceConfig *dc);
			
			friend class Device;
			friend class Interface_IPv4;
			
			HardwareManager hwman;
			
		private slots:
			void ca_timer();
			void gotCarrier(const QString &ifName, int ifIndex, const QString &essid);
			void lostCarrier(const QString &ifName);
			void newDevice(const QString &ifName, int ifIndex);
			void delDevice(const QString &ifname);
		
		public:
			DBusDeviceManager *dbus_devMan;
			/**
			 * @brief Constructs the DeviceManager
			 * @param configFile Path to the configfile.
			 */
			DeviceManager(const QString &configFile);
			virtual ~DeviceManager();
			
			/**
			 * @brief Gets all managed devices.
			 * @return Hash of devices; the keys in the hash represent the device name in the kernel.
			 */
			const QHash<QString, Device*>& getDevices() { return devices; }
			/**
			 * @brief Get the config.
			 * @return Config
			 */
			const nut::Config& getConfig() { return *config; }
		
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
		Q_PROPERTY(QString name READ getName) //!< Name of the device in the kernel, e.g. "eth0"
		Q_PROPERTY(int environment READ getEnvironment WRITE setEnvironment) //!< Number of the active environment, or -1
		Q_PROPERTY(libnut::DeviceState state READ getState)
		private:
			void setState(libnut::DeviceState state);
			
			// only false on failed startup, not on "unused"
			bool startWPASupplicant();
			void stopWPASupplicant();
			
		private:
			friend class DeviceManager;
			friend class Environment;
			friend class Interface_IPv4;
			friend class DHCPPacket;
			friend class DHCPClientPacket;
			friend class ARP;
			
			ARP m_arp;
			DeviceManager *dm;
			QString name;
			int interfaceIndex;
			nut::DeviceConfig *config;
			
			int activeEnv, nextEnv, m_userEnv;
			int m_waitForEnvSelects;
			
			libnut::DeviceState m_state;
			QList<Environment*> envs;
			
			// DHCP
			QHash< quint32, Interface_IPv4* > dhcp_xid_iface;
			int dhcp_client_socket;
			QSocketNotifier *dhcp_read_nf, *dhcp_write_nf;
			QLinkedList< QByteArray > dhcp_write_buf;
			
			// Device properties
			nut::MacAddress macAddress;
			bool m_hasWLAN;
			QString m_essid;
			
			// WPA
			QProcess *m_wpa_supplicant;
			
			// Called from Environment
			void envUp(Environment*);
			void envDown(Environment*);
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
			
		private:
			Device(DeviceManager* dm, const QString &name, nut::DeviceConfig *config, bool hasWLAN);
			virtual ~Device();
			
		public slots:
			// Properties
			QString getName() { return name; } //!< Name of the device in the kernel, e.g. "eth0"
			const nut::DeviceConfig& getConfig() { return *config; }
			
			int getEnvironment() { return activeEnv; } //!< Active environment, or -1
			
			int getUserPreferredEnvironment() { return m_userEnv; }
			void setUserPreferredEnvironment(int env);
			
			libnut::DeviceState getState() { return m_state; }
			
			/**
			 * Enable device.
			 * @param force Force the use of a kernel interface, even if it is already up and perhaps in use.
			 *	nuts will takeover the interface.
			 * @return 
			 */
			bool enable(bool force = false);
			void disable(); //!< Disable device
			
			const QList<Environment*>& getEnvironments() { return envs; }
			
			bool hasWLAN() { return m_hasWLAN; }
			QString essid() { return m_essid; }
			nut::MacAddress getMacAddress() { return macAddress; }
			
		signals:
			void stateChanged(libnut::DeviceState newState, libnut::DeviceState oldState, Device* device);
	};
	
	class Environment : public QObject {
		Q_OBJECT
		Q_PROPERTY(Device* device READ getDevice)
		
		private:
			friend class Device;
			friend class Interface;
			friend class Interface_IPv4;
			friend class DHCPPacket;
			friend class DHCPClientPacket;
			
			Device *device;
			QList<Interface*> ifs;
			QVector<nut::SelectResult> m_selectResults;
			nut::SelectResult m_selectResult;
			
			nut::EnvironmentConfig *config;
			QBitArray ifUpStatus;
			bool envIsUp, envStart;
			
			int m_id;
			
			void checkStatus();
			
			void start();
			void stop();
			
			void ifUp(Interface*);
			void ifDown(Interface*);
			
			int selArpWaiting;
			bool startSelect();
			void checkSelectState();
			
			bool m_needUserSetup;
			void updateNeedUserSetup();
		
		private slots:
			void selectArpRequestTimeout(QHostAddress ip);
			void selectArpRequestFoundMac(nut::MacAddress mac, QHostAddress ip);
			
		public:
			Environment(Device *device, nut::EnvironmentConfig *config, int id);
			virtual ~Environment();
			
			Device* getDevice() { return device; }
			
			const QList<Interface*>& getInterfaces() { return ifs; }
			int getID() { return m_id; }
			QString getName() { return config->getName(); }
			const nut::EnvironmentConfig& getConfig() { return *config; }
			
			bool selectionDone() { return (selArpWaiting == -1); }
			nut::SelectResult getSelectResult() { return m_selectResult; }
			QVector<nut::SelectResult> getSelectResults() { return m_selectResults; }
	};
	
	class Interface : public QObject {
		Q_OBJECT
		protected:
			friend class Environment;
			Environment *m_env;
			int m_index;
			
			bool m_needUserSetup;
			void updateNeedUserSetup(bool needUserSetup);
			
		public:
			Interface(Environment *env, int index);
			virtual ~Interface();
			
			virtual void start() = 0;
			virtual void stop() = 0;
			
			int getIndex() { return m_index; }
			
			bool needUserSetup() { return m_needUserSetup; }
			
			Environment *getEnvironment() { return m_env; }
	};
	
	class Interface_IPv4 : public Interface {
		Q_OBJECT
		private:
			int dhcp_timer_id;     // timer id
			int dhcp_retry;        // count retries
			virtual void timerEvent(QTimerEvent *event);
			void dhcp_set_timeout(int msec);
		
		protected:
			enum dhcp_state {
				DHCPS_OFF,
				DHCPS_INIT_START,  // reset dhcp_retry, -> DHCPS_INIT
				DHCPS_INIT,        // (++dhcp_retry >= 5) -> failed, Discover all -> selecting
	 			DHCPS_SELECTING,   // Waiting for offer; request a offer -> requesting, timeout -> init
				DHCPS_REQUESTING,  // Requested a offer, waiting for ack -> bound, nak -> init
				DHCPS_BOUND,       // bound, on timeout request -> renew
				DHCPS_RENEWING,    // wait for ack -> bound, on timeout request -> rebind, on nak -> init
				DHCPS_REBINDING,   // wait for ack -> bound, timeout/nak -> init
				DHCPS_INITREBOOT,  // request last ip -> rebooting
				DHCPS_REBOOTING    // wait for ack -> bound, timeout/nak -> init
			};
			enum zeroconf_state {
				ZCS_OFF,           // No zeroconf activity
				ZCS_START,         // Start Probing
				ZCS_PROBING,       // Startet ARPProbes
				ZCS_RESERVING,     // Probing was successful, now reserve address (more Probes) until
				                   // we really want to use it (wait for DHCP e.g.)
				ZCS_ANNOUNCING,    // Announce Address
				ZCS_BOUND,         // Announce was successful. Listen for conflicting ARP Packets.
				ZCS_CONFLICT,      // Conflict detected, select new address and restart Probing.
			};
			
			DeviceManager *dm;
			
			quint32 dhcp_xid;
			bool dhcp_xid_unicast;
			int dhcp_unicast_socket;
			QSocketNotifier *dhcp_unicast_read_nf;
			dhcp_state dhcpstate;
			QHostAddress dhcp_server_ip;
			QVector<quint8> dhcp_server_identifier;
			quint32 dhcp_lease_time;
			
			zeroconf_state zc_state;
			QHostAddress zc_probe_ip;
			ARPProbe *zc_arp_probe;
			
			nut::IPv4Config *m_config;
			
			libnut::InterfaceState m_ifstate;
			
			nut::IPv4UserConfig m_userConfig;
			
			void dhcp_send_discover();
			void dhcp_send_request(DHCPPacket *offer);
			void dhcp_send_renew();
			void dhcp_send_rebind();
			void dhcp_send_release();
			void dhcp_setup_interface(DHCPPacket *ack, bool renewing = false);
			void dhcpAction(DHCPPacket *source = 0);
			
			void zeroconf_setup_interface();
			void zeroconfProbe();   // select new ip and start probe it
			void zeroconfFree();    // free ARPProbe and ARPWatch
			void zeroconfAction();  // automaton
			
			void startDHCP();
			void stopDHCP();
			void startZeroconf();
			void startStatic();
			void startUserStatic();
			
			void systemUp();
			void systemDown();
			
			bool setupUnicastDHCP(bool temporary = false);
			void closeUnicastDHCP();
			bool sendUnicastDHCP(DHCPPacket *packet);
			bool registerXID(quint32 xid);
			bool registerUnicastXID(quint32 xid);
			void releaseXID();
			void dhcpReceived(DHCPPacket *packet);
			
			friend class DHCPPacket;
			friend class DHCPClientPacket;
			friend class Environment;
			friend class Device;
		
		protected slots:
			void readDHCPUnicastClientSocket();
			
			void zc_conflict();
			void zc_ready();
		
		public:
			Interface_IPv4(Environment *env, int index, nut::IPv4Config *config);
			virtual ~Interface_IPv4();
			virtual void start();
			virtual void stop();
			
			QHostAddress ip, netmask, gateway;
			QString localdomain;
			QList<QHostAddress> dnsserver;
			
			const nut::IPv4Config& getConfig() { return *m_config; }
			libnut::InterfaceState getState() { return m_ifstate; }
			
			bool setUserConfig(const nut::IPv4UserConfig &userConfig);
			const nut::IPv4UserConfig &getUserConfig() { return m_userConfig; }
			
		signals:
			void interfaceUp(Interface_IPv4* iface);
			void interfaceDown(Interface_IPv4* iface);
	};
};

#endif
