#include "cnutservice.h"
#include "clibnut.h"
#include <libnutcommon/dbusmanager.h>

#include <QDBusServiceWatcher>

namespace libnutclient {
	using namespace libnutcommon;

	namespace {
		const QString Service { NUT_DBUS_URL };
		const QString DBusConnectionName { "libnutclient_system_bus" };
	}

	CNutService::CNutService(QString service, QObject* parent)
	: QObject(parent), m_dbusService(service) {
		m_dbusManager = new DBusManager();
		m_dbusManager->setParent(this);

		m_dbusServiceWatcher = new QDBusServiceWatcher(this);

		connect(m_dbusManager, &DBusManager::connected, this, &CNutService::dbus_connected);
		connect(m_dbusManager, &DBusManager::disconnected, this, &CNutService::dbus_disconnected);
		connect(m_dbusManager, &DBusManager::waiting, this, &CNutService::dbus_waiting);

		connect(m_dbusServiceWatcher, &QDBusServiceWatcher::serviceRegistered, this, &CNutService::sw_dbusServiceRegistered);
		connect(m_dbusServiceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &CNutService::sw_dbusServiceUnregistered);
		m_dbusServiceWatcher->addWatchedService(m_dbusService);
	}

	CNutService::CNutService(QObject* parent)
	: CNutService(Service, parent) {
	}

	bool CNutService::isConnected() {
		return m_dbusServiceWatcher && m_dbusServiceWatcher->connection().isConnected();
	}

	void CNutService::dbus_connected(QDBusConnection const& connection) {
		m_dbusServiceWatcher->setConnection(connection);

		auto reply = connection.interface()->isServiceRegistered(m_dbusService);
		if (reply.isValid() && reply && !m_servicePresent) {
			sw_dbusServiceRegistered();
		} else {
			emit log(tr("Couldn't find service '%1'. Please start nuts.").arg(m_dbusService));
		}
	}

	void CNutService::dbus_disconnected(QDBusConnection const& connection) {
		emit log(tr("Error while trying to access the dbus service; please make sure that DBus is running"));
		m_dbusServiceWatcher->setConnection(QDBusConnection(QString{}));
		if (m_servicePresent) sw_dbusServiceUnregistered();
	}

	void CNutService::dbus_waiting() {
		emit log("Waiting for DBus service to start");
	}

	void CNutService::sw_dbusServiceRegistered() {
		emit log(tr("NUTS has been started"));
		m_servicePresent = true;
		emit dbusConnectService(m_dbusService, m_dbusServiceWatcher->connection());
	}

	void CNutService::sw_dbusServiceUnregistered() {
		emit log(tr("NUTS has been stopped"));
		m_servicePresent = false;
		emit dbusLostService();

		if (!isConnected()) m_dbusManager->checkConnection();
	}

	void CNutService::handle_dbusError(QString method, QDBusError error) {
		emit log(QString("Error occured in dbus: %1 at %2").arg(toString(error), method));

		if (QDBusError::AccessDenied == error.type()) {
			emit log(tr("You are not allowed to connect to nuts."));
			emit log(tr("Please make sure you are in the correct group"));
		}
		else if (QDBusError::InvalidSignature == error.type()) {
			// Workaround qt returning wrong Error (Should be AccessDenied)
			emit log(tr("Maybe you don't have sufficient rights"));
		}
	}

	CNutServiceClient::CNutServiceClient(CNutService* serviceParent, QObject* parent)
	: QObject(parent), m_serviceParent(serviceParent) {
		connect(m_serviceParent, &CNutService::dbusLostService, this, &CNutServiceClient::dbusLostService);
		connect(this, &CNutServiceClient::dbusError, m_serviceParent, &CNutService::handle_dbusError);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
		QTimer::singleShot(0, this, &CNutServiceClient::initService);
#else
		QTimer::singleShot(0, this, SLOT(initService()));
#endif
	}

	CNutServiceClient::CNutServiceClient(CNutServiceClient* parent)
	: CNutServiceClient(parent->m_serviceParent, parent) {
		connect(this, &CNutServiceClient::log, parent, &CNutServiceClient::contextLog);
	}
	CNutServiceClient::CNutServiceClient(CNutService* service)
	: CNutServiceClient(service, service) {
		connect(this, &CNutServiceClient::log, service, &CNutService::log);
	}

	void CNutServiceClient::initService() {
		connect(m_serviceParent, &CNutService::dbusConnectService, this, &CNutServiceClient::dbusConnectService);
		if (m_serviceParent->m_servicePresent) {
			dbusConnectService(m_serviceParent->m_dbusService, m_serviceParent->m_dbusServiceWatcher->connection());
		}
	}

	void CNutServiceClient::dbusLostService() {
	}

	void CNutServiceClient::dbusConnectService(QString, QDBusConnection) {
	}

	void CNutServiceClient::contextLog(QString message) {
		if (m_logPrefix.isEmpty()) {
			emit log(std::move(message));
		} else {
			emit log(m_logPrefix + message);
		}
	}
}
