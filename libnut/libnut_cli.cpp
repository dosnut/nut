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
QString toString(QDBusError error) {
	return QDBusError::errorString(error.type());
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
		emit(stateChanged(true));
	}
	connect(dbusConnectionInterface, SIGNAL(serviceOwnerChanged(const QString &, const QString &, const QString &)), this, SLOT(dbusServiceOwnerChanged(const QString &, const QString &, const QString &)));
	//Connect dbus-signals to own slots:
	connect(dbusDevmgr, SIGNAL(deviceAdded(const QDBusObjectPath&)), this, SLOT(dbusDeviceAdded(const QDBusObjectPath&)));
	connect(dbusDevmgr, SIGNAL(deviceRemoved(const QDBusObjectPath&)), this, SLOT(dbusDeviceRemoved(const QDBusObjectPath&)));
}

//CDeviceManager private functions:
//rebuilds the device list
void CDeviceManager::rebuild(QList<QDBusObjectPath> paths) {
	//Only rebuild when nuts is available
	if (!nutsstate) {
		return;
	}
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
		//Only add device if it's not already in our list;
		if (dbusDevices.contains(i)) {
			continue;
		}
		//Add device
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
				emit(stateChanged(true));
			}
			connect(dbusDevmgr, SIGNAL(deviceAdded(const QDBusObjectPath&)), this, SLOT(dbusDeviceAdded(const QDBusObjectPath&)));
			connect(dbusDevmgr, SIGNAL(deviceRemoved(const QDBusObjectPath&)), this, SLOT(dbusDeviceRemoved(const QDBusObjectPath&)));
		}
		else if (newOwner.isEmpty()) {
			*log << tr("NUTS has been stopped");
			if (nutsstate) {
				nutsstate = false;
				clearInformation();
				emit(stateChanged(false));
			}
			disconnect(dbusDevmgr, SIGNAL(deviceAdded(const QDBusObjectPath&)), this, SLOT(dbusDeviceAdded(const QDBusObjectPath&)));
			disconnect(dbusDevmgr, SIGNAL(deviceRemoved(const QDBusObjectPath&)), this, SLOT(dbusDeviceRemoved(const QDBusObjectPath&)));
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
	//Only refresh when nuts is available
	if (!nutsstate) {
		return;
	}
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
	//Do not rebuild if nuts is not running
	if (!nutsstate) {
		return;
	}
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
CDevice::CDevice(CDeviceManager * parent, QDBusObjectPath dbusPath) : CLibNut(parent), /*parent(parent),*/ dbusPath(dbusPath) {
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

	if (DT_AIR == type) {
		QDBusReply<QString> replyessid = dbusDevice->getEssid();
		if (replyessid.isValid()) {
			essid = replyessid.value();
		}
		else {
			*log << tr("(%1) Could not refresh device essid").arg(toString(replyessid.error()));
			essid = QString();
		}
	}

	//get config and set wpa_supplicant variable
	QDBusReply<nut::DeviceConfig> replyconf = dbusDevice->getConfig();
	if (replyconf.isValid()) {
		dbusConfig = replyconf.value();
		need_wpa_supplicant = !(dbusConfig.wpaConfigFile().isEmpty());
		*log << tr("(%2) wpa_supplicant config file at: %1").arg(dbusConfig.wpaConfigFile(),name);
	}
	else {
		throw CLI_DevConnectionException(tr("(%2) Error(%1) while retrieving device config").arg(replyconf.error().name(),name));
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

	//Only use wpa_supplicant if we need one
	if (need_wpa_supplicant) {
		wpa_supplicant = new CWpa_Supplicant(this,name);
		connect(wpa_supplicant,SIGNAL(message(QString)),log,SLOT(log(QString)));
		//Connect to wpa_supplicant only if device is not deactivated
		if (! (DS_DEACTIVATED == state) ) {
			wpa_supplicant->open();
		}
	}
	else {
		wpa_supplicant = NULL;
	}
}
CDevice::~CDevice() {
	//CWpa_supplicant will be killed as child of CDevices
	CEnvironment * env;
	while (!environments.isEmpty()) {
		env = environments.takeFirst();
// 		emit(environmentsUpdated()); //Pending for removal
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
	if (DT_AIR == type) {
		QDBusReply<QString> replyessid = dbusDevice->getEssid();
		if (replyessid.isValid()) {
			essid = replyessid.value();
		}
		else {
			*log << tr("(%1) Could not refresh device essid").arg(toString(replyessid.error()));
			essid = QString();
		}
	}

	if ( !(DS_DEACTIVATED == state) ) {
		if (need_wpa_supplicant) {
			wpa_supplicant->open();
		}
	}
}
//Rebuilds the environment list
void CDevice::rebuild(QList<QDBusObjectPath> paths) {
	//Remove current list:
	dbusEnvironments.clear();
	CEnvironment * env;
	while ( !environments.isEmpty() ) {
		env = environments.takeFirst();
// 		emit(environmentRemoved(env)); //Pending for removal
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
// 	emit(environmentsUpdated()); //Pending for removal
}

//CDevice private slots:
void CDevice::environmentChangedActive(const QDBusObjectPath &newenv) {
	CEnvironment * oldenv = activeEnvironment;
	activeEnvironment = dbusEnvironments.value(newenv, 0);
	emit(environmentChangedActive(activeEnvironment, oldenv));
}
//Every time our device changed from anything to active, our active environment may have changed
void CDevice::dbusstateChanged(int newState, int oldState) {
	//If switching from DS_DEACTIVATED to any other state then connect wpa_supplicant
	//TODO:fix wpa_supplicant connecting; workaround for now:
	//connect when device has carrier, although this should happen, when device is beeing activated
	if (DS_DEACTIVATED == oldState && !(DS_DEACTIVATED == newState) ) {
		if (need_wpa_supplicant) {
			wpa_supplicant->open();
		}
	}
	else if (DS_DEACTIVATED == newState) {
		if (need_wpa_supplicant) {
			wpa_supplicant->close();
		}
	}
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
	if (DT_AIR == type && !(newState == DS_DEACTIVATED) ) {
		QDBusReply<QString> replyessid = dbusDevice->getEssid();
		if (replyessid.isValid()) {
			essid = replyessid.value();
		}
		else {
			essid = QString();
		}
	}
	else {
		essid = QString();
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
	return dbusConfig;
}

//////////////
//CEnvironment
//////////////

CEnvironment::CEnvironment(CDevice * parent, QDBusObjectPath dbusPath) : CLibNut(parent), /*parent(parent),*/ dbusPath(dbusPath) {
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
	//Get select results
	getSelectResult(true); //functions saves SelectResult
	getSelectResults(true); //functions saves SelectResults

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
		emit(interfacesUpdated());
		delete interface;
	}
}

//CEnvironment private functions

void CEnvironment::refreshAll() {
	//Retrieve properties and select config, then interfaces:
	QDBusReply<EnvironmentProperties> replyprop = dbusEnvironment->getProperties();
	if (replyprop.isValid()) {
		if (static_cast<CDevice *>(parent())->environments[0] == this)
			//standard environment
			name = tr("default");
		else {
			name = replyprop.value().name;
			//environment untitled
			if (name.length() == 0)
				name = tr("untitled (%1)").arg(static_cast<CDevice *>(parent())->environments.size());
		}
	}
	else {
		*log << tr("Error while refreshing environment properties");
	}
	getSelectResult(true); //functions saves SelectResult
	getSelectResults(true); //functions saves SelectResults
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
	getSelectResult(true); //functions saves SelectResult
	getSelectResults(true); //functions saves SelectResults
}

//CEnvironment private slots:
void CEnvironment::dbusstateChanged(bool state) {
	active = state;
	emit(activeChanged(state));
}


//CEnvironment SLOTS
void CEnvironment::enter() {
	static_cast<CDevice *>(parent())->setEnvironment(this);
}
nut::EnvironmentConfig CEnvironment::getConfig() {
	return config;
}

nut::SelectResult CEnvironment::getSelectResult(bool refresh) {
	if (refresh) {
		QDBusReply<nut::SelectResult> reply = dbusEnvironment->getSelectResult();
		if (reply.isValid()) {
			selectResult = reply.value();
		}
		else {
			*log << tr("Error while trying to get SelectResult");
			selectResult = nut::SelectResult();
		}
	}
	return selectResult;
}

QVector<nut::SelectResult> CEnvironment::getSelectResults(bool refresh) {
	if (refresh) {
		QDBusReply<QVector<nut::SelectResult> > reply = dbusEnvironment->getSelectResults();
		if (reply.isValid()) {
			selectResults = reply.value();
		}
		else {
			*log << tr("Error while trying to get SelectResults");
			selectResults = QVector<nut::SelectResult>();
		}
	}
	return selectResults;
}

////////////
//CInterface
////////////
CInterface::CInterface(CEnvironment * parent, QDBusObjectPath dbusPath) : CLibNut(parent), /*parent(parent),*/ dbusPath(dbusPath) {
	log = parent->log;
	//Attach to dbus
	dbusConnection = parent->dbusConnection;
	dbusConnectionInterface = parent->dbusConnectionInterface;
	dbusInterface = new DBusInterfaceInterface_IPv4(NUT_DBUS_URL, dbusPath.path(), *dbusConnection, this);
	serviceCheck(dbusConnectionInterface);
	//Get properties:
	QDBusReply<InterfaceProperties> replyprops = dbusInterface->getProperties();
	if (replyprops.isValid()) {
		state = replyprops.value().ifState;
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
	}
	else {
		throw CLI_IfConnectionException(tr("Error while retrieving interface config") + replyconf.error().name());
	}
	getUserConfig(true); //Function will updated userConfig

	connect(dbusInterface, SIGNAL(stateChanged(const InterfaceProperties &)), this, SLOT(dbusstateChanged(const InterfaceProperties &)));
}
CInterface::~CInterface() {
}
//CInterface private functions:
void CInterface::refreshAll() {
	QDBusReply<InterfaceProperties> replyprops = dbusInterface->getProperties();
	if (replyprops.isValid()) {
		state = replyprops.value().ifState;
		ip = replyprops.value().ip;
		netmask = replyprops.value().netmask;
		gateway = replyprops.value().gateway;
		dnsserver = replyprops.value().dns;
		getUserConfig(true); //Function will updated userConfig
	}
	else {
		*log << (tr("Error while refreshing interface at: ") + dbusPath.path());
	}
}
//CInterface private slots
void CInterface::dbusstateChanged(const InterfaceProperties &properties) {
	//Check changes:
	if (properties.ifState != state) {
		state = properties.ifState;
		emit(stateChanged(state));
	}
	state = properties.ifState;
	ip = properties.ip;
	netmask = properties.netmask;
	gateway = properties.gateway;
	getUserConfig(true); //Function will updated userConfig
	emit(ipconfigChanged(ip,netmask,gateway));
}
//CInterface SLOTS
void CInterface::activate() {
	dbusInterface->activate();
}
void CInterface::deactivate() {
	dbusInterface->deactivate();
}
bool CInterface::needUserSetup() {
	QDBusReply<bool> reply = dbusInterface->needUserSetup();
	if (reply.isValid()) {
		return reply.value();
	}
	else {
		*log << (tr("Error while interface->needUserSetup at: ") + dbusPath.path());
	}
	return false;
}
bool CInterface::setUserConfig(const nut::IPv4UserConfig &cuserConfig) {
	QDBusReply<bool> reply = dbusInterface->setUserConfig(cuserConfig);
	if (reply.isValid()) {
		if (reply.value()) {
			userConfig = cuserConfig;
			return true;
		}
		return false;
	}
	else {
		*log << (tr("Error while interface->setUserConfig at: ") + dbusPath.path());
	}
	return false;
}
nut::IPv4UserConfig CInterface::getUserConfig(bool refresh) {
	if (refresh) {
		QDBusReply<nut::IPv4UserConfig> reply = dbusInterface->getUserConfig();
		if (reply.isValid()) {
			userConfig = reply.value();
		}
		else {
			*log << (tr("Error while interface->getUserConfig at: ") + dbusPath.path());
			userConfig = nut::IPv4UserConfig();
		}
	}
	return userConfig;
}

};
