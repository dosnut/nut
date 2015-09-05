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

		connect(m_dbusManager, SIGNAL(connected(QDBusConnection const&)), SLOT(dbus_connected(QDBusConnection const&)));
		connect(m_dbusManager, SIGNAL(disconnected(QDBusConnection const&)), SLOT(dbus_disconnected(QDBusConnection const&)));
		connect(m_dbusManager, SIGNAL(waiting()), SLOT(dbus_waiting()));

		connect(m_dbusServiceWatcher, SIGNAL(serviceRegistered(const QString&)), this, SLOT(sw_dbusServiceRegistered()));
		connect(m_dbusServiceWatcher, SIGNAL(serviceUnregistered(const QString&)), this, SLOT(sw_dbusServiceUnregistered()));
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
		connect(m_serviceParent, SIGNAL(dbusLostService()), this, SLOT(dbusLostService()));
		connect(this, SIGNAL(dbusError(QString, QDBusError)), m_serviceParent, SLOT(handle_dbusError(QString, QDBusError)));
		QTimer::singleShot(0, this, SLOT(initService()));
	}

	CNutServiceClient::CNutServiceClient(CNutServiceClient* parent)
	: CNutServiceClient(parent->m_serviceParent, parent) {
		connect(this, SIGNAL(log(QString)), parent, SLOT(contextLog(QString)));
	}
	CNutServiceClient::CNutServiceClient(CNutService* service)
	: CNutServiceClient(service, service) {
		connect(this, SIGNAL(log(QString)), service, SIGNAL(log(QString)));
	}

	void CNutServiceClient::initService() {
		connect(m_serviceParent, SIGNAL(dbusConnectService(QString, QDBusConnection)), this, SLOT(dbusConnectService(QString, QDBusConnection)));
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
