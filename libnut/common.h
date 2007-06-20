#ifndef QNUT_COMMON_H
#define QNUT_COMMON_H

#include <QObject>
#include <QList>
#include <QHostAddress>

namespace qnut {
    class CDeviceManager;

    class CDevice;
    class CEnvironment;

    typedef QList<CDevice> CDeviceList;
    typedef QList<CEnvironment> CEnvironmentList;
    typedef QList<QHostAddress> CInterfaceList;
};

namespace qnut {
    class CDeviceManager : public QObject {
        Q_OBJECT
    public:
        CDeviceList devices;
    signals:
        void devicesUpdated(QStringList * names);
    };

    class CDevice : public QObject {
        Q_OBJECT
    //private:
    public:
        QString name;
        int activeEnvironment;
        bool enabled;
        CEnvironmentList environments;
    signals:
        void environmentChangedActive(int Index);
        void environmentsUpdated(QStringList * names);
    };
    
    class CEnvironment : public QObject {
        Q_OBJECT
    //private:
    public:
        CInterfaceList interfaces;
    signals:
        void interfacesUpdated(CInterfaceList * ip);
        void up();
        void down();
    };
};

#endif
