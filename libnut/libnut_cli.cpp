#include "libnut_cli.h"
namespace libnut {
////////////////
//CDeviceManager
///////////////
CDeviceManager::CDeviceManager(QObject * parent) : QObject(parent), dbusConnection(QDBusConnection::systemBus()) {
}
CDeviceManager::~CDeviceManager() {
//Cleanup action: delete entire devicelist
    while (!devices.isEmpty()) {
        delete devices.takeFirst();
    }
}

void CDeviceManager::init() {
    //setup dbus connections
    dbusConnectionInterface = dbusConnection.interface();
    //Check if service is running
    try {
        serviceCheck(dbusConnectionInterface);
    }
    catch (CLI_ConnectionInitException& e) {
        QDBusReply<void> reply = dbusConnectionInterface->startService("NUT_DBUS_URL");
        serviceCheck(dbusConnectionInterface);
    }
    //Attach to DbusDevicemanager
    dbusDevmgr = new DBusDeviceManagerInterface("NUT_DBUS_URL", "/DeviceManager",dbusConnection, this);
    //get devicelist etc.
    QDBusReply<QList<QDBusObjectPath> > reply;
    reply = dbusDevmgr->getDeviceList();
    if (reply.isValid()) {
        dbusDeviceList = reply.value();
    }
    else {
        throw CLI_ConnectionException(tr("failed to get DeviceList"));
    }
    //Let's populate our own DeviceList
    CDevice * device;
    for (QList<QDBusObjectPath>::iterator i=dbusDeviceList.begin(); i != dbusDeviceList.end(); ++i) {
        device = new CDevice(this,*i);
        devices.append(device);
        emit(deviceAdded(device));
    }
}

//Check if service up
void CDeviceManager::serviceCheck(QDBusConnectionInterface * interface) {
    QDBusReply<bool> reply = interface->isServiceRegistered("NUT_DBUS_URL");
    if (reply.isValid()) {
        if (!reply.value()) {
            throw CLI_ConnectionInitException(tr("Please start NUTS"));
        }
    }
    else {
        throw CLI_ConnectionInitException(tr("Error while setting-up dbusconnection"));
    }
}


//CDeviceManager SLOTS
void CDeviceManager::refreshAll() {
    foreach(CDevice * i, devices) {
        i->refreshAll();
    }
}


/////////
//CDevice
/////////
CDevice::CDevice(CDeviceManager * parent, QDBusObjectPath dbuspath) : QObject(parent) {

}
CDevice::~CDevice() {

}

//
void CDevice::refreshAll() {}

//CDevice SLOTS
void CDevice::enable() {
}
void CDevice::disable() {
}
//////////////
//CEnvironment
//////////////

CEnvironment::CEnvironment(CDevice * parent) : QObject(parent) {

}
CEnvironment::~CEnvironment() {

}

//CEnvironment SLOTS
void CEnvironment::enter() {
}

////////////
//CInterface
////////////
CInterface::CInterface(CEnvironment * parent) : QObject(parent) {

}
CInterface::~CInterface() {

}

//CInterface SLOTS
void CInterface::activate() {
}
void CInterface::deactivate() {
}
void CInterface::setIP(QHostAddress & address) {
}
void CInterface::setNetmask(QHostAddress & address) {
}
void CInterface::setGateway(QHostAddress & address) {
}
void CInterface::setStatic(bool state) {
}

};
