/*
	TRANSLATOR libnut::QObject
*/
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
QString toString(WlanEncryptionType type) {
	switch (type) {
		case WET_NONE:  return QObject::tr("none");
		case WET_WEP:   return QObject::tr("WEP");
		case WET_WPA1:  return QObject::tr("WPA1");
		case WET_WPA2:  return QObject::tr("WPA2");
		case WET_OTHER: return QObject::tr("Other");
		default:        return QString();
	}
}
QString toString(QDBusError error) {
	return QDBusError::errorString(error.type());
/*
	switch ((int) error) {
		case QDBusError::NoError: return QObject::tr("NoError");
		case QDBusError::Other: return QObject::tr("Other");
		case QDBusError::NoMemory: return QObject::tr("NoMemory");
		case QDBusError::ServiceUnknown: return QObject::tr("ServiceUnknown");
		case QDBusError::NoReply: return QObject::tr("NoReply");
		case QDBusError::BadAddress: return QObject::tr("BadAddress");
		case QDBusError::NotSupported: return QObject::tr("NotSupported");
		case QDBusError::LimitsExceeded: return QObject::tr("LimitsExceeded");
		case QDBusError::AccessDenied: return QObject::tr("AccessDenied");
		case QDBusError::NoServer: return QObject::tr("NoServer");
		case QDBusError::Timeout: return QObject::tr("Timeout");
		case QDBusError::NoNetwork: return QObject::tr("NoNetwork");
		case QDBusError::AddressInUse: return QObject::tr("AddressInUse");
		case QDBusError::Disconnected: return QObject::tr("Disconnected");
		case QDBusError::InvalidArgs: return QObject::tr("InvalidArgs");
		case QDBusError::UnknownMethod: return QObject::tr("UnknownMethod");
		case QDBusError::TimedOut: return QObject::tr("TimedOut");
		case QDBusError::InvalidSignature: return QObject::tr("InvalidSignature");
		case QDBusError::UnknownInterface: return QObject::tr("UnknownInterface");
		case QDBusError::InternalError: return QObject::tr("InternalError");
		case QDBusError::UnknownObject: return QObject::tr("UnknownObject");
		default: return QString();
	}*/
}

////////////////
//CLog
///////////////
CLog::CLog(QObject * parent, QString fileName) : QObject(parent), file(fileName) {
	fileLoggingEnabled = file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
	
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
		throw CLI_ConnectionInitException(tr("(%1)Error while setting-up dbusconnection").arg(toString(reply.error())));
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
	nutsstate = true;
	log = inlog;
	//setup dbus connections
	dbusConnectionInterface = dbusConnection.interface();
	//Check if service is running
	try {
		serviceCheck(dbusConnectionInterface);
	}
	catch (CLI_ConnectionInitException& e) {
		*log << tr("Please start nuts. Starting idle mode");
		nutsstate = false;
	}
	//Attach to DbusDevicemanager
	dbusDevmgr = new DBusDeviceManagerInterface(NUT_DBUS_URL, "/manager",dbusConnection, this);
	if (nutsstate) {
		setInformation();
	}
	connect(dbusConnectionInterface, SIGNAL(serviceOwnerChanged(const QString &, const QString &, const QString &)), this, SLOT(dbusServiceOwnerChanged(const QString &, const QString &, const QString &)));
	//Connect dbus-signals to own slots:
//	connect(dbusDevmgr, SIGNAL(deviceAdded(const QDBusObjectPath&)), this, SLOT(dbusDeviceAdded(const QDBusObjectPath&)));
//	connect(dbusDevmgr, SIGNAL(deviceRemoved(const QDBusObjectPath&)), this, SLOT(dbusDeviceRemoved(const QDBusObjectPath&)));
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

void CDeviceManager::setInformation() {
	//get devicelist etc.
	*log << "setInformation()";
	QDBusReply<QList<QDBusObjectPath> > replydevs;
	replydevs = dbusDevmgr->getDeviceList();
	if (!replydevs.isValid()) {
		*log << tr("(%1) Failed to get DeviceList").arg(toString(replydevs.error()));
	}
	//Let's populate our own DeviceList
	CDevice * device;
	foreach (QDBusObjectPath i, replydevs.value()) {
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
}
void CDeviceManager::clearInformation() {
	//Clean Device list:
	dbusDevices.clear();
	CDevice * dev;
	while (!devices.isEmpty()) {
		dev = devices.takeFirst();
		emit(deviceRemoved(dev));
		delete dev;
	}
}

//CDeviceManager private slots:
void CDeviceManager::dbusServiceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner) {
	if (NUT_DBUS_URL == name) {
		if (oldOwner.isEmpty()) {
			*log << tr("NUTS has been started");
			if (nutsstate) {
				clearInformation();
				setInformation();
			}
			else {
				nutsstate = true;
				setInformation();
			}
/*			connect(dbusDevmgr, SIGNAL(deviceAdded(const QDBusObjectPath&)), this, SLOT(dbusDeviceAdded(const QDBusObjectPath&)));
			connect(dbusDevmgr, SIGNAL(deviceRemoved(const QDBusObjectPath&)), this, SLOT(dbusDeviceRemoved(const QDBusObjectPath&)));*/
		}
		else if (newOwner.isEmpty()) {
			*log << tr("NUTS has been stopped");
			if (nutsstate) {
				nutsstate = false;
				clearInformation();
			}
/*			disconnect(dbusDevmgr, SIGNAL(deviceAdded(const QDBusObjectPath&)), this, SLOT(dbusDeviceAdded(const QDBusObjectPath&)));
			disconnect(dbusDevmgr, SIGNAL(deviceRemoved(const QDBusObjectPath&)), this, SLOT(dbusDeviceRemoved(const QDBusObjectPath&)));*/
 		}
	}
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
		*log << tr("(%1) Could not refresh device list").arg(toString(replydevs.error()));
	}
}

void CDeviceManager::rebuild() {
	QDBusReply<QList<QDBusObjectPath> > replydevs = dbusDevmgr->getDeviceList();
	if (replydevs.isValid()) {
		rebuild(replydevs.value());
	}
	else {
		*log << tr("(%1) Error while retrieving device list").arg(toString(replydevs.error()));
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
		*log << (tr("Device properties fetched"));
		*log << (tr("Name") + ": " + QString(name));
		*log << (tr("Type") + ": " + toString(type));
		*log << (tr("State") + ": " + toString(state));
	}
	else {
		throw CLI_DevConnectionException(tr("(%1) Error while retrieving dbus' device information").arg(toString(replyProp.error())));
	}
	//get config
	QDBusReply<nut::DeviceConfig> replyconf = dbusDevice->getConfig();
	if (replyconf.isValid()) {
		config = replyconf.value();
	}
	else {
		throw CLI_DevConnectionException(tr("Error while retrieving device config") + replyconf.error().name());
	}

	//get Environment list
	QDBusReply<QList<QDBusObjectPath> > replyEnv = dbusDevice->getEnvironments();
	if (!replyEnv.isValid()) {
		throw CLI_DevConnectionException(tr("(%1) Error while trying to get environment list").arg(toString(replyEnv.error())));
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
	}
	
	if (!replyProp.value().activeEnvironment.isEmpty()) {
		dbusActiveEnvironment = QDBusObjectPath(replyProp.value().activeEnvironment);
		activeEnvironment = dbusEnvironments.value(dbusActiveEnvironment, 0);
		*log << (tr("Active Environement") + ": " + dbusActiveEnvironment.path());
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
	
// 	wpa_supplicant = new CWpa_Supplicant(this);
// 	connect(wpa_supplicant,SIGNAL(message(QString)),log,SLOT(log(QString)));
// 	wpa_supplicant->wps_open();
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
 *  refreshAll rebuilds the device's environment list if any environmentpath has changed, otherwise
 *  it will just refresh the information
 */
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
		*log << tr("(%1) Could not refresh environments").arg(toString(replyenvs.error()));
	}
	activeEnvironment = 0;
	//now refresh the rest of our device properties:
	QDBusReply<DeviceProperties> replyprop = dbusDevice->getProperties();
	if (replyprop.isValid()) {
		if (!replyprop.value().activeEnvironment.isEmpty()) {
			dbusActiveEnvironment = QDBusObjectPath(replyprop.value().activeEnvironment);
			activeEnvironment = dbusEnvironments.value(dbusActiveEnvironment);
		}
		*log << "Refreshing active environment: " + dbusActiveEnvironment.path();
		state = (DeviceState) replyprop.value().state;
		type = replyprop.value().type;
		name = replyprop.value().name;
	}
	else {
		*log << tr("(%1) Could not refresh device properties").arg(toString(replyprop.error()));
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
	activeEnvironment = dbusEnvironments.value(newenv, 0);
	emit(environmentChangedActive(activeEnvironment, oldenv));
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
//Every time our device changed from anything to active, our active environment may have changed
void CDevice::dbusstateChanged(int newState, int oldState) {
	state = (DeviceState) newState;
	//Workaround so far, as nuts does not send any environment changed information
	if (state == DS_UP) {
		QDBusReply<libnut::DeviceProperties> replyprop = dbusDevice->getProperties();
		if (replyprop.isValid()) {
			dbusActiveEnvironment = QDBusObjectPath(replyprop.value().activeEnvironment);
			activeEnvironment = dbusEnvironments.value(dbusActiveEnvironment, 0);
			if (activeEnvironment != 0) {
				activeEnvironment->refreshAll();
			}
		}
	}
	else {
		dbusActiveEnvironment = QDBusObjectPath();
		activeEnvironment = 0;
	}
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
	if (DS_DEACTIVATED == state) {
		enable();
	}
	dbusDevice->setEnvironment(dbusEnvironments.key(environment));
}
nut::DeviceConfig CDevice::getConfig() {
	return config;
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

	//get Environment properties
	QDBusReply<EnvironmentProperties> replyprop = dbusEnvironment->getProperties();
	if (replyprop.isValid()) {
		if (parent->environments.size() == 0)
			//standard environment
			name = tr("default");
		else {
			name = replyprop.value().name;
			//environment untitled
			if (name.length() == 0)
				name = tr("untitled (%1)").arg(parent->environments.size());
		}
		*log << "Environmentname" + name;
	}
	else {
		throw CLI_EnvConnectionException(tr("Error while retrieving environment properties").arg(toString(replyprop.error())));
	}
	//Get environment config
	QDBusReply<nut::EnvironmentConfig> replyconf = dbusEnvironment->getConfig();
	if (replyconf.isValid()) {
		config = replyconf.value();
	}
	else {
		throw CLI_EnvConnectionException(tr("Error while retrieving environment config") + replyconf.error().name());
	}
	//Get Interfaces
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
		if (parent->environments[0] == this)
			//standard environment
			name = tr("default");
		else {
			name = replyprop.value().name;
			//environment untitled
			if (name.length() == 0)
				name = tr("untitled (%1)").arg(parent->environments.size());
		}
	}
	else {
		*log << tr("Error while refreshing environment properties");
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
nut::EnvironmentConfig CEnvironment::getConfig() {
	return config;
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
	dbusInterface = new DBusInterfaceInterface_IPv4(NUT_DBUS_URL, dbusPath.path(), *dbusConnection, this);
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
	//Get Config
	QDBusReply<nut::IPv4Config> replyconf = dbusInterface->getConfig();
	if (replyconf.isValid()) {
		dbusConfig = replyconf.value();
		userDefineable = (dbusConfig.getFlags() & nut::IPv4Config::MAY_USERSTATIC);
		//set config for clients
		
		//Set Configflags
		if (isStatic & (dbusConfig.getFlags() == nut::IPv4Config::DO_DHCP)) {
			config.flags = IF_FALLBACK;
		}
		else {
			config.flags = (InterfaceFlags) dbusConfig.getFlags();
		}
		
		//
		if (isStatic) {
			config.staticIp = dbusConfig.getStaticIP();
			config.staticNetmask = dbusConfig.getStaticNetmask();
			config.staticGateway = dbusConfig.getStaticGateway();
		}
	}
	else {
		throw CLI_IfConnectionException(tr("Error while retrieving interface config") + replyconf.error().name());
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
nut::IPv4Config CInterface::getConfig() {
	return dbusConfig;
}

};
