#ifndef LIBNUTCLIENT_CINTERFACE_H
#define LIBNUTCLIENT_CINTERFACE_H

#include <QObject>
#include <QList>

#include "clibnut.h"


namespace libnutclient {
	class CLog;
	class DBusInterfaceInterface_IPv4;
	class CDeviceManager;
	class CDevice;
	class CEnvironment;
	class CInterface;
}

namespace libnutclient {

	/** @brief The Interface represents one network interface
		
		It provides information about its configuration (ip,netmask,gateway,dnsservers,state).
		There's also a function to set an own interface configuration
		It also emits a signal on data changes.
		
	*/
	class CInterface : public CLibNut {
		Q_OBJECT
		friend class CDeviceManager;
		friend class CDevice;
		friend class CEnvironment;
		friend class DBusInterfaceInterface_IPv4;
	private:
		//CEnvironment * parent;
		QDBusObjectPath m_dbusPath;
		CLog * log;
		DBusInterfaceInterface_IPv4 * m_dbusInterface;
		libnutcommon::IPv4Config m_config;
		libnutcommon::IPv4UserConfig m_userConfig;
		libnutcommon::InterfaceState m_state;
		QHostAddress m_ip;
		QHostAddress m_netmask;
		QHostAddress m_gateway;
		QList<QHostAddress> m_dnsservers;
		int m_index;
		void refreshAll();
	private slots:
		void dbusStateChanged(libnutcommon::InterfaceProperties properties);
	public:
		inline libnutcommon::InterfaceState getState() const { return m_state; }
		inline const QHostAddress& getIp() { return m_ip;}
		inline const QHostAddress& getNetmask() { return m_netmask; }
		inline const QHostAddress& getGateway() { return m_gateway; }
		inline const QList<QHostAddress>& getDnsServers() { return m_dnsservers; }
		inline int getIndex() const { return m_index;}
		
		libnutcommon::IPv4UserConfig getUserConfig(bool refresh=false);
		const libnutcommon::IPv4Config& getConfig() const { return m_config; }

		CInterface(CEnvironment * parent, QDBusObjectPath dbusPath);
		~CInterface();
	public slots:
		void activate();
		void deactivate();
		bool needUserSetup();
		bool setUserConfig(const libnutcommon::IPv4UserConfig &cuserConfig);
		
	signals:
		void stateChanged(libnutcommon::InterfaceState state);
	};
}

#endif
