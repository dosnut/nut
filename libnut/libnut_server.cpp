#include "libnut_server.h"
namespace libnut {
CNutsDBusConnection::CNutsDBusConnection(DeviceManager * indevmgr) {
    //DBus systemConnection herstellen und Service regististrieren
    QDBusConnection connection = QDBusConnection::systemBus();
    connection.registerService(QLatin1String("NUT_DBUS_URL"));
    //QDBus Metatypen regististrieren
    qDBusRegisterMetaType<libnut_DeviceProperties>();
    qDBusRegisterMetaType<libnut_SelectConfig>();
    qDBusRegisterMetaType<libnut_EnvironmentProperties>();
    qDBusRegisterMetaType<libnut_InterfaceProperties>();
    devmgr = indevmgr;
}
CNutsDBusConnection::~CNutsDBusConnection() {
    connection.unregisterService(QLatin1String("NUT_DBUS_URL"));
}

CNutsDBusDeviceManager::CNutsDBusDeviceManager(QObject * parent) : QObject(parent) {
    connection = &(parent->connection);
    //DBus initialisieren
    new DeviceManagerAdaptor(this);
    connection->registerObject(QLatin1String("DeviceManager"),this);
}

CNutsDBusDeviceManager::CNutsDBusDeviceManager() {
    connection->unregisterObject(QLatin1String("/DeviceManager"));
}

CNutsDBusDevice::CNutsDBusDevice(QString deviceName, QObject * parent) : QObject(parent) {
    connection = parent->connection;
    properties.name = deviceName;
    devObjectPath = "/Devices/" + devicename;
    //DBus Initialisieren
    new DeviceAdaptor(this);
    connection->registerObject(QLatin1String(devObjectPath),this);
};
CNutsDBusDevice::~CNutsDBusDevice() {
    connection->unregisterObject(QLatin1String(devObjectPath));
}

CNutsDBusEnvironment::CNutsDBusEnvironment(int env_id, QObject * parent) : QObject(parent), env_id(env_id) {
    //DBus initialisieren
    connection = parent->connection;
    deviceName = parent->properties.name;
    envObjectPath = "/Environments/" + deviceName + "/" + QString(env_id);
    new EnvironmentAdaptor(this);
    connection->registerObject(QLatin1String(envObjectPath),this);
    //Es fehlt: vieles
}
CNutsDBusEnvironment::~CNutsDBusEnvironment() {
    connection->unregisterObject(QLatin1String(envObjectPath));
}

CNutsDBusInterface::CNutsDBusInterface(int if_id, intenv_id, QObject * parent) :
    QObject(parent), if_id(if_id), env_id(env_id) {
    ifObjectPath = "/Interfaces/" + QString(env_id) + "/" + QString(if_id);
    connection = parent->connection;
    //DBus initialisieren:
    new InterfaceAdaptor(this);
    connection->registerObject(QLatin1String(ifObjectPath),this);
}
CNutsDBusInterface::~CNutsDBusInterface() {
    connection->unregisterObject(QLatin1String(ifObjectPath));
}

}