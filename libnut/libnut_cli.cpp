#include "libnut_cli.h"
#include <common/dbus.h>

//things that may need to be changed:
//-Check if dev/env/if is already in list (maybe refresh then)
//-Check if we can remove dev/env/if
//-When refreshing: send Changesignals?
//-wlan sach
//-more debugging output
namespace libnut {

QString toString(DeviceState state) {
	switch (state) {
		case DS_UP:             return QObject::tr("up");
		case DS_UNCONFIGURED:   return QObject::tr("unconfigured");
		case DS_CARRIER:        return QObject::tr("got carrier");
		case DS_ACTIVATED:      return QObject::tr("activated");
		case DS_DEACTIVATED:    return QObject::tr("deactivated");
		default:                return QString();
	}
}
QString toString(DeviceType type) {
	switch (type) {
		case DT_ETH: return QObject::tr("Ethernet");
		case DT_AIR: return QObject::tr("Wireless");
		case DT_PPP: return QObject::tr("PPP");
		default:     return QString();
	}
}

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
	//Init Hashtable
	dbusDevices.reserve(10);
}
CDeviceManager::~CDeviceManager() {
//Cleanup action: delete entire devicelist
	CDevice * device;
	while (!devices.isEmpty()) {
		device = devices.takeFirst();
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
	QDBusReply<QList<QDBusObjectPath> > replydevs;
	replydevs = dbusDevmgr->getDeviceList();
	if (!replydevs.isValid()) {
		throw CLI_ConnectionException(tr("Failed to get DeviceList"));
	}
	//Let's populate our own DeviceList
	CDevice * device;
	foreach (QDBusObjectPath i, replydevs.value()) {
//    for (QList<QDBusObjectPath>::iterator i=replydevs.value().begin(); i != replydevs.value().end(); ++i) {
		try {
			device = new CDevice(this, i);
		}
		catch (CLI_DevConnectionException e) {
			*log << e.msg();
			continue;
		}
		devices.append(device);
		dbusDevices.insert(i, device);
		emit(deviceAdded(device));
	}
	//Connect dbus-signals to own slots:
	connect(dbusDevmgr, SIGNAL(deviceAdded(const QDBusObjectPath&)), this, SLOT(dbusDeviceAdded(const QDBusObjectPath&)));
	connect(dbusDevmgr, SIGNAL(deviceRemoved(const QDBusObjectPath&)), this, SLOT(dbusDeviceRemoved(const QDBusObjectPath&)));
}
//CDeviceManager DBUS-SLOTS:
void CDeviceManager::dbusDeviceAdded(const QDBusObjectPath &objectpath) {
	if (!dbusDevices.contains(objectpath)) {
		*log << (tr("Adding device at: ") + objectpath.path());
		CDevice * device;
		try {
			device = new CDevice(this, objectpath);
		}
		catch (CLI_DevConnectionException e) {
			*log << e.msg();
			return;
		}
		dbusDevices.insert(objectpath,device);
		devices.append(device);
		emit(deviceAdded(device));
		}
	else {
		dbusDevices.value(objectpath)->refreshAll();
	}
}
void CDeviceManager::dbusDeviceRemoved(const QDBusObjectPath &objectpath) {
	//remove devices from devicelist
	CDevice * device = dbusDevices.take(objectpath);
	devices.removeAll(device);
	emit(deviceRemoved(device));
	delete device;
}


//CDeviceManager SLOTS
void CDeviceManager::refreshAll() {
	//Get our current DeviceList
	QDBusReply<QList<QDBusObjectPath> > replydevs = dbusDevmgr->getDeviceList();
	if (replydevs.isValid()) {
		//If a device is missing or there are too many devices in our Hash
		//Then rebuild complete tree, otherwise just call refresh on child
		bool equal = true;
		if ( (dbusDevices.size() == replydevs.value().size()) ) {
			foreach(QDBusObjectPath i, replydevs.value()) {
				if ( !dbusDevices.contains(i) ) {
					equal = false;
					break;
				}
			}
			if ( equal ) {
				foreach(CDevice * i, devices) {
					i->refreshAll();
				}
			}
			else {
				rebuild(replydevs.value());
				return;
			}
		}
		else {
			rebuild(replydevs.value());
			return;
		}
	}
	else {
		*log << tr("Could not refresh device list");
	}
}

//CDeviceManager private functions:
//rebuilds the device list
void CDeviceManager::rebuild(QList<QDBusObjectPath> paths) {
	//Delete all devices
	dbusDevices.clear();
	CDevice * device;
	while (!devices.isEmpty()) {
		device = devices.takeFirst();
		emit(deviceRemoved(device));
		delete device;
	}
	//Build new devices
	foreach(QDBusObjectPath i, paths) {
		try {
			device = new CDevice(this, i);
		}
		catch (CLI_ConnectionException &e) {
			*log << e.what();
			continue;
		}
		dbusDevices.insert(i, device);
		devices.append(device);
		emit(deviceAdded(device));
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
	QDBusReply<DeviceProperties> replyProp = dbusDevice->getProperties();
	if (replyProp.isValid()) {
		name = replyProp.value().name;
		type = replyProp.value().type;
		state = (DeviceState) replyProp.value().state;
		activeEnvironment = 0;
		if (!replyProp.value().activeEnvironment.isEmpty()) {
			dbusActiveEnvironment = QDBusObjectPath(replyProp.value().activeEnvironment);
			activeEnvironment = dbusEnvironments.value(dbusActiveEnvironment);
			emit(environmentChangedActive(activeEnvironment, 0));
		}
		*log << (tr("Device properties fetched"));
		*log << (tr("Name") + ": " + QString(name));
		*log << (tr("Type") + ": " + toString(type));
		*log << (tr("State") + ": " + toString(state));
		*log << (tr("Active Environement") + ": " + dbusActiveEnvironment.path());
	}
	else {
		throw CLI_DevConnectionException(tr("Error while retrieving dbus' device information"));
	}
	//get Environment list
	//set activeEnv to NULL
	QDBusReply<QList<QDBusObjectPath> > replyEnv = dbusDevice->getEnvironments();
	if (!replyEnv.isValid()) {
		throw CLI_DevConnectionException(tr("Error while trying to get environment list"));
	}
	//poppulate own Environmentlist
	CEnvironment * env;
	foreach(QDBusObjectPath i, replyEnv.value()) {
		*log << (tr("Adding Environment at: ") + i.path());
		try {
			env = new CEnvironment(this,i);
		}
		catch (CLI_EnvConnectionException e) {
			*log << e.msg();
			continue;
		}
		dbusEnvironments.insert(i,env);
		environments.append(env);
		//Maybe we need to send signal environmentsUpdated?
		emit(environmentsUpdated());
	}
	//connect signals to slots
	connect(dbusDevice, SIGNAL(environmentChangedActive(const QDBusObjectPath &)),
			this, SLOT(environmentChangedActive(const QDBusObjectPath &)));

	connect(dbusDevice, SIGNAL(environmentRemoved(const QDBusObjectPath &)),
			this, SLOT(environmentRemoved(const QDBusObjectPath &)));

	connect(dbusDevice, SIGNAL(environmentAdded(const QDBusObjectPath &)),
			this, SLOT(environmentAdded(const QDBusObjectPath &)));

	connect(dbusDevice, SIGNAL(stateChanged(int , int)),
			this, SLOT(dbusstateChanged(int, int)));
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

void CDevice::refreshAll() {
	//Refresh environment list
	QDBusReply<QList<QDBusObjectPath> > replyenvs = dbusDevice->getEnvironments();
	if (replyenvs.isValid()) {
		//Compare local with remote list:
		//if they are equal just refresh otherwise rebuild
		bool envequal = (dbusEnvironments.size() == replyenvs.value().size());
		if (envequal) {
			foreach(QDBusObjectPath i, replyenvs.value()) {
				if ( !dbusEnvironments.contains(i) ) {
					envequal = false;
					break;
				}
			}
		}
		if (envequal) {
			foreach(CEnvironment * i, environments) {
				i->refreshAll();
			}
		}
		else {
			rebuild(replyenvs.value());
		}
	}
	else {
		*log << tr("Could not refresh environments");
	}
	activeEnvironment = 0;
	//now refresh the rest of our device properties:
	QDBusReply<DeviceProperties> replyprop = dbusDevice->getProperties();
	if (replyprop.isValid()) {
		if (!replyprop.value().activeEnvironment.isEmpty()) {
			dbusActiveEnvironment = QDBusObjectPath(replyprop.value().activeEnvironment);
			activeEnvironment = dbusEnvironments.value(dbusActiveEnvironment);
		}
		state = (DeviceState) replyprop.value().state;
		type = replyprop.value().type;
		name = replyprop.value().name;
	}
	else {
		*log << tr("Could not refresh device properties");
	}
}
//Rebuilds the environment list
void CDevice::rebuild(QList<QDBusObjectPath> paths) {
	//Remove current list:
	dbusEnvironments.clear();
	CEnvironment * env;
	while ( !environments.isEmpty() ) {
		env = environments.takeFirst();
		emit(environmentRemoved(env));
		delete env;
	}
	//now rebuild:
	foreach(QDBusObjectPath i, paths) {
		try {
			env = new CEnvironment(this,i);
		}
		catch (CLI_ConnectionException &e) {
			*log << e.what();
			continue;
		}
		dbusEnvironments.insert(i,env);
		environments.append(env);
	}
	emit(environmentsUpdated());
}

//CDevice private slots:
void CDevice::environmentChangedActive(const QDBusObjectPath &newenv) {
	CEnvironment * oldenv = activeEnvironment;
	activeEnvironment = dbusEnvironments.value(newenv);
	emit(environmentChangedActive(activeEnvironment,oldenv));
}
void CDevice::environmentAdded(const QDBusObjectPath &path) {
	if (!dbusEnvironments.contains(path)) {
		CEnvironment * env;
		try {
			env = new CEnvironment(this,path);
		}
		catch (CLI_ConnectionException &e) {
			*log << e.what();
			return;
		}
		dbusEnvironments.insert(path,env);
		emit(environmentsUpdated());
		emit(environmentAdded(env));
	}
	else {
		dbusEnvironments.value(path)->refreshAll();
		emit(environmentsUpdated());
	}
}
void CDevice::environmentRemoved(const QDBusObjectPath &path) {
	if (dbusEnvironments.contains(path)) {
		CEnvironment * env = dbusEnvironments.take(path);
		environments.removeAll(env);
		emit(environmentRemoved(env));
		delete env;
		emit(environmentsUpdated());
	}
	else {
		*log << tr("Tried to remove non-existing environment");
	}
}
void CDevice::dbusstateChanged(int newState, int oldState) {
	state = (DeviceState) newState;
	emit(stateChanged(state));
}


//CDevice SLOTS
void CDevice::enable() {
	dbusDevice->enable();
}
void CDevice::disable() {
	dbusDevice->disable();
}
void CDevice::setEnvironment(CEnvironment * environment) {
	dbusDevice->setEnvironment(dbusEnvironments.key(environment));
}
void CDevice::addEnvironment(QString name) {
	EnvironmentProperties props;
	props.name = name;
	dbusDevice->addEnvironment(props);
}
void CDevice::removeEnvironment(CEnvironment * environment) {
	dbusDevice->removeEnvironment(dbusEnvironments.key(environment));
}

//////////////
//CEnvironment
//////////////

CEnvironment::CEnvironment(CDevice * parent, QDBusObjectPath dbusPath) : CLibNut(parent), parent(parent), dbusPath(dbusPath) {
	//Set log.
	log = parent->log;
	//First attach to dbus
	dbusConnection = parent->dbusConnection;
	dbusConnectionInterface = parent->dbusConnectionInterface;
	serviceCheck(dbusConnectionInterface);
	dbusEnvironment = new DBusEnvironmentInterface(NUT_DBUS_URL, dbusPath.path(),*dbusConnection,this);
	//Retrieve dbus information:
	QDBusReply<EnvironmentProperties> replyprop = dbusEnvironment->getProperties();
	if (replyprop.isValid()) {
		name = replyprop.value().name;
	}
	else {
		throw CLI_EnvConnectionException(tr("Error while retrieving environment properties"));
	}
	QDBusReply<QList<SelectConfig> > replyselconfs = dbusEnvironment->getSelectConfig();
	if (replyselconfs.isValid()) {
		selectStatements = replyselconfs.value();
	}
	else {
		throw CLI_EnvConnectionException(tr("Error while retrieving environment select config"));
	}
	
 	QDBusReply<QList<QDBusObjectPath> > replyifs = dbusEnvironment->getInterfaces();
	if (replyifs.isValid()) {
		CInterface * interface;
		foreach(QDBusObjectPath i, replyifs.value()) {
			try {
				interface = new CInterface(this,i);
			}
			catch (CLI_ConnectionException &e) {
				*log << e.what();
				continue;
			}
			dbusInterfaces.insert(i,interface);
			interfaces.append(interface);
		}
	}
	else {
		throw CLI_EnvConnectionException(tr("Error while retrieving environment's interfaces"));
	}
	connect(dbusEnvironment, SIGNAL(interfaceAdded(const QDBusObjectPath &)), this, SLOT(dbusinterfaceAdded(const QDBusObjectPath &)));
	connect(dbusEnvironment, SIGNAL(interfaceRemoved(const QDBusObjectPath &)), this, SLOT(dbusinterfaceRemoved(const QDBusObjectPath &)));
	connect(dbusEnvironment, SIGNAL(stateChanged(bool )), this, SLOT(dbusstateChanged(bool )));
}
CEnvironment::~CEnvironment() {
	CInterface * interface;
	while (!interfaces.isEmpty()) {
		interface = interfaces.takeFirst();
		emit(interfaceRemoved(interface));
		emit(interfacesUpdated());
		delete interface;
	}
}

//CEnvironment private functions

void CEnvironment::refreshAll() {
	//Retrieve properties and select config, then interfaces:
	QDBusReply<EnvironmentProperties> replyprop = dbusEnvironment->getProperties();
	if (replyprop.isValid()) {
		name = replyprop.value().name;
	}
	else {
		*log << tr("Error while refreshing environment properties");
	}
	QDBusReply<QList<SelectConfig> > replyselconfs = dbusEnvironment->getSelectConfig();
	if (replyselconfs.isValid()) {
		selectStatements = replyselconfs.value();
	}
	else {
		*log << tr("Error while refreshing environment select config");
	}
	QDBusReply<QList<QDBusObjectPath> > replyifs = dbusEnvironment->getInterfaces();
	if (replyifs.isValid()) {
		//Check if we need to rebuild the interface list or just refresh them:
		bool ifequal = (replyifs.value().size() == dbusInterfaces.size());
		if (ifequal) {
			foreach(QDBusObjectPath i, replyifs.value()) {
				if (!dbusInterfaces.contains(i)) {
					ifequal = false;
					break;
				}
			}
			if (ifequal) {
				foreach (CInterface * i, interfaces) {
					i->refreshAll();
				}
			}
			else {
				rebuild(replyifs.value());
				return;
			}
		}
		else {
			rebuild(replyifs.value());
			return;
		}
	}
	else {
		*log << tr("Error while refreshing environment interfaces");
	}
}
void CEnvironment::rebuild(const QList<QDBusObjectPath> &paths) {
	//Remove all interfaces
	dbusInterfaces.clear();
	CInterface * interface;
	while (!interfaces.isEmpty()) {
		interface = interfaces.takeFirst();
		emit(interfaceRemoved(interface));
		emit(interfacesUpdated());
		delete interface;
	}
	//Now rebuild:
	foreach(QDBusObjectPath i, paths) {
		try {
			interface = new CInterface(this,i);
		}
		catch (CLI_ConnectionException &e) {
			*log << e.what();
			continue;
		}
		dbusInterfaces.insert(i,interface);
		interfaces.append(interface);
	}
}

//CEnvironment private slots:
void CEnvironment::dbusinterfaceAdded(const QDBusObjectPath &path) {
	if (!dbusInterfaces.contains(path)) {
		CInterface * interface;
		try {
			interface = new CInterface(this,path);
		}
		catch (CLI_IfConnectionException e){
			*log << e.what();
			return;
		}
		dbusInterfaces.insert(path,interface);
		interfaces.append(interface);
	}
	else {
		dbusInterfaces.value(path)->refreshAll();
	}
}
void CEnvironment::dbusinterfaceRemoved(const QDBusObjectPath &path) {
	if (dbusInterfaces.contains(path)) {
		CInterface * interface = dbusInterfaces.take(path);
		interfaces.removeAll(interface);
		emit(interfaceRemoved(interface));
		delete interface;
		emit(interfacesUpdated());
	}
	else {
		*log << tr("Tried to remove non-existing interface");
	}
}
void CEnvironment::dbusstateChanged(bool state) {
	active = state;
	emit(activeChanged(state));
}


//CEnvironment SLOTS
void CEnvironment::enter() {
	parent->setEnvironment(this);
}
void CEnvironment::addInterface(bool isStatic, QHostAddress ip, QHostAddress netmask, QHostAddress gateway, bool active=true) {
	InterfaceProperties ifprops;
	ifprops.isStatic = isStatic;
	ifprops.active = active;
	ifprops.userDefineable = true;
	ifprops.ip = ip;
	ifprops.netmask = netmask;
	ifprops.gateway = gateway;
	dbusEnvironment->addInterface(ifprops);
}
void CEnvironment::removeInterface(CInterface * interface) {
	dbusEnvironment->removeInterface(dbusInterfaces.key(interface));
}

////////////
//CInterface
////////////
CInterface::CInterface(CEnvironment * parent, QDBusObjectPath dbusPath) : CLibNut(parent), parent(parent), dbusPath(dbusPath) {
	log = parent->log;
	//Attach to dbus
	dbusConnection = parent->dbusConnection;
	dbusConnectionInterface = parent->dbusConnectionInterface;
<<<<<<< HEAD:libnut/libnut_cli.cpp
	dbusInterface = new DBusInterfaceInterface_IPv4(NUT_DBUS_URL, dbusPath.path(), *dbusConnection, this);
=======
	dbusInterface = new DBusInterfaceInterface(NUT_DBUS_URL, dbusPath.path(), *dbusConnection, this);
>>>>>>> 9e958d39c68db61381ac6aad055c2eec25b977ed:libnut/libnut_cli.cpp
	serviceCheck(dbusConnectionInterface);
	//Get properties:
	QDBusReply<InterfaceProperties> replyprops = dbusInterface->getProperties();
	if (replyprops.isValid()) {
		isStatic = replyprops.value().isStatic;
		active = replyprops.value().active;
		userDefineable = replyprops.value().userDefineable;
		ip = replyprops.value().ip;
		netmask = replyprops.value().netmask;
		gateway = replyprops.value().gateway;
		dnsserver = replyprops.value().dns;
	}
	else {
		throw CLI_IfConnectionException(tr("Error while retrieving interface properties") + replyprops.error().name());
	}

	connect(dbusInterface, SIGNAL(stateChanged(const InterfaceProperties &)), this, SLOT(dbusstateChanged(const InterfaceProperties &)));
}
CInterface::~CInterface() {
}
//CInterface private functions:
void CInterface::refreshAll() {
	QDBusReply<InterfaceProperties> replyprops = dbusInterface->getProperties();
	if (replyprops.isValid()) {
		isStatic = replyprops.value().isStatic;
		active = replyprops.value().active;
		userDefineable = replyprops.value().userDefineable;
		ip = replyprops.value().ip;
		netmask = replyprops.value().netmask;
		gateway = replyprops.value().gateway;
		dnsserver = replyprops.value().dns;
	}
	else {
		*log << (tr("Error while refreshing interface at: ") + dbusPath.path());
	}
}
//CInterface private slots
void CInterface::dbusstateChanged(const InterfaceProperties &properties) {
	//Check changes:
	if (properties.active != active) {
		active = properties.active;
		emit(activeChanged(active));
	}
	bool equal = ( (isStatic == properties.isStatic) && (userDefineable == properties.userDefineable) && (ip == properties.ip) && (netmask == properties.netmask) && (gateway == properties.gateway) && (dnsserver == properties.dns));
	if (!equal) {
		isStatic = properties.isStatic;
		userDefineable = properties.userDefineable;
		ip = properties.ip;
		netmask = properties.netmask;
		gateway = properties.gateway;
		emit(ipconfigChanged(isStatic,ip,netmask,gateway));
	}
}
//CInterface SLOTS
void CInterface::activate() {
	dbusInterface->activate();
}
void CInterface::deactivate() {
	dbusInterface->deactivate();
}
void CInterface::setIP(QHostAddress & address) {
	dbusInterface->setIP(address.toIPv4Address());
}
void CInterface::setNetmask(QHostAddress & address) {
	dbusInterface->setNetmask(address.toIPv4Address());
}
void CInterface::setGateway(QHostAddress & address) {
	dbusInterface->setGateway(address.toIPv4Address());
}
void CInterface::setDynamic() {
	if (isStatic) {
		dbusInterface->setDynamic();
	}
}

};
