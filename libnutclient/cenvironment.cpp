#include "client.h"
#include "libnutclientbase/dbus.h"
#include "libnutcommon/common.h"

namespace libnutclient {
	using namespace libnutcommon;

	CEnvironment::CEnvironment(CDevice * parent, QDBusObjectPath dbusPath, int index)
	: CNutServiceClient(parent), m_device(parent), m_dbusPath(dbusPath), m_index(index) {
		updateLogPrefix();
	}

	CEnvironment::~CEnvironment() {
		clear();
	}

	void CEnvironment::updateLogPrefix() {
		m_logPrefix = (m_properties.name.isEmpty() ? m_dbusPath.path() : QString::number(m_properties.id) + "@" + m_properties.name) + ": ";
	}

	bool CEnvironment::checkInitDone() {
		return 0 == m_initCounter;
	}

	void CEnvironment::checkInitDone(bool previous) {
		if (!previous && checkInitDone()) {
			m_device->environmentInitializationCompleted(this);
		}
	}

	void CEnvironment::clear() {
		if (m_dbusEnvironment) {
			m_dbusEnvironment->deleteLater();
			m_dbusEnvironment = nullptr;
		}

		m_interfaces.clear(); // all interfaces are also in m_dbusInterfaces
		/* mark interfaces for later deletion, but clear container before */
		decltype(m_dbusInterfaces) tmp;
		tmp.swap(m_dbusInterfaces);
		for (auto const& env: tmp) {
			env->deleteLater();
		}
	}

	void CEnvironment::dbusLostService() {
		clear();
		if (!checkInitDone()) m_device->environmentInitializationFailed(this);
		deleteLater();
	}

	void CEnvironment::dbusConnectService(QString service, QDBusConnection connection) {
		if (m_dbusEnvironment) return;

		m_dbusEnvironment = new libnutclientbase::DBusEnvironment(service, m_dbusPath, connection, this);

		/* all other signals are covered by this one */
		connect(m_dbusEnvironment, SIGNAL(propertiesChanged(libnutcommon::EnvironmentProperties)), this, SLOT(dbusPropertiesChanged(libnutcommon::EnvironmentProperties)));

		auto handleInterfaces = [this](libnutclientbase::DBusEnvironment::Result_getInterfaces ifPaths) {
			if (!m_dbusEnvironment) return;
			auto initDone = checkInitDone();
			--m_initCounter;

			for (auto const& ifPath: ifPaths) {
				++m_initCounter;
				auto interface = new CInterface(this, ifPath, m_interfaces.size());
				m_dbusInterfaces.insert(ifPath, interface);
				m_interfaces.append(nullptr);
			}
			checkInitDone(initDone);
		};
		auto handleInterfaces_error = [this](QDBusError error) {
			emit dbusError(m_dbusPath.path() + ": Environment.getInterfaces", error);
			m_device->environmentInitializationFailed(this);
		};
		++m_initCounter;
		m_dbusEnvironment->getInterfaces(handleInterfaces, handleInterfaces_error, this);

		auto handleConfig = [this](libnutcommon::EnvironmentConfig config) {
			if (!m_dbusEnvironment) return;
			auto initDone = checkInitDone();
			--m_initCounter;

			m_config = config;
			checkInitDone(initDone);
		};
		auto handleConfig_error = [this](QDBusError error) {
			emit dbusError(m_dbusPath.path() + ": Environment.getConfig", error);
			m_device->environmentInitializationFailed(this);
		};
		++m_initCounter;
		m_dbusEnvironment->getConfig(handleConfig, handleConfig_error, this);

		auto handleProperties = [this](libnutcommon::EnvironmentProperties props) {
			auto initDone = checkInitDone();
			--m_initCounter;
			if (!m_dbusEnvironment) {
				m_device->environmentInitializationFailed(this);
				return;
			}

			m_properties = props;
			updateLogPrefix();
			checkInitDone(initDone);
		};
		auto handleProperties_error = [this](QDBusError error) {
			emit dbusError(m_dbusPath.path() + ": Environment.getProperties", error);
			m_device->environmentInitializationFailed(this);
		};
		++m_initCounter;
		m_dbusEnvironment->getProperties(handleProperties, handleProperties_error, this);
	}

	void CEnvironment::initializationFailed(CInterface*) {
		auto initDone = checkInitDone();
		if (initDone) return; /* ignore after init is done */
		/* fail device */
		m_device->environmentInitializationFailed(this);
	}

	void CEnvironment::initializationCompleted(CInterface* interface) {
		auto initDone = checkInitDone();
		if (initDone) return; /* ignore after init is done */

		auto internalIfNdx = interface->m_index;
		if (nullptr != m_interfaces[internalIfNdx]) return; /* ignore after init is done */

		m_interfaces[internalIfNdx] = interface;
		--m_initCounter;

		checkInitDone(initDone);
	}

	void CEnvironment::dbusPropertiesChanged(libnutcommon::EnvironmentProperties properties) {
		if (!checkInitDone()) {
			/* no event handling yet */
			m_properties = properties;
			updateLogPrefix();
			return;
		}

		bool active_changed = properties.active != m_properties.active;
		bool sr_changed = (properties.selectResult != m_properties.selectResult || properties.selectResults != m_properties.selectResults);

		m_properties = properties;
		updateLogPrefix();

		if (active_changed) emit activeChanged(m_properties.active);
		if (sr_changed) emit selectResultsChanged();

		emit newDataAvailable();
	}

	void CEnvironment::enter() {
		m_device->setEnvironment(this);
	}
}
