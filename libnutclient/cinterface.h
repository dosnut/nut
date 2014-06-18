#ifndef LIBNUTCLIENT_CINTERFACE_H
#define LIBNUTCLIENT_CINTERFACE_H

#include <QObject>
#include <QList>

#include "cnutservice.h"
#include "libnutcommon/common.h"

namespace libnutclient {
	class DBusInterface_IPv4;
	class CEnvironment;
}

namespace libnutclient {
	/** @brief The Interface represents one network interface

		It provides information about its configuration (ip,netmask,gateway,dnsServers,state).
		There's also a function to set an own interface configuration
	*/
	class CInterface : public CNutServiceClient {
		Q_OBJECT
	private:
		friend class CEnvironment;
		friend class DBusInterface_IPv4;

		CEnvironment* m_environment = nullptr;
		DBusInterface_IPv4* m_dbusInterface = nullptr;
		QDBusObjectPath m_dbusPath;

		int m_initCounter = 0;
		bool checkInitDone();
		void checkInitDone(bool previous);

		libnutcommon::InterfaceProperties m_properties;
		libnutcommon::IPv4Config m_config;
		libnutcommon::IPv4UserConfig m_userConfig;

		int const m_index;

		void updateLogPrefix();
		void clear();

	protected:
		void dbusLostService() override;
		void dbusConnectService(QString service, QDBusConnection connection) override;

	private slots:
		void dbusPropertiesChanged(libnutcommon::InterfaceProperties properties);
		void dbusUserConfigUpdate(libnutcommon::IPv4UserConfig userConfig);

	public:
		CInterface(CEnvironment* parent, QDBusObjectPath dbusPath, int index);
		~CInterface();

		int getIndex() const { return m_index; }

		libnutcommon::InterfaceProperties const& getProperties() const { return m_properties; }
		libnutcommon::InterfaceState getState() const { return m_properties.state; }
		QHostAddress getIp() const { return m_properties.ip;}
		QHostAddress getNetmask() const { return m_properties.netmask; }
		QHostAddress getGateway() const { return m_properties.gateway; }
		const QList<QHostAddress>& getDnsServers() const { return m_properties.dnsServers; }
		int getGatewayMetric() const { return m_properties.gatewayMetric; }
		bool needUserSetup() const { return m_properties.needUserSetup; }

		const libnutcommon::IPv4Config& getConfig() const { return m_config; }

		const libnutcommon::IPv4UserConfig& getUserConfig() const { return m_userConfig; }

	public slots:
		void setUserConfig(const libnutcommon::IPv4UserConfig& userConfig);

	signals:
		void newDataAvailable();

		void stateChanged(libnutcommon::InterfaceState state);
		void propertiesChangedd(libnutcommon::InterfaceProperties properties);
		void userConfigChanged(libnutcommon::IPv4UserConfig userConfig);
	};
}

#endif
