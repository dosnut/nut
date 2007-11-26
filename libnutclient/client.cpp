/*
	TRANSLATOR libnutclient::QObject
*/
#include "client.h"
#include "libnutcommon/common.h"
#include <QDebug>

//things that may need to be changed:
//-Check if dev/env/if is already in list (maybe refresh then)
//-Check if we can remove dev/env/if
//-When refreshing: send Changesignals?
//-wlan sach
//-more debugging output
namespace libnutclient {
using namespace libnutcommon;
QString toStringTr(DeviceState state) {
	switch (state) {
		case DS_UP:             return QObject::tr("up");
		case DS_UNCONFIGURED:   return QObject::tr("unconfigured");
		case DS_CARRIER:        return QObject::tr("got carrier");
		case DS_ACTIVATED:      return QObject::tr("activated");
		case DS_DEACTIVATED:    return QObject::tr("deactivated");
		default:                return QString();
	}
}
QString toStringTr(DeviceType type) {
	switch (type) {
		case DT_ETH: return QObject::tr("Ethernet");
		case DT_AIR: return QObject::tr("Wireless");
		case DT_PPP: return QObject::tr("PPP");
		default:     return QString();
	}
}
QString toStringTr(InterfaceState state) {
	switch (state) {
		case IFS_OFF: return QObject::tr("off");
		case IFS_STATIC: return QObject::tr("static");
		case IFS_DHCP: return QObject::tr("dynamic");
		case IFS_ZEROCONF: return QObject::tr("zeroconf");
		case IFS_WAITFORCONFIG: return QObject::tr("wait for config");
		default: return QString();
	}
}
QString toString(QDBusError error) {
	return QDBusError::errorString(error.type());
}

////////////////
//CLog
///////////////
CLog::CLog(QObject * parent, QString fileName) : QObject(parent), m_file(fileName) {
	m_fileLoggingEnabled = m_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
	
	if (m_fileLoggingEnabled) {
		m_outStream.setDevice(&m_file);
	}
}

void CLog::operator<<(QString text) {
	emit printed(text);
	
	if (m_fileLoggingEnabled) {
		m_outStream << text << endl;
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
CDeviceManager::CDeviceManager(QObject * parent) : CLibNut(parent), m_dbusConnection(QDBusConnection::systemBus()) {
	//Init Hashtable
	m_dbusDevices.reserve(10);
}
CDeviceManager::~CDeviceManager() {
//Cleanup action: delete entire devicelist
	CDevice * device;
	while (!devices.isEmpty()) {
		device = devices.takeFirst();
		delete device;
	}
}

bool CDeviceManager::init(CLog * inlog) {
	m_nutsstate = true;
	log = inlog;
	//setup dbus connections
	m_dbusConnectionInterface = m_dbusConnection.interface();
	//Check if service is running
	try {
		serviceCheck(m_dbusConnectionInterface);
	}
	catch (CLI_ConnectionInitException& e) {
		*log << tr("Please start nuts. Starting idle mode");
		m_nutsstate = false;
	}
	//Attach to DbusDevicemanager
	m_dbusDevmgr = new DBusDeviceManagerInterface(NUT_DBUS_URL, "/manager",m_dbusConnection, this);
	if (m_nutsstate) {
		setInformation();
		emit(stateChanged(true));
	}
	connect(m_dbusConnectionInterface, SIGNAL(serviceOwnerChanged(const QString &, const QString &, const QString &)), this, SLOT(dbusServiceOwnerChanged(const QString &, const QString &, const QString &)));
	//Connect dbus-signals to own slots:
	connect(m_dbusDevmgr, SIGNAL(deviceAdded(const QDBusObjectPath&)), this, SLOT(dbusDeviceAdded(const QDBusObjectPath&)));
	connect(m_dbusDevmgr, SIGNAL(deviceRemoved(const QDBusObjectPath&)), this, SLOT(dbusDeviceRemoved(const QDBusObjectPath&)));
	return m_nutsstate;
}

//CDeviceManager private functions:
//rebuilds the device list
void CDeviceManager::rebuild(QList<QDBusObjectPath> paths) {
	//Only rebuild when nuts is available
	if (!m_nutsstate) {
		return;
	}
	//Delete all devices
	m_dbusDevices.clear();
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
			qWarning() << e.what();
			continue;
		}
		m_dbusDevices.insert(i, device);
		devices.append(device);
		device->index = devices.indexOf(device); // Set device index;
		emit(deviceAdded(device));
	}
}

void CDeviceManager::setInformation() {
	//get devicelist etc.
	qDebug() << "setInformation()";
	QDBusReply<QList<QDBusObjectPath> > replydevs;
	replydevs = m_dbusDevmgr->getDeviceList();
	if (!replydevs.isValid()) {
		qWarning() << tr("(%1) Failed to get DeviceList").arg(toString(replydevs.error()));
	}
	//Let's populate our own DeviceList
	
	CDevice * device;
	foreach (QDBusObjectPath i, replydevs.value()) {
		//Only add device if it's not already in our list;
		if (m_dbusDevices.contains(i)) {
			continue;
		}
		//Add device
		try {
			device = new CDevice(this, i);
		}
		catch (CLI_DevConnectionException e) {
			qWarning() << e.msg();
			continue;
		}
		devices.append(device);
		device->index = devices.indexOf(device); // Set device index;
		m_dbusDevices.insert(i, device);
		emit(deviceAdded(device));
	}
}
void CDeviceManager::clearInformation() {
	//Clean Device list:
	m_dbusDevices.clear();
	CDevice * dev;
	while (!devices.isEmpty()) {
		dev = devices.takeFirst();
		emit(deviceRemoved(dev));
		delete dev;
	}
}

//CDeviceManager private slots:
void CDeviceManager::dbusServiceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner) {
	if (NUT_DBUS_URL == name) { //nuts starts
		if (oldOwner.isEmpty()) {
			*log << tr("NUTS has been started");
			if (m_nutsstate) {
				clearInformation();
				setInformation();
			}
			else {
				m_nutsstate = true;
				setInformation();
				emit(stateChanged(true));
			}
			connect(m_dbusDevmgr, SIGNAL(deviceAdded(const QDBusObjectPath&)), this, SLOT(dbusDeviceAdded(const QDBusObjectPath&)));
			connect(m_dbusDevmgr, SIGNAL(deviceRemoved(const QDBusObjectPath&)), this, SLOT(dbusDeviceRemoved(const QDBusObjectPath&)));
		}
		else if (newOwner.isEmpty()) { //nuts stops
			*log<< tr("NUTS has been stopped");
			if (m_nutsstate) {
				m_nutsstate = false;
				clearInformation();
				emit(stateChanged(false));
			}
			disconnect(m_dbusDevmgr, SIGNAL(deviceAdded(const QDBusObjectPath&)), this, SLOT(dbusDeviceAdded(const QDBusObjectPath&)));
			disconnect(m_dbusDevmgr, SIGNAL(deviceRemoved(const QDBusObjectPath&)), this, SLOT(dbusDeviceRemoved(const QDBusObjectPath&)));
 		}
	}
}

//CDeviceManager DBUS-SLOTS:
void CDeviceManager::dbusDeviceAdded(const QDBusObjectPath &objectpath) {
	if (!m_dbusDevices.contains(objectpath)) {
		*log << tr("Adding device at: %1").arg(objectpath.path());
		CDevice * device;
		try {
			device = new CDevice(this, objectpath);
		}
		catch (CLI_DevConnectionException e) {
			qWarning() << e.msg();
			return;
		}
		m_dbusDevices.insert(objectpath,device);
		devices.append(device);
		device->index = devices.indexOf(device); // Set device index;
		emit(deviceAdded(device));
		}
	else {
		m_dbusDevices.value(objectpath)->refreshAll();
	}
}
void CDeviceManager::dbusDeviceRemoved(const QDBusObjectPath &objectpath) {
	//remove devices from devicelist
	if (m_dbusDevices.value(objectpath)->m_lockCount == 0) {
		CDevice * device = m_dbusDevices.take(objectpath);
		devices.removeAll(device);
		//Rebuild device->index for qnut
		foreach(CDevice * dev, devices) {
			dev->index = devices.indexOf(dev);
		}
		emit(deviceRemoved(device));
		device->deleteLater();
	}
	else {
		m_dbusDevices.value(objectpath)->m_pendingRemoval = true;
	}
}


//CDeviceManager SLOTS
void CDeviceManager::refreshAll() {
	//Only refresh when nuts is available
	if (!m_nutsstate) {
		return;
	}
	//Get our current DeviceList
	QDBusReply<QList<QDBusObjectPath> > replydevs = m_dbusDevmgr->getDeviceList();
	if (replydevs.isValid()) {
		//If a device is missing or there are too many devices in our Hash
		//Then rebuild complete tree, otherwise just call refresh on child
		bool equal = true;
		if ( (m_dbusDevices.size() == replydevs.value().size()) ) {
			foreach(QDBusObjectPath i, replydevs.value()) {
				if ( !m_dbusDevices.contains(i) ) {
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
		qWarning() << tr("(%1) Could not refresh device list").arg(toString(replydevs.error()));
	}
}

void CDeviceManager::rebuild() {
	//Do not rebuild if nuts is not running
	if (!m_nutsstate) {
		return;
	}
	QDBusReply<QList<QDBusObjectPath> > replydevs = m_dbusDevmgr->getDeviceList();
	if (replydevs.isValid()) {
		rebuild(replydevs.value());
	}
	else {
		qWarning() << tr("(%1) Error while retrieving device list").arg(toString(replydevs.error()));
	}
}



/////////
//CDevice
/////////
CDevice::CDevice(CDeviceManager * parent, QDBusObjectPath m_dbusPath) : CLibNut(parent), /*parent(parent),*/ m_dbusPath(m_dbusPath), m_pendingRemoval(false), m_lockCount(0) {
	log = parent->log;
	//get m_dbusConnection from parent:
	m_dbusConnection = &(parent->m_dbusConnection);
	m_dbusConnectionInterface = parent->m_dbusConnectionInterface;
	//Service check
	serviceCheck(m_dbusConnectionInterface);
	//connect to dbus-object
	m_dbusDevice = new DBusDeviceInterface(NUT_DBUS_URL, m_dbusPath.path(),*m_dbusConnection, this);
	
	//get properties
	*log << tr("Getting device properties at: %1").arg(m_dbusPath.path());
	QDBusReply<DeviceProperties> replyProp = m_dbusDevice->getProperties();
	if (replyProp.isValid()) {
		name = replyProp.value().name;
		type = replyProp.value().type;
		state = (DeviceState) replyProp.value().state;
		activeEnvironment = 0;
		*log << tr("Device properties fetched");
		*log << tr("Name : %1").arg(name);
		*log << tr("Type: %1").arg(libnutclient::toStringTr(type));
		*log << tr("State: %1").arg(libnutclient::toStringTr(state));
	}
	else {
		throw CLI_DevConnectionException(tr("(%1) Error while retrieving dbus' device information").arg(toString(replyProp.error())));
	}

	if (DT_AIR == type) {
		QDBusReply<QString> replyessid = m_dbusDevice->getEssid();
		if (replyessid.isValid()) {
			essid = replyessid.value();
		}
		else {
			qWarning() << tr("(%1) Could not refresh device essid").arg(toString(replyessid.error()));
			essid = QString();
		}
	}

	//get config and set wpa_supplicant variable
	QDBusReply<libnutcommon::DeviceConfig> replyconf = m_dbusDevice->getConfig();
	if (replyconf.isValid()) {
		m_config = replyconf.value();
		m_needWpaSupplicant = !(m_config.wpaConfigFile().isEmpty());
		*log << tr("(%2) wpa_supplicant config file at: %1").arg(m_config.wpaConfigFile(),name);
	}
	else {
		throw CLI_DevConnectionException(tr("(%2) Error(%1) while retrieving device config").arg(replyconf.error().name(),name));
	}

	//get Environment list
	QDBusReply<QList<QDBusObjectPath> > replyEnv = m_dbusDevice->getEnvironments();
	if (!replyEnv.isValid()) {
		throw CLI_DevConnectionException(tr("(%1) Error while trying to get environment list").arg(toString(replyEnv.error())));
	}
	//poppulate own Environmentlist
	CEnvironment * env;
	foreach(QDBusObjectPath i, replyEnv.value()) {
		*log << tr("Adding Environment at: %1").arg(i.path());
		try {
			env = new CEnvironment(this,i);
		}
		catch (CLI_EnvConnectionException e) {
			qWarning() << e.msg();
			continue;
		}
		m_dbusEnvironments.insert(i,env);
		environments.append(env);
		env->index = environments.indexOf(env);
	}
	
	if (!replyProp.value().activeEnvironment.isEmpty()) {
		m_dbusActiveEnvironment = QDBusObjectPath(replyProp.value().activeEnvironment);
		activeEnvironment = m_dbusEnvironments.value(m_dbusActiveEnvironment, 0);
		*log << tr("Active Environement: %1").arg(m_dbusActiveEnvironment.path());
	}
	else {
		activeEnvironment = 0;
	}
	//connect signals to slots
	connect(m_dbusDevice, SIGNAL(environmentChangedActive(const QString &)),
			this, SLOT(environmentChangedActive(const QString &)));

	connect(m_dbusDevice, SIGNAL(stateChanged(int , int)),
			this, SLOT(dbusstateChanged(int, int)));

	//Only use wpa_supplicant if we need one
	if (m_needWpaSupplicant) {
		wpa_supplicant = new libnutwireless::CWpa_Supplicant(this,name);
		connect(wpa_supplicant,SIGNAL(message(QString)),log,SLOT(log(QString)));
		//Connect to wpa_supplicant only if device is not deactivated
		if (! (DS_DEACTIVATED == state) ) {
			wpa_supplicant->open();
		}
		if (DT_AIR == type && DS_CARRIER <= state && wpa_supplicant != NULL) {
			wpa_supplicant->setSignalQualityPollRate(500);
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
	QDBusReply<QList<QDBusObjectPath> > replyenvs = m_dbusDevice->getEnvironments();
	if (replyenvs.isValid()) {
		//Compare local with remote list:
		//if they are equal just refresh otherwise rebuild
		bool envequal = (m_dbusEnvironments.size() == replyenvs.value().size());
		if (envequal) {
			foreach(QDBusObjectPath i, replyenvs.value()) {
				if ( !m_dbusEnvironments.contains(i) ) {
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
		qWarning() << tr("(%1) Could not refresh environments").arg(toString(replyenvs.error()));
	}
	activeEnvironment = 0;
	//now refresh the rest of our device properties:
	QDBusReply<DeviceProperties> replyprop = m_dbusDevice->getProperties();
	if (replyprop.isValid()) {
		if (!replyprop.value().activeEnvironment.isEmpty()) {
			m_dbusActiveEnvironment = QDBusObjectPath(replyprop.value().activeEnvironment);
			activeEnvironment = m_dbusEnvironments.value(m_dbusActiveEnvironment);
		}
		*log << tr("Refreshing active environment: %1").arg(m_dbusActiveEnvironment.path());
		state = (DeviceState) replyprop.value().state;
		type = replyprop.value().type;
		name = replyprop.value().name;
	}
	else {
		qWarning() << tr("(%1) Could not refresh device properties").arg(toString(replyprop.error()));
	}
	if (DT_AIR == type) {
		QDBusReply<QString> replyessid = m_dbusDevice->getEssid();
		if (replyessid.isValid()) {
			essid = replyessid.value();
		}
		else {
			qWarning() << tr("(%1) Could not refresh device essid").arg(toString(replyessid.error()));
			essid = QString();
		}
	}

	if ( !(DS_DEACTIVATED == state) ) {
		if (m_needWpaSupplicant) {
			wpa_supplicant->open();
		}
	}
	if (DT_AIR == type && DS_CARRIER <= state && wpa_supplicant != NULL) {
		wpa_supplicant->setSignalQualityPollRate(500);
	}
}
//Rebuilds the environment list
void CDevice::rebuild(QList<QDBusObjectPath> paths) {
	//Remove current list:
	m_dbusEnvironments.clear();
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
			qWarning() << e.what();
			continue;
		}
		m_dbusEnvironments.insert(i,env);
		environments.append(env);
		env->index = environments.indexOf(env);
	}
// 	emit(environmentsUpdated()); //Pending for removal
}


//Locking functions
bool CDevice::incrementLock() {
	if (m_pendingRemoval) {
		if (0 == m_lockCount) {
			static_cast<CDeviceManager* >(parent())->dbusDeviceRemoved(m_dbusPath);
		}
		return false;
	}
	else {
		m_lockCount++;
		return true;
	}
}
void CDevice::decrementLock() {
	if (m_lockCount > 0) {
		m_lockCount--;
	}
	else {
		*log << "ERROR: LOCK-COUNT<0";
	}
	if ( (m_pendingRemoval) && (0 == m_lockCount) ){
		static_cast<CDeviceManager* >(parent())->dbusDeviceRemoved(m_dbusPath);
	}
}

//CDevice private slots:
void CDevice::environmentChangedActive(const QString &newenv) {
	CEnvironment * oldenv = activeEnvironment;
	if (newenv.isEmpty()) { //No active Environment is set.
		activeEnvironment = NULL;
	}
	else {
		activeEnvironment = m_dbusEnvironments.value(QDBusObjectPath(newenv), 0);
	}
	emit(environmentChangedActive(activeEnvironment, oldenv));
}
//Every time our device changed from anything to active, our active environment may have changed
void CDevice::dbusstateChanged(int newState, int oldState) {
	//If switching from DS_DEACTIVATED to any other state then connect wpa_supplicant
	//TODO:fix wpa_supplicant connecting; workaround for now:
	//connect when device has carrier, although this should happen, when device is beeing activated
	if (DS_DEACTIVATED == oldState && !(DS_DEACTIVATED == newState) ) {
		if (m_needWpaSupplicant) {
			wpa_supplicant->open();
		}
	}
	else if (DS_DEACTIVATED == newState) {
		if (m_needWpaSupplicant) {
			wpa_supplicant->close();
		}
	}
	state = (DeviceState) newState;
	//Workaround so far, as nuts does not send any environment changed information
	if (DT_AIR == type && !(newState == DS_DEACTIVATED) ) {
		QDBusReply<QString> replyessid = m_dbusDevice->getEssid();
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
	if (DT_AIR == type && DS_CARRIER <= state && wpa_supplicant != NULL) {
		wpa_supplicant->setSignalQualityPollRate(500);
	}
	emit(stateChanged(state));
}


//CDevice SLOTS
void CDevice::enable() {
	m_dbusDevice->enable();
}
void CDevice::disable() {
	m_dbusDevice->disable();
}
void CDevice::setEnvironment(CEnvironment * environment) {
	if (DS_DEACTIVATED == state) {
		enable();
	}
	m_dbusDevice->setEnvironment(m_dbusEnvironments.key(environment));
}
libnutcommon::DeviceConfig& CDevice::getConfig() {
	return m_config;
}

//////////////
//CEnvironment
//////////////

CEnvironment::CEnvironment(CDevice * parent, QDBusObjectPath dbusPath) : CLibNut(parent), /*parent(parent),*/ m_dbusPath(dbusPath) {
	//Set log.
	log = parent->log;
	active = false;
	//First attach to dbus
	m_dbusConnection = parent->m_dbusConnection;
	m_dbusConnectionInterface = parent->m_dbusConnectionInterface;
	serviceCheck(m_dbusConnectionInterface);
	m_dbusEnvironment = new DBusEnvironmentInterface(NUT_DBUS_URL, m_dbusPath.path(),*m_dbusConnection,this);

	//get Environment properties
	QDBusReply<EnvironmentProperties> replyprop = m_dbusEnvironment->getProperties();
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
		active = replyprop.value().active;
	}
	else {
		throw CLI_EnvConnectionException(tr("Error while retrieving environment properties").arg(toString(replyprop.error())));
	}
	//Get environment config
	QDBusReply<libnutcommon::EnvironmentConfig> replyconf = m_dbusEnvironment->getConfig();
	if (replyconf.isValid()) {
		m_config = replyconf.value();
	}
	else {
		throw CLI_EnvConnectionException(tr("(%1) Error while retrieving environment config").arg(replyconf.error().name()));
	}
	//Get select results
	getSelectResult(true); //functions saves SelectResult
	getSelectResults(true); //functions saves SelectResults

	//Get Interfaces
 	QDBusReply<QList<QDBusObjectPath> > replyifs = m_dbusEnvironment->getInterfaces();
	if (replyifs.isValid()) {
		CInterface * interface;
		foreach(QDBusObjectPath i, replyifs.value()) {
			try {
				interface = new CInterface(this,i);
			}
			catch (CLI_ConnectionException &e) {
				qWarning() << e.what();
				continue;
			}
			m_dbusInterfaces.insert(i,interface);
			interfaces.append(interface);
			interface->index = interfaces.indexOf(interface);
		}
	}
	else {
		throw CLI_EnvConnectionException(tr("Error while retrieving environment's interfaces"));
	}
	connect(m_dbusEnvironment, SIGNAL(stateChanged(bool )), this, SLOT(dbusstateChanged(bool )));
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
	QDBusReply<EnvironmentProperties> replyprop = m_dbusEnvironment->getProperties();
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
		qWarning() << tr("Error while refreshing environment properties");
	}
	getSelectResult(true); //functions saves SelectResult
	getSelectResults(true); //functions saves SelectResults
	QDBusReply<QList<QDBusObjectPath> > replyifs = m_dbusEnvironment->getInterfaces();
	if (replyifs.isValid()) {
		//Check if we need to rebuild the interface list or just refresh them:
		bool ifequal = (replyifs.value().size() == m_dbusInterfaces.size());
		if (ifequal) {
			foreach(QDBusObjectPath i, replyifs.value()) {
				if (!m_dbusInterfaces.contains(i)) {
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
		qWarning() << tr("Error while refreshing environment interfaces");
	}
}
void CEnvironment::rebuild(const QList<QDBusObjectPath> &paths) {
	//Remove all interfaces
	m_dbusInterfaces.clear();
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
			qWarning() << e.what();
			continue;
		}
		m_dbusInterfaces.insert(i,interface);
		interfaces.append(interface);
		interface->index = interfaces.indexOf(interface);
	}
	getSelectResult(true); //functions saves SelectResult
	getSelectResults(true); //functions saves SelectResults
}

//CEnvironment private slots:
void CEnvironment::dbusstateChanged(bool state) {
	active = state;
	emit(activeChanged(state));
}

void CEnvironment::dbusselectResultChanged(libnutcommon::SelectResult result, QVector<libnutcommon::SelectResult> results) {
	m_selectResult = result;
	m_selectResults = results;
	emit selectResultsChanged();
}


//CEnvironment SLOTS
void CEnvironment::enter() {
	static_cast<CDevice *>(parent())->setEnvironment(this);
}
libnutcommon::EnvironmentConfig& CEnvironment::getConfig() {
	return m_config;
}

libnutcommon::SelectResult& CEnvironment::getSelectResult(bool refresh) {
	if (refresh) {
		QDBusReply<libnutcommon::SelectResult> reply = m_dbusEnvironment->getSelectResult();
		if (reply.isValid()) {
			m_selectResult = reply.value();
		}
		else {
			qWarning() << tr("(%1) Error while trying to get SelectResult").arg(toString(reply.error()));
			m_selectResult = libnutcommon::SelectResult();
		}
	}
	return m_selectResult;
}

QVector<libnutcommon::SelectResult>& CEnvironment::getSelectResults(bool refresh) {
	if (refresh) {
		QDBusReply<QVector<libnutcommon::SelectResult> > reply = m_dbusEnvironment->getSelectResults();
		if (reply.isValid()) {
			m_selectResults = reply.value();
		}
		else {
			qWarning() << tr("(%1) Error while trying to get SelectResults").arg(toString(reply.error()));
			m_selectResults = QVector<libnutcommon::SelectResult>();
		}
	}
	return m_selectResults;
}

////////////
//CInterface
////////////
CInterface::CInterface(CEnvironment * parent, QDBusObjectPath dbusPath) : CLibNut(parent), /*parent(parent),*/ m_dbusPath(dbusPath) {
	log = parent->log;
	//Attach to dbus
	m_dbusConnection = parent->m_dbusConnection;
	m_dbusConnectionInterface = parent->m_dbusConnectionInterface;
	m_dbusInterface = new DBusInterfaceInterface_IPv4(NUT_DBUS_URL, m_dbusPath.path(), *m_dbusConnection, this);
	serviceCheck(m_dbusConnectionInterface);
	//Get properties:
	QDBusReply<InterfaceProperties> replyprops = m_dbusInterface->getProperties();
	if (replyprops.isValid()) {
		state = replyprops.value().ifState;
		ip = replyprops.value().ip;
		netmask = replyprops.value().netmask;
		gateway = replyprops.value().gateway;
		dnsserver = replyprops.value().dns;
	}
	else {
		throw CLI_IfConnectionException(tr("(%1) Error while retrieving interface properties").arg(replyprops.error().name()));
	}
	//Get Config
	QDBusReply<libnutcommon::IPv4Config> replyconf = m_dbusInterface->getConfig();
	if (replyconf.isValid()) {
		m_config = replyconf.value();
	}
	else {
		throw CLI_IfConnectionException(tr("(%1) Error while retrieving interface config").arg(replyconf.error().name()));
	}
	getUserConfig(true); //Function will updated userConfig

	connect(m_dbusInterface, SIGNAL(stateChanged(libnutcommon::InterfaceProperties)), this, SLOT(dbusstateChanged(libnutcommon::InterfaceProperties)));
}
CInterface::~CInterface() {
}
//CInterface private functions:
void CInterface::refreshAll() {
	QDBusReply<InterfaceProperties> replyprops = m_dbusInterface->getProperties();
	if (replyprops.isValid()) {
		state = replyprops.value().ifState;
		ip = replyprops.value().ip;
		netmask = replyprops.value().netmask;
		gateway = replyprops.value().gateway;
		dnsserver = replyprops.value().dns;
		getUserConfig(true); //Function will updated userConfig
	}
	else {
		qWarning() << (tr("Error while refreshing interface at: %1").arg(m_dbusPath.path()));
	}
}
//CInterface private slots
void CInterface::dbusstateChanged(libnutcommon::InterfaceProperties properties) {
	//Check changes:
	state = properties.ifState;
	ip = properties.ip;
	netmask = properties.netmask;
	gateway = properties.gateway;
	dnsserver = properties.dns;
	getUserConfig(true); //Function will updated userConfig
	*log << tr("Interface state of %1 has changed to %2").arg(m_dbusPath.path(),libnutclient::toStringTr(state));
	emit(stateChanged(state));
}
//CInterface SLOTS
void CInterface::activate() {
	m_dbusInterface->activate();
}
void CInterface::deactivate() {
	m_dbusInterface->deactivate();
}
bool CInterface::needUserSetup() {
	QDBusReply<bool> reply = m_dbusInterface->needUserSetup();
	if (reply.isValid()) {
		return reply.value();
	}
	else {
		qWarning() << tr("Error while interface->needUserSetup at: %1").arg(m_dbusPath.path());
	}
	return false;
}
bool CInterface::setUserConfig(const libnutcommon::IPv4UserConfig &cuserConfig) {
	QDBusReply<bool> reply = m_dbusInterface->setUserConfig(cuserConfig);
	if (reply.isValid()) {
		if (reply.value()) {
			m_userConfig = cuserConfig;
			return true;
		}
		return false;
	}
	else {
		qWarning() << tr("(%1) Error while setting user config at: %2").arg(toString(reply.error()),m_dbusPath.path());
	}
	return false;
}
libnutcommon::IPv4UserConfig CInterface::getUserConfig(bool refresh) {
	if (refresh) {
		QDBusReply<libnutcommon::IPv4UserConfig> reply = m_dbusInterface->getUserConfig();
		if (reply.isValid()) {
			m_userConfig = reply.value();
		}
		else {
			qDebug() << tr("Error while getting user config at: %1").arg(m_dbusPath.path());
			m_userConfig = libnutcommon::IPv4UserConfig();
		}
	}
	return m_userConfig;
}

};
