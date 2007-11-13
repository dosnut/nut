#ifndef LIBNUT_LIBNUT_CLI_H
#define LIBNUT_LIBNUT_CLI_H
#include <QObject>
#include <QList>
#include <QHostAddress>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QFile>
#include <QTextStream>
#include <libnutcommon/types.h>
#include <libnutcommon/config.h>
#include "libnut_server_proxy.h"
#include "libnut_client_exceptions.h"
#include <libnutwireless/libnut_wpa_supplicant.h>

namespace libnutclient {
	class CDeviceManager;

	class CLog;
	class CDevice;
	class CEnvironment;
	class CInterface;

	typedef QList<CDevice *> CDeviceList;
	typedef QList<CEnvironment *> CEnvironmentList;
	typedef QList<CInterface *> CInterfaceList;
};

namespace libnutclient {

	QString toString(libnutcommon::DeviceState state);
	QString toString(libnutcommon::DeviceType type);
	QString toString(QDBusError error);

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

	class CLibNut : public QObject {
		Q_OBJECT
		protected:
			QDBusConnectionInterface * dbusConnectionInterface;
			QDBusConnection * dbusConnection;
			void serviceCheck(QDBusConnectionInterface * interface);
		public:
			CLibNut(QObject * parent) : QObject(parent) {}

	};

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
		CDeviceList devices;
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
		libnutcommon::DeviceConfig getConfig();

	public slots:
		void enable();
		void disable();
		void setEnvironment(CEnvironment * environment);
		
	signals:
		void environmentChangedActive(libnutclient::CEnvironment * current, libnutclient::CEnvironment * previous);
// 		void environmentsUpdated(); //Pending for removal
// 		void environmentAdded(CEnvironment * environment); //Pending for removal
// 		void environmentRemoved(CEnvironment * environment); //Pending for removal
		void stateChanged(libnutcommon::DeviceState newState);
	};
	
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
	public:
		QString name;
		CInterfaceList interfaces;
		bool active;
		int index;
		
		CEnvironment(CDevice * parent, QDBusObjectPath dbusPath);
		~CEnvironment();
	public slots:
		void enter();
		libnutcommon::EnvironmentConfig getConfig();
		libnutcommon::SelectResult getSelectResult(bool refresh=false);
		QVector<libnutcommon::SelectResult> getSelectResults(bool refresh=false);
		
	signals:
		void activeChanged(bool active);
		void interfacesUpdated();
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
		libnutcommon::IPv4Config getConfig() { return dbusConfig; }
		libnutcommon::IPv4Config config() { return dbusConfig; } //wants qnut wants it like that

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

#endif
