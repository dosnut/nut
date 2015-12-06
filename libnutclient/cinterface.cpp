#include "client.h"
#include "libnutclientbase/dbus.h"
#include "libnutcommon/common.h"

namespace libnutclient {
	using namespace libnutcommon;

	CInterface::CInterface(CEnvironment * parent, QDBusObjectPath dbusPath, int index)
	: CNutServiceClient(parent), m_environment(parent), m_dbusPath(dbusPath), m_index(index) {
		updateLogPrefix();
	}

	CInterface::~CInterface() {
		clear();
	}

	bool CInterface::checkInitDone() {
		return 0 == m_initCounter;
	}

	void CInterface::checkInitDone(bool previous) {
		if (!previous && checkInitDone()) {
			m_environment->initializationCompleted(this);
		}
	}

	void CInterface::updateLogPrefix() {
		m_logPrefix = "iface #" + QString::number(m_index) + ": ";
	}

	void CInterface::clear() {
		if (m_dbusInterface) {
			m_dbusInterface->deleteLater();
			m_dbusInterface = nullptr;
		}
	}

	void CInterface::dbusLostService() {
		clear();
		if (!checkInitDone()) m_environment->initializationFailed(this);
		deleteLater();
	}

	void CInterface::dbusConnectService(QString service, QDBusConnection connection) {
		if (m_dbusInterface) return;

		m_dbusInterface = new libnutclientbase::DBusInterface_IPv4(service, m_dbusPath, connection, this);

		/* all other signals are covered by these two */
		connect(m_dbusInterface, &libnutclientbase::DBusInterface_IPv4::propertiesChanged, this, &CInterface::dbusPropertiesChanged);
		connect(m_dbusInterface, &libnutclientbase::DBusInterface_IPv4::userConfigChanged, this, &CInterface::dbusUserConfigUpdate);

		auto handleConfig = [this](libnutcommon::IPv4Config config) {
			if (!m_dbusInterface) return;
			auto initDone = checkInitDone();
			--m_initCounter;

			m_config = config;
			checkInitDone(initDone);
		};
		auto handleConfig_error = [this](QDBusError error) {
			emit dbusError(m_dbusPath.path() + ": Interface_IPv4.getConfig", error);
			m_environment->initializationFailed(this);
		};
		++m_initCounter;
		m_dbusInterface->getConfig(handleConfig, handleConfig_error, this);

		auto handleProperties = [this](libnutcommon::InterfaceProperties props) {
			auto initDone = checkInitDone();
			--m_initCounter;
			if (!m_dbusInterface) {
				m_environment->initializationFailed(this);
				return;
			}

			m_properties = props;
			checkInitDone(initDone);
		};
		auto handleProperties_error = [this](QDBusError error) {
			emit dbusError(m_dbusPath.path() + ": Interface_IPv4.getProperties", error);
			m_environment->initializationFailed(this);
		};
		++m_initCounter;
		m_dbusInterface->getProperties(handleProperties, handleProperties_error, this);

		auto handleUserConfig = [this](libnutcommon::IPv4UserConfig userConfig) {
			auto initDone = checkInitDone();
			--m_initCounter;
			if (!m_dbusInterface) {
				m_environment->initializationFailed(this);
				return;
			}

			m_userConfig = userConfig;
			checkInitDone(initDone);
		};
		auto handleUserConfig_error = [this](QDBusError error) {
			emit dbusError(m_dbusPath.path() + ": Interface_IPv4.getUserConfig", error);
			m_environment->initializationFailed(this);
		};
		++m_initCounter;
		m_dbusInterface->getUserConfig(handleUserConfig, handleUserConfig_error, this);
	}

	void CInterface::dbusPropertiesChanged(libnutcommon::InterfaceProperties properties) {
		if (!checkInitDone()) {
			/* no event handling yet */
			m_properties = properties;
			return;
		}

		bool state_changed = properties.state != m_properties.state;

		m_properties = properties;

		if (state_changed) {
			contextLog(tr("Interface state changed to %1").arg(toStringTr(m_properties.state)));
			emit stateChanged(m_properties.state);
		}

		emit propertiesChangedd(m_properties);
		emit newDataAvailable();
	}


	void CInterface::dbusUserConfigUpdate(libnutcommon::IPv4UserConfig userConfig) {
		if (!checkInitDone()) {
			/* no event handling yet */
			m_userConfig = userConfig;
			return;
		}

		emit userConfigChanged(userConfig);
		emit newDataAvailable();
	}

	void CInterface::setUserConfig(const libnutcommon::IPv4UserConfig &cuserConfig) {
		if (m_dbusInterface) m_dbusInterface->setUserConfig(cuserConfig);
	}
}
