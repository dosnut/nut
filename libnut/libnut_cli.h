#ifndef QNUT_COMMON_H
#define QNUT_COMMON_H

#include <QObject>
#include <QList>
#include <QHostAddress>
/*
Benötigte Informationen:
device liste : /device_name/
    environment_liste
        
*/

namespace qnut {
    class CDeviceManager;

    class CDevice;
    class CEnvironment;
    class CInterface;

    typedef QList<CDevice *> CDeviceList;
    typedef QList<CEnvironment *> CEnvironmentList;
    typedef QList<CInterface *> CInterfaceList;
};

namespace qnut {
    class CDeviceManager : public QObject {
        Q_OBJECT
    public:
        CDeviceList devices;
        
        CDeviceManager(QObject * parent);
    signals:
        void deviceAdded(CDevice * device);
        void deviceRemoved(CDevice * device);
    };

    class CDevice : public QObject {
        Q_OBJECT
    public:
        QString name;
        int activeEnvironment;
        bool enabled;
        CEnvironmentList environments;  //darf nie leer sein
        
        CDevice(QObject * parent);
    public slots:
        void enable();
        void disable();

    signals:
        void environmentChangedActive(int previous);
        void environmentsUpdated(CDevice * device);
    };
    
    class CEnvironment : public QObject {
        Q_OBJECT
    public:
        enum SelectType {user, arp, essid};
        
        struct SelectConfig {
            SelectType type;
            bool useMAC;
            //fillin mac-member here
            QHostAddress arpIP;
            QString essid;
        };
        
        bool active;
        QString name;
        QList<SelectConfig> selectStatements;
        CInterfaceList interfaces;  //darf nie leer sein
        
        CEnvironment(CDevice * parent);
    public slots:
        void activate();
    signals:
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
        void setIP(QHostAddress * address);
        void setNetmask(QHostAddress * address);
        void setGateway(QHostAddress * address);
    signals:
        void activeStateChanged();
    };
};

#endif
