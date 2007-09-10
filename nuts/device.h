
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

#include <common/macaddress.h>
#include <common/types.h>

namespace nuts {
	class DeviceManager;
	class Device;
	class Environment;
	class Interface;
	class Interface_IPv4;
	
	class DHCPPacket;
	class DHCPClientPacket;
};

#include "config.h"
#include "hardware.h"

namespace nuts {
	class DeviceManager : public QObject {
		Q_OBJECT
		protected:
			ConfigParser configParser;
			nut::Config *config;
			HardwareManager hwman;
			QTimer carrier_timer;
			struct ca_evt {
				QString ifName;
				int ifIndex;
				bool up;
			};
			QLinkedList<struct ca_evt> ca_evts;
			
			QHash<QString, Device*> devices;
			
			friend class HardwareManager;
			friend class Device;
			friend class Environment;
			friend class Interface_IPv4;
			
		private slots:
			void ca_timer();
		protected slots:
			void gotCarrier(const QString &ifName, int ifIndex);
			void lostCarrier(const QString &ifName);
		
		public:
			DeviceManager(const QString &configFile);
			virtual ~DeviceManager();
			
			const QHash<QString, Device*>& getDevices() { return devices; }
			nut::Config *getConfig() { return config; }
		
		signals:
			void deviceAdded(QString devName, Device *dev);
			void deviceRemoved(QString devName, Device *dev);
	};
	
	class Device : public QObject {
		Q_OBJECT
		Q_PROPERTY(QString name READ getName)
		Q_PROPERTY(int environment READ getEnvironment WRITE setEnvironment)
		Q_PROPERTY(bool state READ getState)
		private:
			void setState(libnut::DeviceState state);
			
		protected:
			friend class DeviceManager;
			friend class Environment;
			friend class Interface_IPv4;
			friend class DHCPPacket;
			friend class DHCPClientPacket;
			
			DeviceManager *dm;
			QString name;
			int interfaceIndex;
			nut::DeviceConfig *config;
			int activeEnv, nextEnv;
			libnut::DeviceState m_state;
			QList<Environment*> envs;
			QHash< quint32, Interface_IPv4* > dhcp_xid_iface;
			int dhcp_client_socket;
			QSocketNotifier *dhcp_read_nf, *dhcp_write_nf;
			QLinkedList< QByteArray > dhcp_write_buf;
			nut::MacAddress macAddress;
		
			void envUp(Environment*);
			void envDown(Environment*);
			
			void gotCarrier(int ifIndex);
			void lostCarrier();
			
			// DHCP
			bool registerXID(quint32 xid, Interface_IPv4 *iface);
			void unregisterXID(quint32 xid);
			void sendDHCPClientPacket(DHCPPacket *packet);
			void setupDHCPClientSocket();
			void closeDHCPClientSocket();
		
		private slots:
			void readDHCPClientSocket();
			void writeDHCPClientSocket();
			
		protected:
		private slots:
			// ARP
			
		protected:
			nut::MacAddress getMacAddress();
			
		public:
			Device(DeviceManager* dm, const QString &name, nut::DeviceConfig *config);
			virtual ~Device();
			
		public slots:
			// Properties
			QString getName();
			
			int getEnvironment();
			void setEnvironment(int env);
			
			libnut::DeviceState getState() { return m_state; }
			
			// enable(true) forces the use of an interface, even if it is already up.
			void enable(bool force = false);
			void disable();
			
			const QList<Environment*>& getEnvironments() { return envs; }
		
		signals:
			void stateChanged(libnut::DeviceState newState, libnut::DeviceState oldState);
			void environmentChanged(int newEnv, int oldEnv);
	};
	
	class Environment : public QObject {
		Q_OBJECT
		Q_PROPERTY(Device* device READ getDevice)
		
		protected:
			friend class Device;
			friend class Interface;
			friend class Interface_IPv4;
			friend class DHCPPacket;
			friend class DHCPClientPacket;
			
			Device *device;
			QList<Interface*> ifs;
			
			nut::EnvironmentConfig *config;
			QBitArray ifUpStatus;
			bool envIsUp, envStart;
			
			int m_id;
			
			void checkStatus();
			
			void start();
			void stop();
			
			void ifUp(Interface*);
			void ifDown(Interface*);
			
		public:
			Environment(Device *device, nut::EnvironmentConfig *config, int id);
			virtual ~Environment();
			
			Device* getDevice();
			
			const QList<Interface*>& getInterfaces();
			int getID() { return m_id; }
			QString getName() { return config->getName(); }
	};
	
	class Interface : public QObject {
		Q_OBJECT
		protected:
			friend class Environment;
			Environment *m_env;
			int m_index;
			
		public:
			Interface(Environment *env, int index);
			virtual ~Interface();
			
			virtual void start() = 0;
			virtual void stop() = 0;
			
			int getIndex() { return m_index; }
	};
	
	class Interface_IPv4 : public Interface {
		Q_OBJECT
		protected:
			enum dhcp_state {
				DHCPS_OFF, DHCPS_FAILED,
				DHCPS_INIT,        // Discover all
	 			DHCPS_SELECTING,   // Waiting for offer; request a offer -> requesting
				DHCPS_REQUESTING,  // Requested a offer, waiting for ack -> bound, nak -> init
				DHCPS_BOUND,       // bound, on timeout request -> renew
				DHCPS_RENEWING,    // wait for ack -> bound, on timeout request -> rebind, on nak -> init
				DHCPS_REBINDING,   // wait for ack -> bound, timeout/nak -> init
				DHCPS_INITREBOOT,  // request last ip -> rebooting
				DHCPS_REBOOTING    // wait for ack -> bound, timeout/nak -> init
			};
			DeviceManager *dm;
			quint32 dhcp_xid;
			dhcp_state dhcpstate;
			QVector<quint8> dhcp_server_identifier;
			quint32 dhcp_lease_time;
			
			void dhcp_send_discover();
			void dhcp_send_request(DHCPPacket *offer);
			void dhcp_setup_interface(DHCPPacket *ack);
			void dhcpAction(DHCPPacket *source = 0);
			
			void startDHCP();
			void startZeroconf();
			void startStatic();
			
			void systemUp();
			void systemDown();
			
			bool registerXID(quint32 xid);
			void dhcpReceived(DHCPPacket *packet);
			
			friend class DHCPPacket;
			friend class DHCPClientPacket;
			friend class Environment;
			friend class Device;
		
		public:
			Interface_IPv4(Environment *env, int index, nut::IPv4Config *config);
			virtual ~Interface_IPv4();
			virtual void start();
			virtual void stop();
			
			QHostAddress ip, netmask, gateway;
			QString localdomain;
			QList<QHostAddress> dnsserver;
			
			nut::IPv4Config *config;
	};
};

#endif
