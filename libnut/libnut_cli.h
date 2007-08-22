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
/*
Benötigte Informationen:
device liste : /device_name/
    environment_liste
        
*/

namespace libnut {
    class CDeviceManager;

    class CDevice;
    class CEnvironment;
    class CInterface;

    typedef QList<CDevice *> CDeviceList;
    typedef QList<CEnvironment *> CEnvironmentList;
    typedef QList<CInterface *> CInterfaceList;
};

namespace libnut {
    class CDeviceManager : public QObject {
        Q_OBJECT
    private:
        DBusDeviceManagerInterface * dbusDevmgr;
        QDBusConnectionInterface dbusConnectionInterface;
        QDBusConnection * dbusConnection;
        void serviceCheck();
        QList<QDBusObjectPath> dbusDeviceList;
    public:
        CDeviceList devices;
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
    public:
        CEnvironmentList environments;
        
        QString name;
        bool enabled;
        int type;
        CEnvironment * activeEnvironment;
        
        CDevice(QObject * parent);
    public slots:
        void enable();
        void disable();

    signals:
        void environmentChangedActive(int current, int previous);
        void environmentsUpdated();
        void stateChanged(bool state);
    };
    
    class CEnvironment : public QObject {
        Q_OBJECT
    public:
        bool active;
        QString name;
        QList<libnut_SelectConfig> selectStatements;
        CInterfaceList interfaces;
        
        CEnvironment(CDevice * parent);
    public slots:
        void enter();
    signals:
        void stateChanged(bool state);
        void interfacesUpdated();
    };
    
    class CInterface : public QObject {
        Q_OBJECT
    public:
        bool isStatic;
        bool active;
        bool userDefineable;
        QHostAddress ip;
        QHostAddress netmask;
        QHostAddress gateway;

        CInterface(QObject * parent);
    public slots:
        void activate();
        void deactivate();
        void setIP(QHostAddress & address); //zuvor pointer
        void setNetmask(QHostAddress & address); //zuvor pointer
        void setGateway(QHostAddress & address); //zuvor pointer
        void setStatic(bool state); // war zuvor nicht da
    signals:
        void stateChanged(bool state); //zuvor activeStateChanged()
    };
};

#endif
