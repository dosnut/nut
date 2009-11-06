#ifndef LIBNUTCLIENT_CDEVICEMANAGER_H
#define LIBNUTCLIENT_CDEVICEMANAGER_H

#include <QObject>
#include <QList>
#include <QHash>
#include <QDBusMessage>

#include "libnutcommon/dbusmonitor.h"
#include "clibnut.h"

namespace libnutclient {
	class CLog;
	class DBusDeviceManagerInterface;
	class CDeviceManager;
	class CDevice;
	class CEnvironment;
	class CInterface;

	typedef QList<CDevice *> CDeviceList;
}

namespace libnutclient {
	/** @brief The DeviceManager keeps track of all devices

		The DeviceManager represents the server's device manager on the client side.
		It handles the dbus connections, server disconnect/connect events and device adds/removes.

		Server disconnect/connects are published via the stateChanged signal.
		Device adds/removes are published via deviceAdded/deviceRemoved

		After creating a new CDeviceManager init() has to be called for associating
		to DBus.
	*/
	class CDeviceManager : public CLibNut {
		Q_OBJECT
		friend class CDevice;
		friend class CEnvironment;
		friend class CInterface;
		friend class DBusDeviceManagerInterface;
// 		friend class QDBusConnection; //neseccary for qdbuscalls?
	private:
		QDBusConnection m_dbusGlobalConnection;
		DBusDeviceManagerInterface * m_dbusDevmgr;
		QHash<QDBusObjectPath, CDevice* > m_dbusDevices;
		CLog * log;
		bool m_nutsstate;
		int m_dbusTimerId;
		CDeviceList m_devices;
		qint32 m_dbusCallTimeout;
		QString m_dbusPath;

		QDBusMessage devMessage;

		libnutcommon::CDBusMonitor m_dbusMonitor;

		QDBusInterface * m_dbusInterface;
		void rebuild(QList<QDBusObjectPath> paths);
		void setInformation();
		void clearInformation();
		void timerEvent(QTimerEvent *event);
		void dbusKilled(bool doinit=true);

	private slots:
		//dbus return methods
		void dbusretGetDeviceList(QList<QDBusObjectPath> devices);

		void dbusretErrorOccured(QDBusError error, QString method=QString());

		void dbusDeviceAdded(const QDBusObjectPath &objectpath);
		void dbusDeviceRemoved(const QDBusObjectPath &objectpath);
		void dbusServiceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
		void dbusStopped();
		void dbusStarted();

		void deviceInitializationFailed(CDevice * device);
		void deviceInitializationCompleted(CDevice * device);

		void globalDBusErrorOccured(QDBusError error = QDBusError());

	public:
		/** @brief List of devices managed by the DeviceManager
		*/
		inline const CDeviceList& getDevices() { return m_devices; } //TODO:change to const

		/** @brief Returns the pointer to the libnutcommon::CDBusMonitor object
		*/
		inline const libnutcommon::CDBusMonitor * getDBusMonitor() { return &m_dbusMonitor; }

		/** @brief Init function to initialize
			It has to be called to start the device manager
		*/
		bool init(CLog * inlog);

		bool createBridge(QList<CDevice *> devices);
		bool destroyBridge(CDevice * device);

		CDeviceManager(QObject * parent);
		~CDeviceManager();
	public slots:
		void refreshAll();
// 		void rebuild();
	signals:
		void deviceAdded(libnutclient::CDevice * device);
		void deviceRemoved(libnutclient::CDevice * device); //nach entfernen aus der liste aber vor dem löschen
		void stateChanged(bool state); //Information about server state
	};
}

#endif
