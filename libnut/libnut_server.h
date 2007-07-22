#include <QObject>
#include <QList>
#include <QStringList>
#include "../nuts/device.h"
#include "libnut_types.h"

//Verwaltet die unterene Klassen, sorgt für die kommunikation zwischen DBUS und den anderen objekten (im server teil)
class CNutsDBusConnection: public QObject {
    Q_OBJECT;
    private:
        QList<CNutsDBusDevice> devices;
    public:
        CNutsDBusConnection(DeviceManager * devmgr);
    public slots:
        //Funktionen, falls sich etwas ändert (mit serverteil verbinden)
    signals:
        //Funktionen, die aufgerufen werden müssen, wenn sich hier etwas ändert.
};
class CNutsDBusDeviceManager: public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "NUT_DBUS_URL.DeviceManager")
    public:
        CNutsDBusDeviceManager(QObject * parent);
    public Q_SLOTS:
        Q_SCRIPTABLE QList<QDBusObjectPath> getDeviceList();
    Q_SIGNALS:
        Q_SCRIPTABLE void deviceAdded(QDBusObjectPath objectpath);
        Q_SCRIPTABLE void deviceRemoved(QDBusObjectPath objectpath);
};
class CNutsDBusDevice: public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "NUT_DBUS_URL.Device")
    CNutsDBusDevice(QObject * parent);
    public Q_SLOTS:
        Q_SCRIPTABLE libnut_DeviceProperties getProperties();
        Q_SCRIPTABLE QList<QDBusObjectPath> getEnvironments();
        Q_SCRIPTABLE bool enable();
        Q_SCRIPTABLE bool disable();

    Q_SIGNALS:
        Q_SCRIPTABLE void environmentChangedActive(QString previous);
        Q_SCRIPTABLE void environmentsUpdated(QDBusObjectPath device);
        Q_SCRIPTABLE void stateChanged();
};
class CNutsDBusEnvironment: public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "NUT_DBUS_URL.Environment")
    public Q_SLOTS:
        //Geht noch nicht wirklich. muss mit Q_DECLARE_METATYPE berkannt gemacht werden+ and dbus
        //oder aber QList of QVariant und dann jeweils in 4er parsen
        //Return type: libnut_cli.h::CEnvironment::SelectConfig;
        Q_SCRIPTABLE QList<libnut_SelectType> getSelectConfig();
        Q_SCRIPTABLE libnut_SelectType getCurrentSelection();
        Q_SCRIPTABLE libnut_EnvironmentProperties getProperties();
        Q_SCRIPTABLE QList<QDBusObjectPath> getInterfaces();
    Q_SIGNALS:
        Q_SCRIPTABLE void interfacesUpdated();
    
};

class CNutsDBusInterface: public QOBject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "NUT_DBUS_URL.Interface")
    public Q_SLOTS:
        //top to down: see libnut_cli.h::CINterface public variables
        Q_SCRIPTABLE libnut_InterfaceProperties getProperties();
        Q_SCRIPTABLE void setIP(QVariant HostAddress);
        Q_SCRIPTABLE void setNetmask(quint32 Netmask);
        Q_SCRIPTABLE void setGateway(quint32 Gateway);
        //folgende womöglich woanders rein:
        Q_SCRIPTABLE void activate();
        Q_SCRIPTABLE void deactivate();
    Q_SIGNALS:
        Q_SCRIPTABLE void activeStateChanged();
};