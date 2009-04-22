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
		bool m_needUserSetup;
		int m_index;

		bool m_propertiesFetched;
		bool m_configFetched;
		bool m_userConfigFetched;
		bool m_needUserSetupFeteched;
		bool m_initCompleted;

		void refreshAll();
		void checkInitCompleted();
	private slots:
		void dbusStateChanged(libnutcommon::InterfaceProperties properties);

		//dbus return functions
		void dbusretGetProperties(libnutcommon::InterfaceProperties properties);
		void dbusretGetConfig(libnutcommon::IPv4Config config);
		void dbusretGetNeedUserSetup(bool need);
		void dbusretSetUserConfig(bool worked);
		void dbusretGetUserConfig(libnutcommon::IPv4UserConfig config);

		void dbusret_errorOccured(QDBusError error, QString method = QString());
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

		void init();
	public slots:
		void activate();
		void deactivate();
		bool needUserSetup();
		void setUserConfig(const libnutcommon::IPv4UserConfig &cuserConfig);
		
	signals:
		void initializationFailed(CInterface * interface); //TODO:Implement this: has to be called if init fails
		void initializationCompleted(CInterface * interface);

		void newDataAvailable();

		void stateChanged(libnutcommon::InterfaceState state);
		void setUserConfig(bool worked);
	};
}

#endif
