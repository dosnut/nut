#include "dbusmanager.h"

#include <QCoreApplication>
#include <QDBusConnectionInterface>
#include <QTextStream>
#include <QTimerEvent>

namespace libnutcommon {

	namespace {
		QString const DBusConnectionName { "libnutclient_system_bus" };

		/* exits the application if disconnect didn't work */
		/* QT BUG: if the connection becomes invalid (d->mode = InvalidMode),
		 * for example when the dbus server closes the connection,
		 * QDBusConnection::disconnectFromBus() doesn't call
		 * QDBusConnectionManager::removeConnection(), leaking the internal connection
		 * and blocking the name.
		 */
		void disconnectDBusName(QString const& name) {
			/* disconnect should remove the name -> bus association */
			QDBusConnection::disconnectFromBus(name);
			QDBusConnection c(name);
			/* if the name -> bus association was removed, the new bus doesn't
			 * have a "private" part, and interface() is null */
			if (c.interface()) {
				QTextStream(stderr) << "QtDBus can't remove the DBusConnection for name '" << name << "', and establishing a new one with this name is therefore not possible; using a new name would leak the memory. Terminating.\n";
				QCoreApplication::exit(77);
			}
		}

		QDBusConnection defaultConnection(QString{});
	}

	QDBusConnection createDefaultDBusConnection() {
		if (!defaultConnection.isConnected()) {
			defaultConnection = QDBusConnection::connectToBus(QDBusConnection::SystemBus, DBusConnectionName);
		}
		return defaultConnection;
	}

	DBusManager::DBusManager(std::function<QDBusConnection()> createConnection, int checkMsec, int retryMsec, QObject* parent)
	: QObject(parent), m_createConnection(createConnection), m_checkMsec(checkMsec), m_retryMsec(retryMsec) {
		m_reconnectTimerId = startTimer(0);
	}

	DBusManager::~DBusManager() {
		if (m_connection) {
			auto c = std::move(m_connection);
			// QTextStream(stderr) << "emit disconnected\n";
			emit disconnected(*c);
		}
	}

	void DBusManager::reconnect() {
		if (m_reconnecting) return;
		m_reconnecting = true;

		if (-1 != m_checkTimerId) {
			killTimer(m_checkTimerId);
			m_checkTimerId = -1;
		}

		if (-1 != m_reconnectTimerId) {
			killTimer(m_reconnectTimerId);
			m_reconnectTimerId = -1;
		}

		if (m_connection) {
			auto c = std::move(m_connection);
			// QTextStream(stderr) << "emit disconnected\n";
			emit disconnected(*c);
		}

		// QTextStream(stderr) << "connecting...\n";
		decltype(m_connection) c { new QDBusConnection(m_createConnection()) };
		// unlink name immediately - we keep the connection alive with an explicit reference
		disconnectDBusName(c->name());

		if (!c->isConnected()) {
			c.reset();

			// QTextStream(stderr) << "connect failed\n";
			m_reconnectTimerId = startTimer(m_retryMsec);
			if (m_hadConnection) {
				m_hadConnection = false;
				// QTextStream(stderr) << "emit waiting\n";
				emit waiting();
			}
			m_reconnecting = false;
			return;
		}

		m_checkTimerId = startTimer(m_checkMsec);

		m_connection = std::move(c);
		// QTextStream(stderr) << "emit connected\n";
		emit connected(*m_connection);

		m_reconnecting = false;
	}

	void DBusManager::checkConnection() {
		if (!m_connection || !m_connection->isConnected()) {
			// QTextStream(stderr) << "not connected anymore\n";
			reconnect();
		}
	}

	void DBusManager::timerEvent(QTimerEvent* event) {
		if (event->timerId() == m_reconnectTimerId) {
			reconnect();
		}
		else if (event->timerId() == m_checkTimerId) {
			checkConnection();
		}
		else {
			QObject::timerEvent(event);
		}
	}

	DBusAbstractAdapater::DBusAbstractAdapater(QDBusObjectPath path, QObject* parent)
	: QDBusAbstractAdaptor(parent), m_path(std::move(path)) {
	}

	DBusAbstractAdapater::~DBusAbstractAdapater() {
		unregisterAll();
	}

	void DBusAbstractAdapater::removeConnections(std::list<QDBusConnection>&& l) {
		for (auto& c: l) {
			for (auto si = begin(m_services), se = end(m_services); si != se; ++si) {
				for (auto sci = begin(si->second), sce = end(si->second); sci != sce; ++sci) {
					if (c.interface() == sci->interface()) {
						c.unregisterService(si->first);
						sci = si->second.erase(sci);
					}
				}
				if (si->second.empty()) {
					si = m_services.erase(si);
				}
			}
		}

		for (auto& c: l) {
			c.unregisterObject(m_path.path());
			onDbusDisconnected(c);
			m_dbusDisconnected(c);
		}
	}

	void DBusAbstractAdapater::_onDbusConnected(QDBusConnection const& connection) {
		// remove dead connections
		bool found = false;
		auto const ci = connection.interface();
		std::list<QDBusConnection> removeCons;
		for (auto i = begin(m_connections), e = end(m_connections); i != e; ++i) {
			found = found || (ci == i->interface());
			if (!i->isConnected()) {
				removeCons.push_back(*i);
				i = m_connections.erase(i);
			}
		}
		removeConnections(std::move(removeCons));

		if (found) return;

		auto c = connection; // registerService() doesn't work on const& -.-

		/* only add connection if we could register our own object */
		if (!c.registerObject(m_path.path(), parent())) return;

		for (auto s: m_services) {
			if (c.registerService(s.first)) s.second.push_back(c);
		}

		onDbusConnected(c);
		m_dbusConnected(connection);
	}

	void DBusAbstractAdapater::_onDbusDisconnected(QDBusConnection const& connection) {
		// remove all matching and dead connections
		auto ci = connection.interface();
		std::list<QDBusConnection> removeCons;
		for (auto i = begin(m_connections), e = end(m_connections); i != e; ++i) {
			if (ci == i->interface() || !i->isConnected()) {
				removeCons.push_back(*i);
				i = m_connections.erase(i);
			}
		}
		removeConnections(std::move(removeCons));
	}

	void DBusAbstractAdapater::onDbusConnected(QDBusConnection& connection) {
	}

	void DBusAbstractAdapater::onDbusDisconnected(QDBusConnection& connection) {
	}

	void DBusAbstractAdapater::registerAdaptor(DBusAbstractAdapater* child) {
		connect(&m_dbusConnected, &DBusAbstractAdapaterConnectionEmitter::notify, child, &DBusAbstractAdapater::_onDbusConnected);
		connect(&m_dbusDisconnected, &DBusAbstractAdapaterConnectionEmitter::notify, child, &DBusAbstractAdapater::_onDbusDisconnected);
		for (auto &c: m_connections) {
			child->_onDbusConnected(c);
		}
	}

	void DBusAbstractAdapater::unregisterAll() {
		using std::swap;
		{
			decltype(m_connections) tmp;
			swap(tmp, m_connections);
			removeConnections(std::move(tmp));
		}

		{
			// removeConnections should already have emptied this, but make sure
			decltype(m_services) tmp;
			swap(tmp, m_services);
			for (auto& s: tmp) {
				for (auto& c: s.second) {
					c.unregisterService(s.first);
				}
			}
		}
	}

	void DBusAbstractAdapater::registerService(QString const& serviceName) {
		/* if service name was already registered, c.registerService() will
		 * fail for connections it already succeeded, so we don't get any
		 * duplicates in the m_services[] connection list.
		 */
		auto& cons = m_services[serviceName];
		for (auto &c: m_connections) {
			if (c.registerService(serviceName)) cons.push_back(c);
		}
	}

	void DBusAbstractAdapater::unregisterService(QString const& serviceName) {
		auto i = m_services.find(serviceName);
		if (m_services.end() == i) return;
		for (auto &c: i->second) {
			c.unregisterService(i->first);
		}
		m_services.erase(i);
	}

	void DBusAbstractAdapater::connectManager(DBusManager* manager) {
		connect(manager, &DBusManager::connected, this, &DBusAbstractAdapater::_onDbusConnected);
		connect(manager, &DBusManager::disconnected, this, &DBusAbstractAdapater::_onDbusDisconnected);
	}

	void DBusAbstractAdapaterConnectionEmitter::operator()(const QDBusConnection& connection)
	{
		emit notify(connection);
	}
}
