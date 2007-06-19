#ifndef QNUT_COMMON_H
#define QNUT_COMMON_H

#include <QObject>
#include <QList>

namespace qnut {
    class CDeviceManager;

    class CDevice;
    class CEnvironment;
    class CInterface;

    typedef QList<CDevice> CDeviceList;
    typedef QList<CEnvironment> CEnvironmentList;
    typedef QList<CInterface> CInterfaceList;
};

namespace qnut {
    class CDeviceManager : public QObject {
        Q_OBJECT
    public:
        CDeviceList Devices;
    signals:
        void deviceAdded(QString name);
        void deviceRemoved(QString name);
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
        void environmentAdded();
        void environmentRemoved();
    };
    
    class CEnvironment : public QObject {
        Q_OBJECT
    //private:
    public:
        CInterfaceList interfaces;
    signals:
        void up();
        void down();
    };
    
    class CInterface : public QObject {
        Q_OBJECT
    //private:
    //public:
    signals:
        void up();
        void down();
    };
};

#endif
