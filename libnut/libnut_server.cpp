#include "libnut_server.h"
namespace libnut {


CNutsDBusConnection::CNutsDBusConnection(nuts::DeviceManager * devmgr) : devmgr(devmgr), connection(QDBusConnection::sessionBus()) {
    //DBus systemConnection herstellen und Service regististrieren
    connection.registerService(QLatin1String("NUT_DBUS_URL"));
    //QDBus Metatypen regististrieren
    qDBusRegisterMetaType<libnut_DeviceProperties>();
    qDBusRegisterMetaType<libnut_SelectConfig>();
    qDBusRegisterMetaType<libnut_EnvironmentProperties>();
    qDBusRegisterMetaType<libnut_InterfaceProperties>();
}
CNutsDBusConnection::~CNutsDBusConnection() {
    connection.unregisterService(QLatin1String("NUT_DBUS_URL"));
}

CNutsDBusDeviceManager::CNutsDBusDeviceManager(QObject * parent) : QObject(parent) {
    real_parent = (CNutsDBusConnection *) parent;
    connection = &(real_parent->connection);
    //DBus initialisieren
    new DeviceManagerAdaptor(this);
    connection->registerObject(QLatin1String("DeviceManager"),this);
}

CNutsDBusDeviceManager::~CNutsDBusDeviceManager() {
    connection->unregisterObject(QLatin1String("/DeviceManager"));
}

CNutsDBusDevice::CNutsDBusDevice(QString deviceName, QObject * parent) : QObject(parent) {
    real_parent = (CNutsDBusDeviceManager *) parent;
    connection = real_parent->connection;
    properties.name = deviceName;
    devObjectPath = "/Devices/" + deviceName;
    //DBus Initialisieren
    new DeviceAdaptor(this);
    connection->registerObject(QLatin1String(devObjectPath),this);
};
CNutsDBusDevice::~CNutsDBusDevice() {
    connection->unregisterObject(QLatin1String(devObjectPath));
}

CNutsDBusEnvironment::CNutsDBusEnvironment(int env_id, QObject * parent) : QObject(parent), env_id(env_id) {
    //DBus initialisieren
    real_parent = (CNutsDBusDevice *) parent;
    connection = real_parent->connection;
    deviceName = real_parent->properties.name;
    envObjectPath = "/Environments/" + deviceName + "/" + QString(env_id);
    new EnvironmentAdaptor(this);
    connection->registerObject(QLatin1String(envObjectPath),this);
    //Es fehlt: vieles
}
CNutsDBusEnvironment::~CNutsDBusEnvironment() {
    connection->unregisterObject(QLatin1String(envObjectPath));
}

CNutsDBusInterface::CNutsDBusInterface(int if_id, int env_id, QObject * parent) :
    QObject(parent), if_id(if_id), env_id(env_id) {
    real_parent = (CNutsDBusEnvironment *) parent;
    connection = real_parent->connection;
    ifObjectPath = "/Interfaces/" + QString(env_id) + "/" + QString(if_id);
    //DBus initialisieren:
    new InterfaceAdaptor(this);
    connection->registerObject(QLatin1String(ifObjectPath),this);
}
CNutsDBusInterface::~CNutsDBusInterface() {
    connection->unregisterObject(QLatin1String(ifObjectPath));
}

}
