#include "libnut_cli.h"
namespace libnut {
////////////////
//CDeviceManager
///////////////
CDeviceManager::CDeviceManager(QObject * parent) : QObject(parent) {
    //Check if service is running
    try {
        serviceCheck();
    }
    catch (CLI_ConnectionInitException& e) {
        QDBusReply<void> reply = dbusConnectionInterface.startService("NUT_DBUS_URL");
        serviceCheck();
    }
    //Attach to DbusDevicemanager
    dbusConnection = new QDBusConnection::systemBus();
    dbusDevmgr = new DBusDeviceManagerInterface("NUT_DBUS_URL", "/DeviceManager",*dbusConnection, this);
    //get devicelist etc.
    QDBusReply<QList<QDBusObjectPath>> reply = dbusDevmgr->getDeviceList();
    if (reply.isValid()) {
        dbusDeviceList = QDBusreply.value();
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
CDeviceManager::~CDeviceManager() {
//Cleanup action: delete entire devicelist
    while (!devices.isEmpty()) {
        delete devices.takeFirst();
    }
}
//Check if service up
CDeviceManager::serviceCheck() {
    QDBusReply<bool> reply = dbusConnectionInterface.isServiceRegistered("NUT_DBUS_URL");
    if (reply.isValid()) {
        if (!reply.Value()) {
            throw CLI_ConnectionInitException(tr("Please start NUTS"));
        }
    }
    else {
        throw CLI_ConnectionInitException(tr("Error while setting-up dbusconnection"));
    }
}


//CDeviceManager SLOTS
void CDeviceManager::refreshAll() {
    for (CDeviceList::iterator i=devices.begin(); i != devices.end(); ++i) {
        *i->refreshAll();
    }
}


/////////
//CDevice
/////////
CDevice::CDevice(QObject * parent, QDBusObjectPath dbuspath) : QObject(parent) {

}
CDevice::CDevice() {

}

//CDevice SLOTS
void CDevice::enable() {
}
void CDevice::disable() {
}
//////////////
//CEnvironment
//////////////

CEnvironment::CEnvironment() {

}
CEnvironment::~CEnvironment() {

}

//CEnvironment SLOTS
void CEnvironment::enter() {
}

////////////
//CInterface
////////////
CInterface::CInterface() {

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
