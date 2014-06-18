#include "client.h"
#include "libnutcommon/common.h"
#include "dbus.h"
#ifndef LIBNUT_NO_WIRELESS
# include "libnutwireless/cwireless.h"
#endif

namespace libnutclient {
	using namespace libnutcommon;

	CDevice::CDevice(CDeviceManager * parent, QDBusObjectPath m_dbusPath)
	: CNutServiceClient(parent), m_devManager(parent), m_dbusPath(m_dbusPath) {
		updateLogPrefix();
	}

	CDevice::~CDevice() {
		clear();
	}

	bool CDevice::checkInitDone() {
		return 0 == m_initCounter;
	}

	void CDevice::checkInitDone(bool previous) {
		if (!previous && checkInitDone()) {
#ifndef LIBNUT_NO_WIRELESS
			m_needWireless = !(m_config.wpaConfigFile.isEmpty());

			//Only use wpa_supplicant if we need one
			if (m_needWireless) {
				contextLog(tr("wpa_supplicant config file at: %2").arg(m_config.wpaConfigFile));

				m_wlAccess = new libnutwireless::CWireless(this, getName());
				connect(m_wlAccess, SIGNAL(message(QString)), this, SIGNAL(log(QString)));
				connect(m_dbusDevice, SIGNAL(newWirelessNetworkFound()), this, SIGNAL(newWirelessNetworkFound()));

				//Connect to wpa_supplicant only if device is not deactivated
				if (DeviceState::DEACTIVATED != m_properties.state) {
					m_wlAccess->open();
				}
				if (DeviceType::AIR == m_properties.type && DeviceState::CARRIER <= m_properties.state && nullptr != m_wlAccess->getHardware()) {
					m_wlAccess->getHardware()->setSignalQualityPollRate(500);
				}
			}
#endif

			auto const& actEnvPath = m_properties.activeEnvironment;
			m_activeEnvironment = actEnvPath ? m_dbusEnvironments.value(actEnvPath.objectPath(), nullptr) : nullptr;

			m_devManager->deviceInitializationCompleted(this);
		}
	}

	void CDevice::updateLogPrefix() {
		m_logPrefix = (m_properties.name.isEmpty() ? m_dbusPath.path() : m_properties.name) + ": ";
	}

	void CDevice::clear() {
		if (m_dbusDevice) {
			m_dbusDevice->deleteLater();
			m_dbusDevice = nullptr;
		}

		m_environments.clear(); // all environments are also in m_dbusEnvironments
		/* mark environments for later deletion, but clear container before */
		decltype(m_dbusEnvironments) tmp;
		tmp.swap(m_dbusEnvironments);
		for (auto const& env: tmp) {
			env->deleteLater();
		}
	}

	void CDevice::dbusLostService() {
		clear();
		if (!checkInitDone()) m_devManager->deviceInitializationFailed(this);
		deleteLater();
	}

	void CDevice::dbusConnectService(QString service, QDBusConnection connection) {
		if (m_dbusDevice) return;

		m_dbusDevice = new DBusDevice(service, m_dbusPath, connection, this);

		/* all other signals are covered by this one */
		connect(m_dbusDevice, SIGNAL(propertiesChanged(libnutcommon::DeviceProperties)), this, SLOT(dbusPropertiesChanged(libnutcommon::DeviceProperties)));

		auto handleEnvironments = [this](libnutclient::DBusDevice::Result_getEnvironments envPaths) {
			if (!m_dbusDevice) return;
			auto initDone = checkInitDone();
			--m_initCounter;

			for (auto const& envPath: envPaths) {
				++m_initCounter;
				auto env = new CEnvironment(this, envPath, m_environments.size());
				m_dbusEnvironments.insert(envPath, env);
				m_environments.append(nullptr);
			}
			checkInitDone(initDone);
		};
		auto handleEnvironments_error = [this](QDBusError error) {
			emit dbusError(m_dbusPath.path() + ": Device.getEnvironments", error);
			m_devManager->deviceInitializationFailed(this);
		};
		++m_initCounter;
		m_dbusDevice->getEnvironments(handleEnvironments, handleEnvironments_error, this);

		auto handleConfig = [this](libnutcommon::DeviceConfig config) {
			if (!m_dbusDevice) return;
			auto initDone = checkInitDone();
			--m_initCounter;

			m_config = config;

			checkInitDone(initDone);
		};
		auto handleConfig_error = [this](QDBusError error) {
			emit dbusError(m_dbusPath.path() + ": Device.getConfig", error);
			m_devManager->deviceInitializationFailed(this);
		};
		++m_initCounter;
		m_dbusDevice->getConfig(handleConfig, handleConfig_error, this);

		auto handleProperties = [this](libnutcommon::DeviceProperties props) {
			if (!m_dbusDevice) return;
			auto initDone = checkInitDone();
			--m_initCounter;

			// contextLog should show path as context for these message, update m_properties after logging.
			if (DeviceType::AIR == props.type) {
				contextLog(tr("Device properties fetched: name='%1', type=%2, state=%3, essid='%4'").arg(props.name, toStringTr(props.type), toStringTr(props.state), props.essid));
			} else {
				contextLog(tr("Device properties fetched: name='%1', type=%2, state=%3").arg(props.name, toStringTr(props.type), toStringTr(props.state)));
			}
			m_properties = props;
			updateLogPrefix();

			checkInitDone(initDone);
		};
		auto handleProperties_error = [this](QDBusError error) {
			emit dbusError(m_dbusPath.path() + ": Device.getProperties", error);
			m_devManager->deviceInitializationFailed(this);
		};
		++m_initCounter;
		m_dbusDevice->getProperties(handleProperties, handleProperties_error, this);
	}

	void CDevice::environmentInitializationFailed(CEnvironment*) {
		auto initDone = checkInitDone();
		if (initDone) return; /* ignore after init is done */
		/* fail device */
		m_devManager->deviceInitializationFailed(this);
	}

	void CDevice::environmentInitializationCompleted(CEnvironment* environment) {
		auto initDone = checkInitDone();
		if (initDone) return; /* ignore after init is done */

		auto internalEnvNdx = environment->m_index;
		if (nullptr != m_environments[internalEnvNdx]) return; /* ignore after init is done */

		m_environments[internalEnvNdx] = environment;
		--m_initCounter;

		checkInitDone(initDone);
	}

	void CDevice::dbusPropertiesChanged(libnutcommon::DeviceProperties properties) {
		if (!checkInitDone()) {
			/* no event handling yet */
			m_properties = properties;
			updateLogPrefix();
			return;
		}

		auto oldState = m_properties.state;
		bool actenv_changed = properties.activeEnvironment != m_properties.activeEnvironment;
		bool state_changed = properties.state != m_properties.state;
		bool essid_changed = properties.essid != m_properties.essid;

		/* no changes? */
		if (!actenv_changed && !state_changed && !essid_changed) return;

		m_properties = properties;
		updateLogPrefix();

		if (actenv_changed) {
			auto oldenv = m_activeEnvironment;
			auto const& newEnvPath = m_properties.activeEnvironment;
			m_activeEnvironment = newEnvPath ? m_dbusEnvironments.value(newEnvPath.objectPath(), nullptr) : nullptr;

			if (oldenv != m_activeEnvironment) {
				auto actEnvName = m_activeEnvironment ? m_activeEnvironment->getName() : "<none>";
				auto actEnvNdx = m_activeEnvironment ? m_activeEnvironment->getIndex() : -1;
				contextLog(tr("Changed active environment: '%1' (%2)").arg(actEnvName, QString::number(actEnvNdx)));
				emit activeEnvironmentChanged(m_activeEnvironment, oldenv);
			}
		}
		if (state_changed) {
			contextLog(tr("Changed state: %1").arg(toStringTr(m_properties.state)));

#ifndef LIBNUT_NO_WIRELESS
			if (m_needWireless) {
				if (DeviceState::DEACTIVATED == oldState) {
					// got "activated"
					m_wlAccess->open();
				}
				else if (DeviceState::DEACTIVATED == properties.state) {
					m_wlAccess->close();
				}
			}

			if (DeviceType::AIR == m_properties.type && DeviceState::CARRIER <= m_properties.state && nullptr != m_wlAccess->getHardware()) {
				m_wlAccess->getHardware()->setSignalQualityPollRate(500);
			}
#endif

			emit stateChanged(m_properties.state);
		}
		if (essid_changed) {
			contextLog(tr("Changed essid: '%1'").arg(m_properties.essid));
			emit essidUpdate(m_properties.essid);
		}

		emit propertiesChanged(m_properties);
		emit newDataAvailable();
	}

	void CDevice::enable() {
		if (m_dbusDevice) m_dbusDevice->enable();
	}

	void CDevice::disable() {
		if (m_dbusDevice) m_dbusDevice->disable();
	}

	void CDevice::setEnvironment(CEnvironment* environment) {
		if (!m_dbusDevice) return;

		if (nullptr == environment) {
			m_dbusDevice->setEnvironment(-1);
		} else {
			if (DeviceState::DEACTIVATED == m_properties.state) {
				enable();
			}
			m_dbusDevice->setEnvironment(environment->path());
		}
	}
}
