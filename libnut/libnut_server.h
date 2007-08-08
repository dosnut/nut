#ifndef libnut_libnut_server
#define libnut_libnut_server

#include <QObject>
#include <QList>
#include <QStringList>
//QDBUS
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QVariant>
#include <QMetaType>

#include "../nuts/device.h"
#include "libnut_types.h"
#include "libnut_server_adaptor.h"

namespace libnut {
    class CNutsDBusConnection;
    class CNutsDBusDeviceManager;
    class CNutsDBusDevice;
    class CNutsDBusEnvironment;
    class CNutsDBusInterface;
}

namespace libnut {

//Verwaltet die unterene Klassen, sorgt für die kommunikation zwischen DBUS und den anderen objekten (im server teil)
class CNutsDBusConnection: public QObject {
    Q_OBJECT;
    private:
        QList<CNutsDBusDevice *> devices;
        nuts::DeviceManager * devmgr;
    public:
        CNutsDBusConnection(nuts::DeviceManager * devmgr);
        ~CNutsDBusConnection();
        QDBusConnection connection;
    public slots:
        //Funktionen, falls sich etwas ändert (mit serverteil verbinden)
    signals:
        //Funktionen, die aufgerufen werden müssen, wenn sich hier etwas ändert.
};
class CNutsDBusDeviceManager: public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "NUT_DBUS_URL.DeviceManager")
    private:
        DeviceManagerAdaptor * devmgr_adaptor;
        QDBusConnection * connection;
        CNutsDBusConnection * real_parent;
    public:
        CNutsDBusDeviceManager(QObject * parent);
        ~CNutsDBusDeviceManager();
    public slots:
        QList<QDBusObjectPath> getDeviceList();
    signals:
        void deviceAdded(QDBusObjectPath objectpath);
        void deviceRemoved(QDBusObjectPath objectpath);
};
class CNutsDBusDevice: public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "NUT_DBUS_URL.Device")
    private:
        DeviceAdaptor * dev_adaptor;
        QDBusConnection * connection;
        QString devObjectPath;
        libnut_DeviceProperties properties;
        CNutsDBusDeviceManager * real_parent;
    public:
        CNutsDBusDevice(QString deviceName, QObject * parent);
        ~CNutsDBusDevice();
    public slots:
        libnut_DeviceProperties getProperties();
        QList<QDBusObjectPath> getEnvironments();
        void setEnvironment(QDBusObjectPath envpath);
        bool enable();
        bool disable();

    signals:
        void environmentChangedActive(QDBusObjectPath newenv);
        void environmentsUpdated();
        void stateChanged();
};

class CNutsDBusEnvironment: public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "NUT_DBUS_URL.Environment")
    private:
        EnvironmentAdaptor * env_adaptor;
        QDBusConnection * connection;
        QString deviceName;
        QString envObjectPath;
        int env_id;
        libnut_EnvironmentProperties properties;
        CNutsDBusDevice * real_parent;
    public:
        CNutsDBusEnvironment(int env_id, QObject * parent);
        ~CNutsDBusEnvironment();
    public slots:
        QList<libnut_SelectConfig> getSelectConfig();
        libnut_SelectConfig getCurrentSelection();
        libnut_EnvironmentProperties getProperties();
        QList<QDBusObjectPath> getInterfaces();
    signals:
        void interfacesUpdated();
        void stateChanged(bool state);
    
};

class CNutsDBusInterface: public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "NUT_DBUS_URL.Interface")
    private:
        InterfaceAdaptor * if_adaptor;
        QDBusConnection * connection;
        int env_id;
        int if_id;
        QString ifObjectPath;
        CNutsDBusEnvironment * real_parent;
    public:
        CNutsDBusInterface(int if_id, int env_id, QObject * parent);
        ~CNutsDBusInterface();
    public slots:
        //top to down: see libnut_cli.h::CINterface public variables
        libnut_InterfaceProperties getProperties();
        void setIP(quint32 HostAddress);
        void setNetmask(quint32 Netmask);
        void setGateway(quint32 Gateway);
        //folgende womöglich woanders rein:
        void activate();
        void deactivate();
    signals:
        void stateChanged(bool state);
};
}
#endif
