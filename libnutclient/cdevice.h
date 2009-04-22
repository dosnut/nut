#ifndef LIBNUTCLIENT_CDEVICE_H
#define LIBNUTCLIENT_CDEVICE_H

#include <QObject>
#include <QList>

#include "clibnut.h"

namespace libnutclient {
	class CLog;
	class DBusDeviceInterface;
	class CDeviceManager;
	class CDevice;
	class CEnvironment;
	class CInterface;
	typedef QList<CEnvironment *> CEnvironmentList;
}

namespace libnutwireless {
	class CWpaSupplicant;
}

namespace libnutclient {
	/** @brief The Device represents a hardware device with its Environments

		The Devices provides information about the state of the hardware device.
		It also manages its environments.
		
		Events are emitted on an environment change and a state change of the device
	*/
	class CDevice : public CLibNut {
		Q_OBJECT
		friend class CDeviceManager;
		friend class CEnvironment;
		friend class CInterface;
		friend class DBusDeviceInterface;
	private:
		//CDeviceManager * parent;
		QDBusObjectPath m_dbusPath;
		QHash<QDBusObjectPath, CEnvironment*> m_dbusEnvironments;
		CLog * log;
		DBusDeviceInterface * m_dbusDevice;
		libnutcommon::DeviceConfig m_config;
		#ifndef LIBNUT_NO_WIRELESS
		bool m_needWpaSupplicant;
		#endif
		
		//Locking functions;
		bool m_pendingRemoval;
		int m_lockCount;

		//device init variables
		bool m_propertiesFetched;
		bool m_environmentsFetched;
		bool m_essidFetched;
		bool m_configFetched;
		bool m_activeEnvFetched;
		bool m_initCompleted;
		
		//Device information
		CEnvironmentList m_environments;
		QString m_name;
		QString m_essid;
		libnutcommon::DeviceState m_state;
		libnutcommon::DeviceType m_type;
		CEnvironment * m_activeEnvironment;
		#ifndef LIBNUT_NO_WIRELESS
		libnutwireless::CWpaSupplicant * m_wpaSupplicant;
		#endif
		int m_index;

	//private  methods
		void refreshAll();
		void setActiveEnvironment(CEnvironment * env, QDBusObjectPath m_dbusPath);
		void rebuild(QList<QDBusObjectPath> paths);
		void checkInitCompleted();


	private slots:

		void environmentChangedActive(const QString &newenv);
		void dbusStateChanged(int newState, int oldState);

		void dbusretGetProperties(libnutcommon::DeviceProperties props);
		void dbusretGetEssid(QString essid);
		void dbusretGetEnvironments(QList<QDBusObjectPath> envs);
		void dbusretGetActiveEnvironment(QString activeEnv);
		void dbusretGetConfig(libnutcommon::DeviceConfig config);
		
		void dbusret_errorOccured(QDBusError error, QString method = QString());

		void environmentInitializationFailed(CEnvironment * environment);
		void environmentInitializationCompleted(CEnvironment * environment);
	public:
		//Initializes this device
		void init();
		inline const CEnvironmentList& getEnvironments() { return m_environments; }
		inline const QString& getName() { return m_name; }
		inline const QString& getEssid() { return m_essid; }
		inline libnutcommon::DeviceState getState() { return m_state; }
		inline libnutcommon::DeviceType getType() { return m_type; }
		inline CEnvironment * getActiveEnvironment() { return m_activeEnvironment; }
		/** If the device has a wpa_supplicant config, this function returns the pointer
			to the wpa_supplicant object; See CWpaSupplicant.
			If no config file is present, the pointer will be null.
		*/
		#ifndef LIBNUT_NO_WIRELESS
		inline libnutwireless::CWpaSupplicant * getWpaSupplicant() { return m_wpaSupplicant; }
		#endif
		inline int getIndex() { return m_index; }
		
		CDevice(CDeviceManager * parent, QDBusObjectPath dbuspath);
		~CDevice();
		libnutcommon::DeviceConfig& getConfig();
		bool incrementLock();
		void decrementLock();

	public slots:
		void enable();
		void disable();
		void setEnvironment(CEnvironment * environment);
		
	signals:

		void initializationFailed(CDevice * device); //TODO:Implement this: has to be called if init fails
		void initializationCompleted(CDevice * device);


		void newDataAvailable();
		void environmentChangedActive(libnutclient::CEnvironment * current, libnutclient::CEnvironment * previous);
		void stateChanged(libnutcommon::DeviceState newState);
		void newWirelessNetworkFound();
		void gotProperties(libnutcommon::DeviceProperties properties);
		void gotEssid(QString essid);
		void gotActiveEnvironment(libnutclient::CEnvironment * activeEnv);
		void gotConfig(libnutcommon::DeviceConfig config);
		void gotEnvironments();
		
	};

// #include "server_proxy.h"
// #endif
}

#endif
