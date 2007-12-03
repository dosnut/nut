#ifndef LIBNUTCLIENT_CLIENT_H
#define LIBNUTCLIENT_CLIENT_H
#include <QObject>
#include <QList>
#include <QHostAddress>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QFile>
#include <QTextStream>
#include "libnutcommon/common.h"
#include <libnutwireless/wpa_supplicant.h>
#include "client_exceptions.h"

namespace libnutclient {
	class CDeviceManager;

	class CLog;
	class CDevice;
	class CEnvironment;
	class CInterface;

	class DBusDeviceManagerInterface;
	class DBusDeviceInterface;
	class DBusEnvironmentInterface;
	class DBusInterfaceInterface_IPv4;

	typedef QList<CDevice *> CDeviceList;
	typedef QList<CEnvironment *> CEnvironmentList;
	typedef QList<CInterface *> CInterfaceList;
};

namespace libnutclient {

	QString toStringTr(libnutcommon::DeviceState state);
	QString toStringTr(libnutcommon::DeviceType type);
	QString toStringTr(QDBusError error);
	QString toStringTr(libnutcommon::InterfaceState state);

	/** @brief CLog provides a logging facility for the client_exceptions
		
		The Log can be used to save the logging information to a file.
		You can get access to via the printed signal, which is called every time
		the logging functions are invoked.
	*/
	class CLog : public QObject {
		Q_OBJECT
	private:
		QFile m_file;
		QTextStream m_outStream;
		bool m_fileLoggingEnabled;
	public:
		CLog(QObject * parent, QString fileName);
		inline QFile::FileError error() const {
			return m_file.error();
		}
		inline bool getFileLoggingEnabled() const {
			return m_fileLoggingEnabled;
		}
		inline void setFileLoggingEnabled(bool isEnabled) {
			m_fileLoggingEnabled = isEnabled && (m_file.error() == QFile::NoError);
		}
		void operator<<(QString text);
	public slots:
		void log(QString text) { operator<<(text); }
	signals:
		void printed(const QString & line);
	};


	/** @brief CLibNut is the base class for all libnutclient classes
		
		The class provides very basic functions and members that all derived classes have in common
	*/
	class CLibNut : public QObject {
		Q_OBJECT
		protected:
			QDBusConnectionInterface * m_dbusConnectionInterface;
			QDBusConnection * m_dbusConnection;
			void serviceCheck(QDBusConnectionInterface * interface);
		public:
			CLibNut(QObject * parent) : QObject(parent) {}

	};

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
	private:
		DBusDeviceManagerInterface * m_dbusDevmgr;
		QHash<QDBusObjectPath, CDevice* > m_dbusDevices;
		QDBusConnection m_dbusConnection;
		CLog * log;
		bool m_nutsstate;
		CDeviceList m_devices;
		void rebuild(QList<QDBusObjectPath> paths);
		void setInformation();
		void clearInformation();
	private slots:
		void dbusDeviceAdded(const QDBusObjectPath &objectpath);
		void dbusDeviceRemoved(const QDBusObjectPath &objectpath);
		void dbusServiceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
	public:
		/** @brief List of devices managed by the DeviceManager
		*/
		inline CDeviceList& getDevices() { return m_devices; } //TODO:change to const


		/** @brief Init function to initialize
			It has to be called to start the device manager
		*/
		bool init(CLog * inlog);

		CDeviceManager(QObject * parent);
		~CDeviceManager();
	public slots:
		void refreshAll();
		void rebuild();
	signals:
		void deviceAdded(libnutclient::CDevice * device);
		void deviceRemoved(libnutclient::CDevice * device); //nach entfernen aus der liste aber vor dem löschen
		void stateChanged(bool state); //Information about server state
	};

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
		QDBusObjectPath m_dbusActiveEnvironment;
		QHash<QDBusObjectPath, CEnvironment*> m_dbusEnvironments;
		CLog * log;
		DBusDeviceInterface * m_dbusDevice;
		libnutcommon::DeviceConfig m_config;
		bool m_needWpaSupplicant;
		void refreshAll();
		void setActiveEnvironment(CEnvironment * env, QDBusObjectPath m_dbusPath);
		void rebuild(QList<QDBusObjectPath> paths);
		
		//Locking functions;
		bool m_pendingRemoval;
		int m_lockCount;
		
		//Device information
		CEnvironmentList m_environments;
		QString m_name;
		QString m_essid;
		libnutcommon::DeviceState m_state;
		libnutcommon::DeviceType m_type;
		CEnvironment * m_activeEnvironment;
		libnutwireless::CWpa_Supplicant * m_wpaSupplicant;
		int m_index;

	private slots:
		void environmentChangedActive(const QString &newenv);
		void dbusStateChanged(int newState, int oldState);
	public:
		inline const CEnvironmentList& getEnvironments() { return m_environments; }
		inline const QString& getName() { return m_name; }
		inline const QString& getEssid() { return m_essid; }
		inline libnutcommon::DeviceState getState() { return m_state; }
		inline libnutcommon::DeviceType getType() { return m_type; }
		inline CEnvironment * getActiveEnvironment() { return m_activeEnvironment; }
		inline libnutwireless::CWpa_Supplicant * getWpaSupplicant() { return m_wpaSupplicant; }
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
		void environmentChangedActive(libnutclient::CEnvironment * current, libnutclient::CEnvironment * previous);
		void stateChanged(libnutcommon::DeviceState newState);
		void newWirelessNetworkFound();
	};
	
	/** @brief The Environment manages the interfaces
		
		The Environment manages its associated interfaces. It also provides information about the Environment (name, select results, state).
		It reports state changes via 3 signals

	*/
	class CEnvironment : public CLibNut {
		Q_OBJECT
		friend class CDeviceManager;
		friend class CDevice;
		friend class CInterface;
		friend class DBusEnvironmentInterface;
	private:
		//CDevice * parent;
		QDBusObjectPath m_dbusPath;
		CLog * log;
		QHash<QDBusObjectPath, CInterface *> m_dbusInterfaces;
		DBusEnvironmentInterface * m_dbusEnvironment;
		libnutcommon::EnvironmentConfig m_config;
		libnutcommon::SelectResult m_selectResult;
		QVector<libnutcommon::SelectResult> m_selectResults;

		QString m_name;
		CInterfaceList m_interfaces;
		bool m_state;
		int m_index;
		
		void refreshAll();
		void rebuild(const QList<QDBusObjectPath> &paths);
	private slots:
		void dbusStateChanged(bool state);
		void dbusSelectResultChanged(libnutcommon::SelectResult result, QVector<libnutcommon::SelectResult> results);
	public:

		inline const QString& getName() { return m_name; }
		inline const CInterfaceList& getInterfaces() { return m_interfaces;}
		inline bool getState() { return m_state; }
		inline int getIndex() { return m_index;}
		
		CEnvironment(CDevice * parent, QDBusObjectPath dbusPath);
		~CEnvironment();
	public slots:
		void enter();
		libnutcommon::EnvironmentConfig& getConfig();
		libnutcommon::SelectResult& getSelectResult(bool refresh=false);
		QVector<libnutcommon::SelectResult>& getSelectResults(bool refresh=false);
		
		/** @brief Environment signals
		
			activeChanged(bool active) is emitted when environment get's activated or deactivated
			interfacesUpdated() is emitted when interfaces are added or removed
			selectResultsChanged() is emitted when select results changed (this normaly hapens, when they're completly done)
			
		*/
	signals:
		void activeChanged(bool active);
		void interfacesUpdated();
		void selectResultsChanged();
	};

	/** @brief The Interface represents one network interface
		
		It provides information about its configuration (ip,netmask,gateway,dnsservers,state).
		There's also a function to set an own interface configuration
		It also emits a signal on data changes.
		
	*/
	class CInterface : public CLibNut {
		Q_OBJECT
		friend class CDeviceManager;
		friend class CDevice;
		friend class CEnvironment;
		friend class DBusInterfaceInterface_IPv4;
	private:
		//CEnvironment * parent;
		QDBusObjectPath m_dbusPath;
		CLog * log;
		DBusInterfaceInterface_IPv4 * m_dbusInterface;
		libnutcommon::IPv4Config m_config;
		libnutcommon::IPv4UserConfig m_userConfig;
		libnutcommon::InterfaceState m_state;
		QHostAddress m_ip;
		QHostAddress m_netmask;
		QHostAddress m_gateway;
		QList<QHostAddress> m_dnsservers;
		int m_index;
		void refreshAll();
	private slots:
		void dbusStateChanged(libnutcommon::InterfaceProperties properties);
	public:
		inline libnutcommon::InterfaceState getState() { return m_state; }
		inline const QHostAddress& getIp() { return m_ip;}
		inline const QHostAddress& getNetmask() { return m_netmask; }
		inline const QHostAddress& getGateway() { return m_gateway; }
		inline const QList<QHostAddress>& getDnsServers() { return m_dnsservers; }
		inline int getIndex() { return m_index;}
		
		libnutcommon::IPv4UserConfig getUserConfig(bool refresh=false);
		libnutcommon::IPv4Config& getConfig() { return m_config; }

		CInterface(CEnvironment * parent, QDBusObjectPath dbusPath);
		~CInterface();
	public slots:
		void activate();
		void deactivate();
		bool needUserSetup();
		bool setUserConfig(const libnutcommon::IPv4UserConfig &cuserConfig);
		
	signals:
		void stateChanged(libnutcommon::InterfaceState state);
	};
};
#include "server_proxy.h"
#endif
