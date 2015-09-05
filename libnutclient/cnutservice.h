#ifndef LIBNUTCLIENT_CNUTSERVICE_H
#define LIBNUTCLIENT_CNUTSERVICE_H

#include <QObject>
#include <QDBusError>
#include <QDBusObjectPath>
#include <QDBusConnection>

QT_BEGIN_NAMESPACE
class QDBusServiceWatcher;
QT_END_NAMESPACE

namespace libnutcommon {
	class DBusManager;
}

namespace libnutclient {
	class CNutService: public QObject {
		Q_OBJECT

	private:
		QString m_dbusService;
		QDBusServiceWatcher* m_dbusServiceWatcher = nullptr;
		libnutcommon::DBusManager* m_dbusManager = nullptr;

		bool m_servicePresent = false; //! whether nut service is present on the bus

		friend class CNutServiceClient;

	public:
		CNutService(QString service, QObject* parent = nullptr);
		CNutService(QObject* parent = nullptr);

		bool isConnected();

	private slots:
		void dbus_connected(QDBusConnection const& connection);
		void dbus_disconnected(QDBusConnection const& connection);
		void dbus_waiting();

		void sw_dbusServiceRegistered();
		void sw_dbusServiceUnregistered();
		void handle_dbusError(QString method, QDBusError error);

	signals:
		void log(QString message);
		void dbusError(QString method, QDBusError error);

		/* internal signals to CNutServiceClient instances */
		void dbusLostService();
		void dbusConnectService(QString service, QDBusConnection connection);
	};

	class CNutServiceClient: public QObject {
		Q_OBJECT
	private:
		CNutService* m_serviceParent;

		CNutServiceClient(CNutService* serviceParent, QObject* parent);
	public:
		CNutServiceClient(CNutServiceClient* parent);
		CNutServiceClient(CNutService* service);

	protected:
		QString m_logPrefix;

	private slots:
		/* automatically deferred init after construction; connects to
		 * m_serviceParent and calls dbusConnectService() if service is
		 * available
		 */
		void initService();

	protected slots:
		virtual void dbusLostService();
		virtual void dbusConnectService(QString service, QDBusConnection connection);
		void contextLog(QString message);

	signals:
		void log(QString message);
		void dbusError(QString method, QDBusError error);
	};
}

#endif
