#ifndef QNUT_COMMON_H
#define QNUT_COMMON_H

#include <QObject>
#include <QList>

namespace qnut {
    class CDevice;
    class CEnvironment;
    class CInterface;

    typedef QList<CDevice> CDeviceList;
    typedef QList<CEnvironment> CEnvironmentList;
    typedef QList<CInterface> CInterfaceList;
};

namespace qnut {
    class CDevice : public QObject {
        Q_OBJECT
    //private:
    public:
        QString name;
        int activeEnvironment;
        bool enabled;
        CEnvironmentList environments;
    signals:
        void changedCurrentEnvironment(int Index);
        void addedEnvironment();
        void deletedEnvironment();
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
