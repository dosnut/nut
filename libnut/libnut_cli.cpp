#include "libnut_cli.h"
#include <common/dbus.h>
namespace libnut {
////////////////
//CLog
///////////////
CLog::CLog(QObject * parent, QString fileName) : QObject(parent), file(fileName) {
    fileLoggingEnabled = file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
    
    if (fileLoggingEnabled) {
        outStream.setDevice(&file);
    }
}

void CLog::operator<<(QString text) {
    emit printed(text);
    
    if (fileLoggingEnabled) {
        outStream << text << endl;
    }
}
/////////////////////
//QDBusObjectPathList
/////////////////////
int QDBusObjectPathList::removeAll(const QDBusObjectPath &path) {
    int count = 0;
    for(QList<QDBusObjectPath>::iterator i = begin(); i != end(); i++) {
        if ((*i).path() == path.path()) {
            count++;
            erase(i);
        }
    }
    return count;
}
//The following is VERY ugly (don't try this at home):
QDBusObjectPathList & QDBusObjectPathList::operator= ( const QDBusObjectPathList & other ) {
    //First: get pointer to base class of other (that's QList<QDBusObjectPath>)
    QList<QDBusObjectPath> * base = (QList<QDBusObjectPath>*) &other;
    //"QList<QDBusObjectPath>::operator=(*base)" returns base class
    //Then get the address in order to typecast with pointer
    return * ( (QDBusObjectPathList*) &( QList<QDBusObjectPath>::operator=(*base) ) );
}
QDBusObjectPathList & QDBusObjectPathList::operator= ( const QList<QDBusObjectPath> & other ) {
return * ( (QDBusObjectPathList*) &( QList<QDBusObjectPath>::operator=(other) ) );
}

////////////////
//CLibNut
///////////////
//Check if service up
void CLibNut::serviceCheck(QDBusConnectionInterface * interface) {
    QDBusReply<bool> reply = interface->isServiceRegistered(NUT_DBUS_URL);
    if (reply.isValid()) {
        if (!reply.value()) {
            throw CLI_ConnectionInitException(tr("Please start NUTS"));
        }
    }
    else {
        throw CLI_ConnectionInitException(tr("Error while setting-up dbusconnection"));
    }
}
void CLibNut::objectCheck(QDBusConnectionInterface * interface) {
}

////////////////
//CDeviceManager
///////////////
CDeviceManager::CDeviceManager(QObject * parent) : CLibNut(parent), dbusConnection(QDBusConnection::systemBus()) {
}
CDeviceManager::~CDeviceManager() {
//Cleanup action: delete entire devicelist
    CDevice * device;
    while (!devices.isEmpty()) {
        device = devices.takeFirst();
        emit(deviceRemoved(device)); //maybe comment out, depends on client
        delete device;
    }
}

void CDeviceManager::init(CLog * inlog) {
    log = inlog;
    //setup dbus connections
    dbusConnectionInterface = dbusConnection.interface();
    //Check if service is running
    try {
        serviceCheck(dbusConnectionInterface);
    }
    catch (CLI_ConnectionInitException& e) {
        QDBusReply<void> reply = dbusConnectionInterface->startService(NUT_DBUS_URL);
        serviceCheck(dbusConnectionInterface);
    }
    //Attach to DbusDevicemanager
    dbusDevmgr = new DBusDeviceManagerInterface(NUT_DBUS_URL, "/manager",dbusConnection, this);
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
        try {
            device = new CDevice(this,*i);
        }
        catch (CLI_DevConnectionException e) {
            dbusDeviceList.removeAll(*i);
            *log << e.msg();
            continue;
        }
        devices.append(device);
        emit(deviceAdded(device));
    }
    //Connect dbus-signals to own slots:
    connect(dbusDevmgr, SIGNAL(deviceAdded(const QDBusObjectPath&)), this, SLOT(dbusDeviceAdded(const QDBusObjectPath&)));
    connect(dbusDevmgr, SIGNAL(deviceRemoved(const QDBusObjectPath&)), this, SLOT(dbusDeviceRemoved(const QDBusObjectPath&)));
}
//CDeviceManager DBUS-SLOTS:
void CDeviceManager::dbusDeviceAdded(const QDBusObjectPath &objectpath) {
    *log << (tr("Adding device at: ") + objectpath.path());
    CDevice * device;
    try {
        device = new CDevice(this, objectpath);
    }
    catch (CLI_DevConnectionException e) {
        *log << e.msg();
        return;
    }
    dbusDeviceList.append(objectpath);
    devices.append(device);
    emit(deviceAdded(device));
}
void CDeviceManager::dbusDeviceRemoved(const QDBusObjectPath &objectpath) {
    //remove devices from devicelist
    CDevice * device;
    for (QList<CDevice *>::iterator i = devices.begin(); i != devices.end(); ++i) {
        if ((*i)->dbusPath.path() == objectpath.path()) {
            device = *i;
            *log << (tr("Remove device: ") + device->name + " at " + objectpath.path());
            devices.erase(i);
            emit(deviceRemoved(device)); //before erasing device
            delete(device);
        }
    }
    //remove all occurences of objectpath in our devicelist
    if (dbusDeviceList.removeAll(objectpath) != 1) {
        *log << (tr("Error while removing dbusobjectpath") + objectpath.path());
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
CDevice::CDevice(CDeviceManager * parent, QDBusObjectPath dbusPath) : CLibNut(parent), parent(parent), dbusPath(dbusPath) {
    log = parent->log;
    //get dbusConnection from parent:
    dbusConnection = &(parent->dbusConnection);
    dbusConnectionInterface = parent->dbusConnectionInterface;
    //Service check
    serviceCheck(dbusConnectionInterface);
    //connect to dbus-object
    dbusDevice = new DBusDeviceInterface(NUT_DBUS_URL, dbusPath.path(),*dbusConnection, this);
    
    //get properties
    *log << (tr("Getting device properties at: ") + dbusPath.path());
    QDBusReply<libnut_DeviceProperties> replyProp = dbusDevice->getProperties();
    if (replyProp.isValid()) {
        name = replyProp.value().name;
        type = replyProp.value().type;
        enabled = replyProp.value().enabled;
        actEnv = replyProp.value().activeEnvironment;
        activeEnvironment = actEnv >= 0 ? environments[actEnv] : 0;
        *log << (tr("Device properties fetched:"));
        *log << (tr("Name: ") + QString(name));
        *log << (tr("Type: ") + QString(type));
        *log << (tr("State: ") + QString(enabled));
        *log << (tr("Active Environement: ") + actEnv);
    }
    else {
        throw CLI_DevConnectionException(tr("Error while retrieving dbus' device information"));
    }
    //get Environment list
    //set activeEnv to NULL
    activeEnvironment = 0;
    QDBusReply<QList<QDBusObjectPath> > replyEnv = dbusDevice->getEnvironments();
    if (replyEnv.isValid()) {
        dbusEnvironmentList = replyEnv.value();
        //poppulate own Environmentlist
        CEnvironment * env;
        CEnvironment * activeenv;
        foreach(QDBusObjectPath i, dbusEnvironmentList) {
            *log << (tr("Adding Environment at: ") + i.path());
            try {
                env = new CEnvironment(this,i);
            }
            catch (CLI_EnvConnectionException e) {
                *log << e.msg();
                dbusEnvironmentList.removeAll(i);
                continue;
            }
            environments.append(env);
            //set active Environment
            if (env->active) {
                activeenv = activeEnvironment;
                activeEnvironment = env;
                emit(environmentChangedActive(env, activeenv));
            }
            //Maybe we need to send signal environmentsUpdated?
            emit(environmentsUpdated());
        }
    }
    else {
        throw CLI_DevConnectionException(tr("Error while retrieving environment list"));
    }
    //connect signals to slots
    connect(dbusDevice, SIGNAL(environmentChangedActive(const QDBusObjectPath &)),
            this, SIGNAL(environmentChangedActive(const QDBusObjectPath &)));
    //MISSING!!!: env added, env removed, state changed with change!!!!
}
CDevice::~CDevice() {
    CEnvironment * env;
    while (!environments.isEmpty()) {
        env = environments.takeFirst();
        emit(environmentsUpdated());
        delete env;
    }
}

//CDevice private functions

/**
 * Refreshes Device properties and environment as well as their properties
 * Will NOT send any signals! Invoked by DeviceManager::refreshAll()!
 */
void CDevice::refreshAll() {
    //refresh environments
    foreach(CEnvironment * i, environments) {
        i->refreshAll();
    }
    //Refresh device:
    QDBusReply<libnut_DeviceProperties> replyprop = dbusDevice->getProperties();
    if (replyprop.isValid()) {
        name = replyprop.value().name;
        enabled = replyprop.value().enabled;
        type = replyprop.value().type;
        //find active Environment:
        actEnv = replyprop.value().activeEnvironment;
        activeEnvironment = actEnv >= 0 ? environments[actEnv] : 0;
    }
}

//CDevice SLOTS
void CDevice::enable() {
}
void CDevice::disable() {
}
CEnvironment * CDevice::addEnvironment(QString name) {
    return 0;
}
void CDevice::removeEnvironment(CEnvironment * environment) {
}

//////////////
//CEnvironment
//////////////

CEnvironment::CEnvironment(CDevice * parent, QDBusObjectPath dbusPath) : CLibNut(parent), parent(parent), dbusPath(dbusPath) {
}
CEnvironment::~CEnvironment() {

}

void CEnvironment::refreshAll() {
}

//CEnvironment SLOTS
void CEnvironment::enter() {
}
void CEnvironment::addInterface(bool isStatic, QHostAddress ip, QHostAddress netmask, QHostAddress gateway) {
}
void CEnvironment::removeInterface(CInterface * interface) {
}

////////////
//CInterface
////////////
CInterface::CInterface(CEnvironment * parent, QDBusObjectPath dbusPath) : CLibNut(parent), parent(parent), dbusPath(dbusPath) {

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
