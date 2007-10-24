#ifndef LIBNUT_LIBNUT_CLI_H
#define LIBNUT_LIBNUT_CLI_H
#include <iostream>
#include <QObject>
#include <QList>
#include <QHostAddress>
#include <common/types.h>
#include <common/config.h>
#include "libnut_server_proxy.h"
#include "libnut_exceptions.h"
#include "libnut_wpa_supplicant.h"
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QFile>
#include <QTextStream>
#include <QHash>

namespace libnut {
	class CDeviceManager;

	class CLog;
	class CDevice;
	class CEnvironment;
	class CInterface;

	typedef QList<CDevice *> CDeviceList;
	typedef QList<CEnvironment *> CEnvironmentList;
	typedef QList<CInterface *> CInterfaceList;
};

namespace libnut {

	QString toString(DeviceState state);
	QString toString(DeviceType type);
	QString toString(WlanEncryptionType type);
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
		void init(CLog * inlog);

		CDeviceManager(QObject * parent);
		~CDeviceManager();
	public slots:
		void refreshAll();
		void rebuild();
	signals:
		void deviceAdded(CDevice * device);
		void deviceRemoved(CDevice * device); //nach entfernen aus der liste aber vor dem löschen
	};

	class CDevice : public CLibNut {
		Q_OBJECT
		friend class CDeviceManager;
		friend class CEnvironment;
		friend class CInterface;
		friend class DBusDeviceInterface;
	private:
		CDeviceManager * parent;
		QDBusObjectPath dbusPath;
		QDBusObjectPath dbusActiveEnvironment;
		QHash<QDBusObjectPath, CEnvironment*> dbusEnvironments;
		CLog * log;
		DBusDeviceInterface * dbusDevice;
		nut::DeviceConfig dbusConfig;
		bool need_wpa_supplicant;
		void refreshAll();
		void setActiveEnvironment(CEnvironment * env, QDBusObjectPath dbusPath);
		void rebuild(QList<QDBusObjectPath> paths);

	private slots:
		void environmentChangedActive(const QDBusObjectPath &newenv);
		void environmentAdded(const QDBusObjectPath &path);
		void environmentRemoved(const QDBusObjectPath &path);
		void dbusstateChanged(int newState, int oldState);
	public:
		CEnvironmentList environments;
		
		QString name;
		DeviceState state;
		DeviceType type;
		CEnvironment * activeEnvironment;
		CWpa_Supplicant * wpa_supplicant;
		
		CDevice(CDeviceManager * parent, QDBusObjectPath dbuspath);
		~CDevice();
		nut::DeviceConfig getConfig();

	public slots:
		void enable();
		void disable();
		void addEnvironment(QString name);
		void removeEnvironment(CEnvironment * environment);
		void setEnvironment(CEnvironment * environment);
		
	signals:
		void environmentChangedActive(CEnvironment * current, CEnvironment * previous);
		void environmentsUpdated();
		void environmentAdded(CEnvironment * environment);
		void environmentRemoved(CEnvironment * environment);
		void stateChanged(DeviceState state);
	};
	
	class CEnvironment : public CLibNut {
		Q_OBJECT
		friend class CDeviceManager;
		friend class CDevice;
		friend class CInterface;
		friend class DBusEnvironmentInterface;
	private:
		CDevice * parent;
		QDBusObjectPath dbusPath;
		CLog * log;
		QHash<QDBusObjectPath, CInterface *> dbusInterfaces;
		DBusEnvironmentInterface * dbusEnvironment;
		bool active;
		nut::EnvironmentConfig config;
		
		void refreshAll();
		void rebuild(const QList<QDBusObjectPath> &paths);
	private slots:
		void dbusinterfaceAdded(const QDBusObjectPath &path);
		void dbusinterfaceRemoved(const QDBusObjectPath &path);
		void dbusstateChanged(bool state);
	public:
		QString name;
		CInterfaceList interfaces;
		
		CEnvironment(CDevice * parent, QDBusObjectPath dbusPath);
		~CEnvironment();
	public slots:
		void enter();
		nut::EnvironmentConfig getConfig();
		void addInterface(bool isStatic, QHostAddress ip, QHostAddress netmask, QHostAddress gateway, bool active);
		void removeInterface(CInterface * interface);
		
	signals:
		void activeChanged(bool active);
		void interfacesUpdated();
		void interfaceAdded(CInterface * interface);
		void interfaceRemoved(CInterface * interface);
	};
	
	class CInterface : public CLibNut {
		Q_OBJECT
		friend class CDeviceManager;
		friend class CDevice;
		friend class CEnvironment;
		friend class DBusInterfaceInterface_IPv4;
	private:
		CEnvironment * parent;
		QDBusObjectPath dbusPath;
		CLog * log;
		DBusInterfaceInterface_IPv4 * dbusInterface;
		nut::IPv4Config dbusConfig;
		void refreshAll();
	private slots:
		void dbusstateChanged(const InterfaceProperties &properties);
	public:
		bool isStatic;
		bool active;
		QHostAddress ip;
		QHostAddress netmask;
		QHostAddress gateway;
		QList<QHostAddress> dnsserver;

		nut::IPv4Config getConfig() { return dbusConfig; }
		nut::IPv4Config config() { return dbusConfig; } //wants qnut wants it like that

		CInterface(CEnvironment * parent, QDBusObjectPath dbusPath);
		~CInterface();
	public slots:
		void activate();
		void deactivate();
		void setIP(QHostAddress & address);
		void setNetmask(QHostAddress & address);
		void setGateway(QHostAddress & address);
		
	signals:
		void activeChanged(bool active);
		void ipconfigChanged(bool isStatic, QHostAddress ip, QHostAddress netmask, QHostAddress gateway);
	};
};

#endif
