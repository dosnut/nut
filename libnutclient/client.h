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

	/** @brief CLog provides a logging facility for the client_exceptions
		
		The Log can be used to save the logging information to a file.
		You can get access to via the printed signal, which is called every time
		the logging functions are invoked.
	*/
	class CLog : public QObject {
		Q_OBJECT
	private:
		QFile file;
		QTextStream outStream;
		bool fileLoggingEnabled;
	public:
		CLog(QObject * parent, QString fileName);
		inline QFile::FileError error() const {
			return file.error();
		}
		inline bool getFileLoggingEnabled() const {
			return fileLoggingEnabled;
		}
		inline void setFileLoggingEnabled(bool isEnabled) {
			fileLoggingEnabled = isEnabled && (file.error() == QFile::NoError);
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
			QDBusConnectionInterface * dbusConnectionInterface;
			QDBusConnection * dbusConnection;
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
		DBusDeviceManagerInterface * dbusDevmgr;
		QHash<QDBusObjectPath, CDevice* > dbusDevices;
		QDBusConnection dbusConnection;
		CLog * log;
		bool nutsstate;
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
		CDeviceList devices;

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
		QDBusObjectPath dbusPath;
		QDBusObjectPath dbusActiveEnvironment;
		QHash<QDBusObjectPath, CEnvironment*> dbusEnvironments;
		CLog * log;
		DBusDeviceInterface * dbusDevice;
		libnutcommon::DeviceConfig dbusConfig;
		bool need_wpa_supplicant;
		void refreshAll();
		void setActiveEnvironment(CEnvironment * env, QDBusObjectPath dbusPath);
		void rebuild(QList<QDBusObjectPath> paths);
		
		//Locking functions;
		bool pendingRemoval;
		int lockCount;
		

	private slots:
		void environmentChangedActive(const QString &newenv);
		void dbusstateChanged(int newState, int oldState);
	public:
		CEnvironmentList environments;
		
		QString name;
		QString essid;
		libnutcommon::DeviceState state;
		libnutcommon::DeviceType type;
		CEnvironment * activeEnvironment;
		libnutwireless::CWpa_Supplicant * wpa_supplicant;
		int index;
		
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
	};
	
	/** @brief The Environment 
	*/
	class CEnvironment : public CLibNut {
		Q_OBJECT
		friend class CDeviceManager;
		friend class CDevice;
		friend class CInterface;
		friend class DBusEnvironmentInterface;
	private:
		//CDevice * parent;
		QDBusObjectPath dbusPath;
		CLog * log;
		QHash<QDBusObjectPath, CInterface *> dbusInterfaces;
		DBusEnvironmentInterface * dbusEnvironment;
		libnutcommon::EnvironmentConfig config;
		libnutcommon::SelectResult selectResult;
		QVector<libnutcommon::SelectResult> selectResults;
		
		void refreshAll();
		void rebuild(const QList<QDBusObjectPath> &paths);
	private slots:
		void dbusstateChanged(bool state);
		void dbusselectResultChanged(libnutcommon::SelectResult result, QVector<libnutcommon::SelectResult> results);
	public:
		QString name;
		CInterfaceList interfaces;
		bool active;
		int index;
		
		CEnvironment(CDevice * parent, QDBusObjectPath dbusPath);
		~CEnvironment();
	public slots:
		void enter();
		libnutcommon::EnvironmentConfig& getConfig();
		libnutcommon::SelectResult& getSelectResult(bool refresh=false);
		QVector<libnutcommon::SelectResult>& getSelectResults(bool refresh=false);
		
	signals:
		void activeChanged(bool active);
		void interfacesUpdated();
		void selectResultsChanged();
	};
	
	class CInterface : public CLibNut {
		Q_OBJECT
		friend class CDeviceManager;
		friend class CDevice;
		friend class CEnvironment;
		friend class DBusInterfaceInterface_IPv4;
	private:
		//CEnvironment * parent;
		QDBusObjectPath dbusPath;
		CLog * log;
		DBusInterfaceInterface_IPv4 * dbusInterface;
		libnutcommon::IPv4Config dbusConfig;
		libnutcommon::IPv4UserConfig userConfig;
		void refreshAll();
	private slots:
		void dbusstateChanged(libnutcommon::InterfaceProperties properties);
	public:
		libnutcommon::InterfaceState state;
		QHostAddress ip;
		QHostAddress netmask;
		QHostAddress gateway;
		QList<QHostAddress> dnsserver;
		int index;
		
		libnutcommon::IPv4UserConfig getUserConfig(bool refresh=false);
		libnutcommon::IPv4Config& getConfig() { return dbusConfig; }

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
