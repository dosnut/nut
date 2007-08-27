#ifndef LIBNUT_LIBNUT_CLI_H
#define LIBNUT_LIBNUT_CLI_H

#include <QObject>
#include <QList>
#include <QHostAddress>
#include "libnut_types.h"
#include "libnut_server_proxy.h"
#include "libnut_exceptions.h"
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QFile>
#include <QTextStream>
/*
Benötigte Informationen:
device liste : /device_name/
    environment_liste
        
*/

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
    signals:
        void printed(const QString & line);
    };

    class CDeviceManager : public QObject {
        Q_OBJECT
        friend class CDevice;
        friend class CEnvironment;
        friend class CInterface;
    private:
        DBusDeviceManagerInterface * dbusDevmgr;
        QDBusConnectionInterface * dbusConnectionInterface;
        QDBusConnection dbusConnection;
        void serviceCheck(QDBusConnectionInterface * dbusConnectionInterface);
        QList<QDBusObjectPath> dbusDeviceList;
    public:
        CDeviceList devices;
        
        void init(CLog * log);
        
        CDeviceManager(QObject * parent);
        ~CDeviceManager();
    public slots:
        void refreshAll();
    signals:
        void deviceAdded(CDevice * device);
        void deviceRemoved(CDevice * device); //nach entfernen aus der liste aber vor dem löschen
    };

    class CDevice : public QObject {
        Q_OBJECT
        friend class CDeviceManager;
        friend class CEnvironment;
        friend class CInterface;
    private:
        void refreshAll();
    public:
        CEnvironmentList environments;
        
        QString name;
        bool enabled;
        int type;
        CEnvironment * activeEnvironment;
        
        CDevice(CDeviceManager * parent, QDBusObjectPath dbuspath);
        ~CDevice();

    public slots:
        void enable();
        void disable();
        CEnvironment * addEnvironment(QString name);
        void removeEnvironment(CEnvironment * environment);
        
    signals:
        void environmentChangedActive(CEnvironment * current, CEnvironment * previous);
        void environmentsUpdated();
        void enabledChanged(bool enabed);
    };
    
    class CEnvironment : public QObject {
        Q_OBJECT
        friend class CDeviceManager;
        friend class CDevice;
        friend class CInterface;
    public:
        bool active;
        QString name;
        QList<libnut_SelectConfig> selectStatements;
        CInterfaceList interfaces;
        
        CEnvironment(CDevice * parent);
        ~CEnvironment();
    public slots:
        void enter();
        void addInterface(bool isStatic, QHostAddress ip, QHostAddress netmask, QHostAddress gateway);
        void removeInterface(CInterface * interface);
        
    signals:
        void activeChanged(bool active);
        void interfacesUpdated();
    };
    
    class CInterface : public QObject {
        Q_OBJECT
        friend class CDeviceManager;
        friend class CDevice;
        friend class CEnvironment;
    public:
        bool isStatic;
        bool active;
        bool userDefineable;
        QHostAddress ip;
        QHostAddress netmask;
        QHostAddress gateway;

        CInterface(CEnvironment * parent);
        ~CInterface();
    public slots:
        void activate();
        void deactivate();
        void setIP(QHostAddress & address); //zuvor pointer
        void setNetmask(QHostAddress & address); //zuvor pointer
        void setGateway(QHostAddress & address); //zuvor pointer
        void setStatic(bool state); // war zuvor nicht da
        
    signals:
        void activeChanged(bool active); //zuvor activeStateChanged()
        void ipconfigChanged(bool isStatic, QHostAddress ip, QHostAddress netmask, QHostAddress gateway);
    };
};

#endif
