#include "libnut_server.h"
namespace libnut {


//CNutsDBusConnection
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
//SLOTS

//CNutsDBusDeviceManager
CNutsDBusDeviceManager::CNutsDBusDeviceManager(QObject * parent) : QObject(parent) {
    real_parent = (CNutsDBusConnection *) parent;
    connection = &(real_parent->connection);
    //DBus initialisieren
    new DeviceManagerAdaptor(this);
    connection->registerObject(QLatin1String("/DeviceManager"),this);
}

CNutsDBusDeviceManager::~CNutsDBusDeviceManager() {
    connection->unregisterObject(QLatin1String("/DeviceManager"));
}

//DBUS-SLOTS
QList<QDBusObjectPath> CNutsDBusDeviceManager::getDeviceList() {
    return deviceList;
}


//CNutsDBusDevice
CNutsDBusDevice::CNutsDBusDevice(QString deviceName, QObject * parent) : QObject(parent) {
    real_parent = (CNutsDBusDeviceManager *) parent;
    connection = real_parent->connection;
    properties.name = deviceName;
    devObjectPath = "/Devices/" + deviceName;
    //DBus Initialisieren
    new DeviceAdaptor(this);
    connection->registerObject(QLatin1String(devObjectPath.toLatin1()),this);
};
CNutsDBusDevice::~CNutsDBusDevice() {
    connection->unregisterObject(QLatin1String(devObjectPath.toLatin1()));
}

//DBUS-SLOTS
libnut_DeviceProperties CNutsDBusDevice::getProperties() {
    #ifdef DEVICE_TEST
    libnut_DeviceProperties testvar;
    testvar.name = QString("testdevice");
    testvar.activeEnvironment = true;
    testvar.enabled = true;
    testvar.type = 0;
    return testvar;
    #endif
    #ifndef DEVICE_TEST
    
    #endif
}
QList<libnut_wlanScanresult> CNutsDBusDevice::getwlanScan() {
    #ifdef DEVICE_TEST
    libnut_wlanScanresult testvar;
    testvar.essid = QList("tolleswlanhier");
    testvar.channel = 1337;
    testvar.bssid = {15,15,15,15,15,15};
    testvar.flags = 0;
    testvar.signallevel = 99999;
    testvar.encryption = 0;
    QList<libnut_wlanScanresult> testvarlist;
    testvarlist.append(testvar);
    return testvarlist;
    #endif
    #ifndef DEVICE_TEST

    #endif
}
QDBusObjectPath CNutsDBusDevice::addwlanEnvironment(libnut_wlanNetworkProperties netprops) {
    #ifdef DEVICE_TEST
    QDBusObjectPath testvar("/Environments/testenv");
    return testvar;
    #endif
    #ifndef DEVICE_TEST

    #endif
}
QList<QDBusObjectPath> CNutsDBusDevice::getEnvironments() {
    #ifdef DEVICE_TEST
    QDBusObjectPath testvar("/Environments/testenv");
    QList<QDBusObjectPath> testvarlist;
    testvarlist.append(testvar);
    return testvarlist;
    #endif
    #ifndef DEVICE_TEST

    #endif
}
void CNutsDBusDevice::setEnvironment(QDBusObjectPath envpath) {
    #ifdef DEVICE_TEST
    
    #endif
    #ifndef DEVICE_TEST

    #endif
}
bool CNutsDBusDevice::enable() {
    #ifdef DEVICE_TEST
    return true;
    #endif
    #ifndef DEVICE_TEST

    #endif
}
bool CNutsDBusDevice::disable() {
    #ifdef DEVICE_TEST
    return true;
    #endif
    #ifndef DEVICE_TEST

    #endif
}

//CNutsDBusEnvironment
CNutsDBusEnvironment::CNutsDBusEnvironment(int env_id, QObject * parent) : QObject(parent), env_id(env_id) {
    //DBus initialisieren
    real_parent = (CNutsDBusDevice *) parent;
    connection = real_parent->connection;
    deviceName = real_parent->properties.name;
    envObjectPath = "/Environments/" + deviceName + "/" + QString(env_id);
    new EnvironmentAdaptor(this);
    connection->registerObject(QLatin1String(envObjectPath.toLatin1()),this);
    //Es fehlt: vieles
}
CNutsDBusEnvironment::~CNutsDBusEnvironment() {
    connection->unregisterObject(QLatin1String(envObjectPath.toLatin1()));
}

//DBUS-SLOTS
QList<libnut_SelectConfig> CNutsDBusEnvironment::getSelectConfig() {
    #ifdef ENVIRONMENT_TEST
    libnut_SelectConfig testvar;
    QList<libnut_SelectConfig> testvarlist;
    testvar.type=0;
    testvar.bool = true;
    testvar.arpIP = QHostAddress("192.168.0.53");
    testvar.essid = QString("tollerap");
    testvarlist.append(testvar);
    return testvarlist;
    #endif
    #ifndef ENVIRONMENT_TEST

    #endif
}
libnut_SelectConfig CNutsDBusEnvironment::getCurrentSelection() {
    #ifdef ENVIRONMENT_TEST
    libnut_SelectConfig testvar;
    testvar.type=0;
    testvar.bool = true;
    testvar.arpIP = QHostAddress("192.168.0.53");
    testvar.essid = QString("tollerap");
    return testvar;
    #endif
    #ifndef ENVIRONMENT_TEST

    #endif
}
libnut_EnvironmentProperties CNutsDBusEnvironment::getProperties() {
    #ifdef ENVIRONMENT_TEST
    libnut_EnvironmentProperties testvarenv;
    libnut_SelectConfig testvar;
    testvar.type=0;
    testvar.bool = true;
    testvar.arpIP = QHostAddress("192.168.0.53");
    testvar.essid = QString("tollerap");
    testvarenv.active = true;
    testvarenv.name = QString"testenv");
    testvarenv.currentSelection = testvar;
    return testvarenv;
    #endif
    #ifndef ENVIRONMENT_TEST

    #endif
}
QList<QDBusObjectPath> CNutsDBusEnvironment::getInterfaces() {
    #ifdef ENVIRONMENT_TEST
    
    #endif
    #ifndef ENVIRONMENT_TEST

    #endif
}
QDBusObjectPath CNutsDBusEnvironment::addInterface(libnut_InterfaceProperties prop, bool state) {
    #ifdef ENVIRONMENT_TEST

    #endif
    #ifndef ENVIRONMENT_TEST

    #endif
}


//CNutsDBusInterface
CNutsDBusInterface::CNutsDBusInterface(int if_id, int env_id, QObject * parent) :
    QObject(parent), if_id(if_id), env_id(env_id) {
    real_parent = (CNutsDBusEnvironment *) parent;
    connection = real_parent->connection;
    ifObjectPath = "/Interfaces/" + QString(env_id) + "/" + QString(if_id);
    //DBus initialisieren:
    new InterfaceAdaptor(this);
    connection->registerObject(QLatin1String(ifObjectPath.toLatin1()),this);
}
CNutsDBusInterface::~CNutsDBusInterface() {
    connection->unregisterObject(QLatin1String(ifObjectPath.toLatin1()));
}
//DBUS-SLOTS
libnut_InterfaceProperties CNutsDBusInterface::getProperties() {
    #ifdef INTERFACE_TEST

    #endif
    #ifndef INTERFACE_TEST

    #endif
}
void CNutsDBusInterface::setIP(quint32 HostAddress) {
    #ifdef INTERFACE_TEST
    
    #endif
    #ifndef INTERFACE_TEST
    
    #endif
}
void CNutsDBusInterface::setNetmask(quint32 Netmask) {
    #ifdef INTERFACE_TEST

    #endif
    #ifndef INTERFACE_TEST

    #endif
}
void CNutsDBusInterface::setGateway(quint32 Gateway) {
    #ifdef INTERFACE_TEST

    #endif
    #ifndef INTERFACE_TEST

    #endif
}
//folgende womöglich woanders rein:
void CNutsDBusInterface::activate() {
    #ifdef INTERFACE_TEST

    #endif
    #ifndef INTERFACE_TEST

    #endif
}
void CNutsDBusInterface::deactivate() {
    #ifdef INTERFACE_TEST

    #endif
    #ifndef INTERFACE_TEST

    #endif
}

}
