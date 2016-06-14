#ifndef LIBNUTCLIENT_CDEVICEMANAGER_H
#define LIBNUTCLIENT_CDEVICEMANAGER_H

#include <QObject>
#include <QList>
#include <QHash>

#include "cnutservice.h"

namespace libnutclientbase {
	class DBusDeviceManager;
}

namespace libnutclient {
	class CDeviceManager;
	class CDevice;
	class CEnvironment;
	class CInterface;

	typedef QList<CDevice *> CDeviceList;
}

namespace libnutclient {
	/** @brief The DeviceManager keeps track of all devices

		The DeviceManager represents the server's device manager on the client side.

		Device adds/removes are published via deviceAdded/deviceRemoved
	*/
	class CDeviceManager : public CNutServiceClient {
		Q_OBJECT
	private:
		friend class CDevice;

		libnutclientbase::DBusDeviceManager* m_dbusDevmgr = nullptr;
		QHash<QDBusObjectPath, CDevice*> m_dbusDevices;
		CDeviceList m_devices;

		void clear();
		void reload();
		void load(const QDBusObjectPath& path);
		void rebuildIndex();

		/* callbacks from CDevice */
		void deviceInitializationCompleted(CDevice* device);
		void deviceInitializationFailed(CDevice* device);

	private slots:
		//dbus signals
		void dbusDeviceAdded(const QDBusObjectPath &devPath);
		void dbusDeviceRemoved(const QDBusObjectPath &devPath);

	protected:
		void dbusLostService() override;
		void dbusConnectService(QString service, QDBusConnection connection) override;

	public:
		explicit CDeviceManager(CNutService* parent);
		~CDeviceManager();

		const CDeviceList& getDevices() const { return m_devices; }

		bool createBridge(QList<CDevice*> devices);
		bool destroyBridge(CDevice* device);

	public slots:
		void refreshAll();

	signals:
		void deviceAdded(libnutclient::CDevice* device);
		void deviceRemoved(libnutclient::CDevice* device);
	};
}

#endif
