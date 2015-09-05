#ifndef LIBNUTCLIENT_CDEVICE_H
#define LIBNUTCLIENT_CDEVICE_H

#include <QObject>
#include <QList>
#include <QHash>

#include "cnutservice.h"
#include "libnutcommon/common.h"

namespace libnutclientbase {
	class DBusDevice;
}

namespace libnutclient {
	class CDeviceManager;
	class CDevice;
	class CEnvironment;
	class CInterface;
	typedef QList<CEnvironment *> CEnvironmentList;
}

#ifndef NUT_NO_WIRELESS
namespace libnutwireless {
	class CWireless;
}
#endif

namespace libnutclient {
	/** @brief The Device represents a hardware device with its Environments

		The Devices provides information about the state of the hardware device.
		It also manages its environments.
	*/
	class CDevice : public CNutServiceClient {
		Q_OBJECT
	private:
		friend class CDeviceManager;
		friend class CEnvironment;

		CDeviceManager* m_devManager = nullptr;
		libnutclientbase::DBusDevice* m_dbusDevice = nullptr;
		QDBusObjectPath m_dbusPath;

		int m_initCounter = 0;
		bool checkInitDone();
		void checkInitDone(bool previous);

		//Device information
		CEnvironmentList m_environments;
		QHash<QDBusObjectPath, CEnvironment*> m_dbusEnvironments;
		CEnvironment* m_activeEnvironment = nullptr;

		libnutcommon::DeviceProperties m_properties;
		libnutcommon::DeviceConfig m_config;

#ifndef NUT_NO_WIRELESS
		bool m_needWireless = false;
		libnutwireless::CWireless* m_wlAccess = nullptr;
#else
		bool unused_m_needWireless = false;
		void* unused_m_wlAccess = nullptr;
#endif

		int m_index = -1; /* maintained by CDeviceManager */

		void updateLogPrefix();
		void clear();

		/* callbacks from CEnvironment */
		void environmentInitializationFailed(CEnvironment* environment);
		void environmentInitializationCompleted(CEnvironment* environment);

	protected:
		void dbusLostService() override;
		void dbusConnectService(QString service, QDBusConnection connection) override;

	private slots:
		void dbusPropertiesChanged(libnutcommon::DeviceProperties properties);

	public:
		CDevice(CDeviceManager* parent, QDBusObjectPath dbuspath);
		~CDevice();

		int getIndex() const { return m_index; }
		QDBusObjectPath path() const { return m_dbusPath; }

		libnutcommon::DeviceProperties const& getProperties() const { return m_properties; }
		QString getName() const { return m_properties.name; }
		libnutcommon::DeviceType getType() const { return m_properties.type; }
		CEnvironment* getActiveEnvironment() { return m_activeEnvironment; }
		libnutcommon::DeviceState getState() const { return m_properties.state; }
		QString getEssid() const { return m_properties.essid; }
		libnutcommon::MacAddress getMacAddress() const { return m_properties.macAddress; }

		const libnutcommon::DeviceConfig& getConfig() const { return m_config; }

		const CEnvironmentList& getEnvironments() const { return m_environments; }

#ifndef NUT_NO_WIRELESS
		/** If the device has a wpa_supplicant config, this function returns the pointer
			to the wpa_supplicant object; See CWpaSupplicant.
			If no config file is present, the pointer will be null.
		*/
		libnutwireless::CWireless* getWireless() { return m_wlAccess; }
#endif

	public slots:
		void enable();
		void disable();
		void setEnvironment(libnutclient::CEnvironment* environment);

	signals:
		void newDataAvailable();
		void activeEnvironmentChanged(libnutclient::CEnvironment* current, libnutclient::CEnvironment* previous);
		void stateChanged(libnutcommon::DeviceState state);
		void propertiesChanged(libnutcommon::DeviceProperties properties);
		void essidUpdate(QString essid);
		void newWirelessNetworkFound();
	};
}

#endif
