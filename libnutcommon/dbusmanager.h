#ifndef NUT_COMMON_DBUSMANAGER_H
#define NUT_COMMON_DBUSMANAGER_H

#pragma once

#include <QObject>
#include <QBasicTimer>
#include <QDBusConnection>
#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

QT_BEGIN_NAMESPACE
class QDBusConnectionInterface;
QT_END_NAMESPACE

#include <functional>
#include <memory>
#include <list>
#include <map>

namespace libnutcommon {
	class DBusManager;

	/* create a SystemBus connection with an internal name */
	QDBusConnection createDefaultDBusConnection();

	/* QtDBus sucks :(
	 * - no "Disconnect" signal available, need to poll isConnected()
	 * - disconnected connections sometimes leak (blocking the "name" Qt
	 *   requires -- without good reason -- for a connection)
	 *   If a connection can't be deleted after it got disconnected
	 *   DBusManager will print a message and call QCoreApplication::exit(77).
	 *
	 * It looks like it's working with the system(d) dbus...
	 *
	 * Watching a pid-file with inotify is no longer an option to polling;
	 * various init systems don't have a pid file anymore, also there is no
	 * longer a standard location.
	 */
	class DBusManager final : public QObject {
		Q_OBJECT
	public:
		explicit DBusManager(
			std::function<QDBusConnection()> createConnection = createDefaultDBusConnection,
			int checkMsec = 10000,
			int retryMsec = 1000,
			QObject* parent = nullptr);
		~DBusManager();

		/* if there is no connection this is just null.
		 * don't keep the pointer! - just copy the content. */
		QDBusConnection* connection() { return m_connection.get(); }

	public slots:
		void checkConnection();

	signals:
		void connected(QDBusConnection const& connection);
		void disconnected(QDBusConnection const& connection);

		/* only emitted once after each connection loss */
		void waiting();

	private:
		void reconnect();
		void timerEvent(QTimerEvent* event) override;

	private:
		std::unique_ptr<QDBusConnection> m_connection;
		std::function<QDBusConnection()> m_createConnection;

		/* poll interval for QDBusConnection->isConnected() */
		int const m_checkMsec;
		/* interval between reconnect retries if not successful connection could be made */
		int const m_retryMsec;

		/* prevent reentrance */
		bool m_reconnecting = false;

		/* emit waiting() at beginning too if necessary */
		bool m_hadConnection = true;
		QBasicTimer m_checkTimer;
		QBasicTimer m_reconnectTimer;
	};

	class DBusAbstractAdapaterConnectionEmitter : public QObject {
		Q_OBJECT
	public:
		void operator()(QDBusConnection const& connection);

	signals:
		void notify(QDBusConnection const& connection);
	};

	class DBusAbstractAdapater: public QDBusAbstractAdaptor {
		Q_OBJECT
	private:
		std::list<QDBusConnection> m_connections;
		std::map<QString, std::list<QDBusConnection>> m_services;

		DBusAbstractAdapaterConnectionEmitter m_dbusConnected;
		DBusAbstractAdapaterConnectionEmitter m_dbusDisconnected;

		void removeConnections(std::list<QDBusConnection>&& l);

	private slots:
		void _onDbusConnected(QDBusConnection const& connection);
		void _onDbusDisconnected(QDBusConnection const& connection);

	protected:
		QDBusObjectPath const m_path;

		/* usually not needed - parent() is registered at m_path automatically
		 * call unregister() in destructor to receive final onDbusDisconnected()
		 * in your class
		 */
		virtual void onDbusConnected(QDBusConnection& connection);
		virtual void onDbusDisconnected(QDBusConnection& connection);

		/* register your service names in the constructor */
		void registerService(QString const& serviceName);
		/* no need to unregister your service name manually */
		void unregisterService(QString const& serviceName);

		/* after creating a child object register it with this to connect
		 * it to existing dbus connections and receive future notifications
		 *
		 * children registered with this don't need to be connected with
		 * connectManager() on their own.
		 */
		void registerAdaptor(DBusAbstractAdapater* child);

		/* see onDbusDisconnected; usually you don't need to call this */
		void unregisterAll();

	public:
		explicit DBusAbstractAdapater(QDBusObjectPath path, QObject* parent);
		~DBusAbstractAdapater();

		QDBusObjectPath const& getPath() const { return m_path; }

		/* you can connect adapters to many dbus managers; usually you only
		 * connect the top-level adaptor to a manager, and use registerAdaptor
		 * for child objects
		 */
		void connectManager(DBusManager* manager);
	};
}

#endif
