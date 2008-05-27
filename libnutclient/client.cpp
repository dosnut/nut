/*
	TRANSLATOR libnutclient::CLibNut
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
		case DS_UP:             return CLibNut::tr("up");
		case DS_UNCONFIGURED:   return CLibNut::tr("unconfigured");
		case DS_CARRIER:        return CLibNut::tr("got carrier");
		case DS_ACTIVATED:      return CLibNut::tr("activated");
		case DS_DEACTIVATED:    return CLibNut::tr("deactivated");
		default:                return QString();
	}
}
QString toStringTr(DeviceType type) {
	switch (type) {
		case DT_ETH: return CLibNut::tr("Ethernet");
		case DT_AIR: return CLibNut::tr("Wireless");
		case DT_PPP: return CLibNut::tr("PPP");
		default:     return QString();
	}
}
QString toStringTr(InterfaceState state) {
	switch (state) {
		case IFS_OFF: return CLibNut::tr("off");
		case IFS_STATIC: return CLibNut::tr("static");
		case IFS_DHCP: return CLibNut::tr("dynamic");
		case IFS_ZEROCONF: return CLibNut::tr("zeroconf");
		case IFS_WAITFORCONFIG: return CLibNut::tr("wait for config");
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
CDeviceManager::CDeviceManager(QObject * parent) : CLibNut(parent), m_dbusDevmgr(0), m_dbusConnection(QDBusConnection::connectToBus(QDBusConnection::SystemBus, QString::fromLatin1("libnut_system_bus"))), m_dbusTimerId(-1), m_dbusMonitor(this) {
	
	//Init dbus monitor
	connect(&m_dbusMonitor,SIGNAL(stopped(void)),this,SLOT(dbusStopped(void)));
	connect(&m_dbusMonitor,SIGNAL(started(void)),this,SLOT(dbusStarted(void)));
	m_dbusMonitor.setPidFileDir(DBUS_PID_FILE_DIR);
	m_dbusMonitor.setPidFileName(DBUS_PID_FILE_NAME);
	qDebug() << "Enabling DBusMonitor";
	m_dbusMonitor.setEnabled(true);
	qDebug() << "Enabled DBusMonitor";

	//Init Hashtable
	m_dbusDevices.reserve(10);
}
CDeviceManager::~CDeviceManager() {
//Cleanup action: delete entire devicelist
	CDevice * device;
	while (!m_devices.isEmpty()) {
		device = m_devices.takeFirst();
		delete device;
	}
}

bool CDeviceManager::init(CLog * inlog) {
	log = inlog;
	//Check if dbus is available
	if ( !m_dbusConnection.isConnected() ) {
		m_nutsstate = false;
		//another timer is running?
		if (m_dbusTimerId != -1) {
			killTimer(m_dbusTimerId);
			m_dbusTimerId = -1;
		}
		*log << tr("Error while trying to access the dbus service");
		*log << tr("Please make sure that dbus is running");
// 		m_dbusTimerId = startTimer(10000);
		return false;
	}

	m_nutsstate = true;
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

void CDeviceManager::timerEvent(QTimerEvent *event) {
	if (event->timerId() == m_dbusTimerId ) {
		killTimer(m_dbusTimerId);
		m_dbusTimerId = -1;
		//Check if already connected to dbus:
		if ( !m_dbusConnection.isConnected() ) {
			m_dbusConnection.disconnectFromBus(QString::fromLatin1("libnut_system_bus"));
			//Try to connect to dbus, we have to do it like that, as QDBusConnection::systemBus does not close the connection correctly
			m_dbusConnection = QDBusConnection(QDBusConnection::connectToBus(QDBusConnection::SystemBus, QString::fromLatin1("libnut_system_bus")));
			*log << "Connecting to dbus";
			//init connection
			init(log);
		}
	}
}

//this function will just clear everything and start the dbus polling timer
void CDeviceManager::dbusKilled(bool doinit) {
	//Kill all dbus polling timers:
	if (-1 != m_dbusTimerId) {
		killTimer(m_dbusTimerId);
	}
	//Clean Device list:
	m_dbusDevices.clear();
	CDevice * dev;
	while (!m_devices.isEmpty()) {
		dev = m_devices.takeFirst();
		emit(deviceRemoved(dev));
		dev->deleteLater();
	}
	//Clean up everything else:
	if (m_dbusDevmgr) {
		delete m_dbusDevmgr;
		m_dbusDevmgr = NULL;
	}
	if (doinit) { //kill came from error on accessing dbus
		//init will start polling
		init(log);
	}
	else { //kill came from inotify
		m_dbusConnection.disconnectFromBus(QString::fromLatin1("libnut_system_bus"));
	}
}

//CDeviceManager private functions:
//rebuilds the device list
void CDeviceManager::rebuild(QList<QDBusObjectPath> paths) {
	//Only rebuild when nuts is available
	if (!m_nutsstate) {
		return;
	}
	//Delete all m_devices
	m_dbusDevices.clear();
	CDevice * device;
	while (!m_devices.isEmpty()) {
		device = m_devices.takeFirst();
		emit(deviceRemoved(device));
		delete device;
	}
	//Build new m_devices
	foreach(QDBusObjectPath i, paths) {
		try {
			device = new CDevice(this, i);
		}
		catch (CLI_ConnectionException &e) {
			if ( !dbusConnected(&m_dbusConnection) ) {
				dbusKilled();
				return;
			}
			qWarning() << e.what();
			continue;
		}
		m_dbusDevices.insert(i, device);
		m_devices.append(device);
		device->m_index = m_devices.indexOf(device); // Set device index;
		emit(deviceAdded(device));
	}
}

void CDeviceManager::setInformation() {
	//get devicelist etc.
	qDebug() << "setInformation()";
	QDBusReply<QList<QDBusObjectPath> > replydevs;
	replydevs = m_dbusDevmgr->getDeviceList();
	if (!replydevs.isValid()) {
		//This is the first time we're trying to connect to nuts.
		//If we're not allowed to do that, we should break here and print out a warning
		if (QDBusError::AccessDenied == replydevs.error().type()) {
			*log << tr("You are not allowed to connect to nuts.");
			*log << tr("Please make sure you are in the correct group");
			return;
		}
		else if (QDBusError::InvalidSignature == replydevs.error().type()) { //Workaround qt returning wrong Error (Should be AccessDenied)
			*log << tr("(%1) Failed to get DeviceList").arg(toString(replydevs.error()));
			*log << tr("Maybe you don't have sufficient rights");
		}
		else {
			if ( !dbusConnected(&m_dbusConnection) ) {
				dbusKilled();
				return;
			}
			qWarning() << tr("(%1) Failed to get DeviceList").arg(toString(replydevs.error()));
		}
	}
	qDebug() << "Populating device list";
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
			if ( !dbusConnected(&m_dbusConnection) ) {
				dbusKilled();
				return;
			}
			qWarning() << e.msg();
			continue;
		}
		m_devices.append(device);
		device->m_index = m_devices.indexOf(device); // Set device index;
		m_dbusDevices.insert(i, device);
		emit(deviceAdded(device));
	}
}
void CDeviceManager::clearInformation() {
	//Clean Device list:
	m_dbusDevices.clear();
	CDevice * dev;
	while (!m_devices.isEmpty()) {
		dev = m_devices.takeFirst();
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
			//start the dbus was killed timer TODO:replace with inotify
			if (-1 == m_dbusTimerId) {
				m_dbusTimerId = startTimer(5000);
			}
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
			if ( !dbusConnected(&m_dbusConnection) ) {
				dbusKilled();
				return;
			}
			qWarning() << e.msg();
			return;
		}
		m_dbusDevices.insert(objectpath,device);
		m_devices.append(device);
		device->m_index = m_devices.indexOf(device); // Set device index;
		emit(deviceAdded(device));
	}
	else {
		m_dbusDevices.value(objectpath)->refreshAll();
	}
}
void CDeviceManager::dbusDeviceRemoved(const QDBusObjectPath &objectpath) {
	//remove m_devices from devicelist
	if (m_dbusDevices.value(objectpath)->m_lockCount == 0) {
		CDevice * device = m_dbusDevices.take(objectpath);
		m_devices.removeAll(device);
		//Rebuild device->index for qnut
		foreach(CDevice * dev, m_devices) {
			dev->m_index = m_devices.indexOf(dev);
		}
		emit(deviceRemoved(device));
		device->deleteLater();
	}
	else {
		m_dbusDevices.value(objectpath)->m_pendingRemoval = true;
	}
}

void CDeviceManager::dbusStopped() {
	*log << tr("The dbus daemon has been stopped. Please restart dbus and nuts");
	dbusKilled(false);
}
void CDeviceManager::dbusStarted() {
	*log << tr("dbus has been started initiating dbus interface");
	//delete all information in case this has not happended yet, but do not init
	dbusKilled(false);
	//open connection
	m_dbusConnection = QDBusConnection(QDBusConnection::connectToBus(QDBusConnection::SystemBus, QString::fromLatin1("libnut_system_bus")));
	//init
	init(log);
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
		//If a device is missing or there are too many m_devices in our Hash
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
				foreach(CDevice * i, m_devices) {
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
		if ( !dbusConnected(&m_dbusConnection) ) {
			dbusKilled();
			return;
		}
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
		if ( !dbusConnected(&m_dbusConnection) ) {
			dbusKilled();
			return;
		}
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
		m_name = replyProp.value().name;
		m_type = replyProp.value().type;
		m_state = (DeviceState) replyProp.value().state;
		m_activeEnvironment = 0;
		*log << tr("Device properties fetched");
		*log << tr("Name : %1").arg(m_name);
		*log << tr("Type: %1").arg(libnutclient::toStringTr(m_type));
		*log << tr("State: %1").arg(libnutclient::toStringTr(m_state));
	}
	else {
		throw CLI_DevConnectionException(tr("(%1) Error while retrieving dbus' device information").arg(toString(replyProp.error())));
	}

	if (DT_AIR == m_type) {
		QDBusReply<QString> replyessid = m_dbusDevice->getEssid();
		if (replyessid.isValid()) {
			m_essid = replyessid.value();
		}
		else {
			qWarning() << tr("(%1) Could not refresh device essid").arg(toString(replyessid.error()));
			m_essid = QString();
		}
	}

	//get config and set wpa_supplicant variable
	QDBusReply<libnutcommon::DeviceConfig> replyconf = m_dbusDevice->getConfig();
	if (replyconf.isValid()) {
		m_config = replyconf.value();
		#ifndef LIBNUT_NO_WIRELESS
		m_needWpaSupplicant = !(m_config.wpaConfigFile().isEmpty());
		#endif
		*log << tr("(%2) wpa_supplicant config file at: %1").arg(m_config.wpaConfigFile(),m_name);
	}
	else {
		throw CLI_DevConnectionException(tr("(%2) Error(%1) while retrieving device config").arg(replyconf.error().name(),m_name));
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
			if ( !dbusConnected(m_dbusConnection) ) {
				throw CLI_DevConnectionException(tr("(%1) Error while trying to add environment").arg(toString(replyEnv.error())));
			}
			qWarning() << e.msg();
			continue;
		}
		m_dbusEnvironments.insert(i,env);
		m_environments.append(env);
		env->m_index = m_environments.indexOf(env);
	}
	
	if (!replyProp.value().activeEnvironment.isEmpty()) {
		m_dbusActiveEnvironment = QDBusObjectPath(replyProp.value().activeEnvironment);
		m_activeEnvironment = m_dbusEnvironments.value(m_dbusActiveEnvironment, 0);
		*log << tr("Active Environement: %1").arg(m_dbusActiveEnvironment.path());
	}
	else {
		m_activeEnvironment = 0;
	}
	//connect signals to slots
	connect(m_dbusDevice, SIGNAL(environmentChangedActive(const QString &)),
			this, SLOT(environmentChangedActive(const QString &)));

	connect(m_dbusDevice, SIGNAL(stateChanged(int , int)),
			this, SLOT(dbusStateChanged(int, int)));

	#ifndef LIBNUT_NO_WIRELESS
	//Only use wpa_supplicant if we need one
	if (m_needWpaSupplicant) {
		m_wpaSupplicant = new libnutwireless::CWpaSupplicant(this,m_name);
		connect(m_wpaSupplicant,SIGNAL(message(QString)),log,SLOT(log(QString)));
		connect(m_dbusDevice,SIGNAL(newWirelssNetworkFound(void)),this,SIGNAL(newWirelessNetworkFound(void)));
		//Connect to wpa_supplicant only if device is not deactivated
		if (! (DS_DEACTIVATED == m_state) ) {
			m_wpaSupplicant->open();
		}
		if (DT_AIR == m_type && DS_CARRIER <= m_state && m_wpaSupplicant != NULL) {
			m_wpaSupplicant->setSignalQualityPollRate(500);
		}
	}
	else {
		m_wpaSupplicant = NULL;
	}
	#endif
}
CDevice::~CDevice() {
	//CWpa_supplicant will be killed as child of CDevices
	CEnvironment * env;
	while (!m_environments.isEmpty()) {
		env = m_environments.takeFirst();
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
			foreach(CEnvironment * i, m_environments) {
				i->refreshAll();
			}
		}
		else {
			rebuild(replyenvs.value());
		}
	}
	else {
		if ( !dbusConnected(m_dbusConnection) ) {
			static_cast<CDeviceManager*>(parent())->dbusKilled();
			return;
		}
		qWarning() << tr("(%1) Could not refresh environments").arg(toString(replyenvs.error()));
	}
	m_activeEnvironment = 0;
	//now refresh the rest of our device properties:
	QDBusReply<DeviceProperties> replyprop = m_dbusDevice->getProperties();
	if (replyprop.isValid()) {
		if (!replyprop.value().activeEnvironment.isEmpty()) {
			m_dbusActiveEnvironment = QDBusObjectPath(replyprop.value().activeEnvironment);
			m_activeEnvironment = m_dbusEnvironments.value(m_dbusActiveEnvironment);
		}
		*log << tr("Refreshing active environment: %1").arg(m_dbusActiveEnvironment.path());
		m_state = (DeviceState) replyprop.value().state;
		m_type = replyprop.value().type;
		m_name = replyprop.value().name;
	}
	else {
		if ( !dbusConnected(m_dbusConnection) ) {
			static_cast<CDeviceManager*>(parent())->dbusKilled();
			return;
		}
		qWarning() << tr("(%1) Could not refresh device properties").arg(toString(replyprop.error()));
	}
	if (DT_AIR == m_type) {
		QDBusReply<QString> replyessid = m_dbusDevice->getEssid();
		if (replyessid.isValid()) {
			m_essid = replyessid.value();
		}
		else {
			if ( !dbusConnected(m_dbusConnection) ) {
				static_cast<CDeviceManager*>(parent())->dbusKilled();
				return;
			}
			qWarning() << tr("(%1) Could not refresh device essid").arg(toString(replyessid.error()));
			m_essid = QString();
		}
	}
	#ifndef LIBNUT_NO_WIRELESS
	if ( !(DS_DEACTIVATED == m_state) ) {
		if (m_needWpaSupplicant) {
			m_wpaSupplicant->open();
		}
	}
	if (DT_AIR == m_type && DS_CARRIER <= m_state && m_wpaSupplicant != NULL) {
		m_wpaSupplicant->setSignalQualityPollRate(500);
	}
	#endif
}
//Rebuilds the environment list
void CDevice::rebuild(QList<QDBusObjectPath> paths) {
	//Remove current list:
	m_dbusEnvironments.clear();
	CEnvironment * env;
	while ( !m_environments.isEmpty() ) {
		env = m_environments.takeFirst();
// 		emit(environmentRemoved(env)); //Pending for removal
		delete env;
	}
	//now rebuild:
	foreach(QDBusObjectPath i, paths) {
		try {
			env = new CEnvironment(this,i);
		}
		catch (CLI_ConnectionException &e) {
			if ( !dbusConnected(m_dbusConnection) ) {
				static_cast<CDeviceManager*>(parent())->dbusKilled();
				return;
			}
			qWarning() << e.what();
			continue;
		}
		m_dbusEnvironments.insert(i,env);
		m_environments.append(env);
		env->m_index = m_environments.indexOf(env);
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
	CEnvironment * oldenv = m_activeEnvironment;
	if (newenv.isEmpty()) { //No active Environment is set.
		m_activeEnvironment = NULL;
	}
	else {
		m_activeEnvironment = m_dbusEnvironments.value(QDBusObjectPath(newenv), 0);
	}
	emit(environmentChangedActive(m_activeEnvironment, oldenv));
}
//Every time our device changed from anything to active, our active environment may have changed
void CDevice::dbusStateChanged(int newState, int oldState) {
	#ifndef LIBNUT_NO_WIRELESS
	//If switching from DS_DEACTIVATED to any other state then connect wpa_supplicant
	if (DS_DEACTIVATED == oldState && !(DS_DEACTIVATED == newState) ) {
		if (m_needWpaSupplicant) {
			m_wpaSupplicant->open();
		}
	}
	else if (DS_DEACTIVATED == newState) {
		if (m_needWpaSupplicant) {
			m_wpaSupplicant->close();
		}
	}
	#endif
	m_state = (DeviceState) newState;

	//get possible new essid
	if (DT_AIR == m_type && !(newState == DS_DEACTIVATED) ) {
		QDBusReply<QString> replyessid = m_dbusDevice->getEssid();
		if (replyessid.isValid()) {
			m_essid = replyessid.value();
		}
		else {
			if ( !dbusConnected(m_dbusConnection) ) {
				static_cast<CDeviceManager*>(parent())->dbusKilled();
				return;
			}
			m_essid = QString();
		}
	}
	else {
		m_essid = QString();
	}
	#ifndef LIBNUT_NO_WIRELESS
	if (DT_AIR == m_type && DS_CARRIER <= m_state && m_wpaSupplicant != NULL) {
		m_wpaSupplicant->setSignalQualityPollRate(500);
	}
	#endif
	emit(stateChanged(m_state));
}


//CDevice SLOTS
void CDevice::enable() {
	m_dbusDevice->enable();
}
void CDevice::disable() {
	m_dbusDevice->disable();
}
void CDevice::setEnvironment(CEnvironment * environment) {
	if (DS_DEACTIVATED == m_state) {
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
	m_state = false;
	//First attach to dbus
	m_dbusConnection = parent->m_dbusConnection;
	m_dbusConnectionInterface = parent->m_dbusConnectionInterface;
	serviceCheck(m_dbusConnectionInterface);
	m_dbusEnvironment = new DBusEnvironmentInterface(NUT_DBUS_URL, m_dbusPath.path(),*m_dbusConnection,this);

	//get Environment properties
	QDBusReply<EnvironmentProperties> replyprop = m_dbusEnvironment->getProperties();
	if (replyprop.isValid()) {
		if (parent->m_environments.size() == 0)
			//standard environment
			m_name = tr("default");
		else {
			m_name = replyprop.value().name;
			//environment untitled
			if (m_name.length() == 0)
				m_name = tr("untitled (%1)").arg(parent->m_environments.size());
		}
		qDebug() << QString("Environmentname: %1").arg(m_name);
		m_state = replyprop.value().active;
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
				if ( !dbusConnected(m_dbusConnection) ) {
					throw CLI_EnvConnectionException(tr("(%1) Error while adding interfaces").arg(replyconf.error().name()));
				}
				qWarning() << e.what();
				continue;
			}
			m_dbusInterfaces.insert(i,interface);
			m_interfaces.append(interface);
			interface->m_index = m_interfaces.indexOf(interface);
		}
	}
	else {
		throw CLI_EnvConnectionException(tr("Error while retrieving environment's interfaces"));
	}
	connect(m_dbusEnvironment, SIGNAL(stateChanged(bool )), this, SLOT(dbusStateChanged(bool )));
}
CEnvironment::~CEnvironment() {
	CInterface * interface;
	while (!m_interfaces.isEmpty()) {
		interface = m_interfaces.takeFirst();
		emit(interfacesUpdated());
		delete interface;
	}
}

//CEnvironment private functions

void CEnvironment::refreshAll() {
	//Retrieve properties and select config, then interfaces:
	QDBusReply<EnvironmentProperties> replyprop = m_dbusEnvironment->getProperties();
	if (replyprop.isValid()) {
		if (static_cast<CDevice *>(parent())->m_environments[0] == this)
			//standard environment
			m_name = tr("default");
		else {
			m_name = replyprop.value().name;
			//environment untitled
			if (m_name.length() == 0)
				m_name = tr("untitled (%1)").arg(static_cast<CDevice *>(parent())->m_environments.size());
		}
	}
	else {
		if ( !dbusConnected(m_dbusConnection) ) {
			static_cast<CDeviceManager*>(parent()->parent())->dbusKilled();
			return;
		}
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
				foreach (CInterface * i, m_interfaces) {
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
		if ( !dbusConnected(m_dbusConnection) ) {
			static_cast<CDeviceManager*>(parent()->parent())->dbusKilled();
			return;
		}
		qWarning() << tr("Error while refreshing environment interfaces");
	}
}
void CEnvironment::rebuild(const QList<QDBusObjectPath> &paths) {
	//Remove all interfaces
	m_dbusInterfaces.clear();
	CInterface * interface;
	while (!m_interfaces.isEmpty()) {
		interface = m_interfaces.takeFirst();
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
		m_interfaces.append(interface);
		interface->m_index = m_interfaces.indexOf(interface);
	}
	getSelectResult(true); //functions saves SelectResult
	getSelectResults(true); //functions saves SelectResults
}

//CEnvironment private slots:
void CEnvironment::dbusStateChanged(bool state) {
	m_state = state;
	emit(activeChanged(state));
}

void CEnvironment::dbusSelectResultChanged(libnutcommon::SelectResult result, QVector<libnutcommon::SelectResult> results) {
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
			if ( !dbusConnected(m_dbusConnection) ) {
				static_cast<CDeviceManager*>(parent()->parent())->dbusKilled();
				return m_selectResult;
			}
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
			if ( !dbusConnected(m_dbusConnection) ) {
				static_cast<CDeviceManager*>(parent()->parent())->dbusKilled();
				return m_selectResults;
			}
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
		m_state = replyprops.value().ifState;
		m_ip = replyprops.value().ip;
		m_netmask = replyprops.value().netmask;
		m_gateway = replyprops.value().gateway;
		m_dnsservers = replyprops.value().dns;
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

	connect(m_dbusInterface, SIGNAL(stateChanged(libnutcommon::InterfaceProperties)), this, SLOT(dbusStateChanged(libnutcommon::InterfaceProperties)));
}
CInterface::~CInterface() {
}
//CInterface private functions:
void CInterface::refreshAll() {
	QDBusReply<InterfaceProperties> replyprops = m_dbusInterface->getProperties();
	if (replyprops.isValid()) {
		m_state = replyprops.value().ifState;
		m_ip = replyprops.value().ip;
		m_netmask = replyprops.value().netmask;
		m_gateway = replyprops.value().gateway;
		m_dnsservers = replyprops.value().dns;
		getUserConfig(true); //Function will updated userConfig
	}
	else {
		if ( !dbusConnected(m_dbusConnection) ) {
			static_cast<CDeviceManager*>(parent()->parent()->parent())->dbusKilled();
			return;
		}
		qWarning() << (tr("Error while refreshing interface at: %1").arg(m_dbusPath.path()));
	}
}
//CInterface private slots
void CInterface::dbusStateChanged(libnutcommon::InterfaceProperties properties) {
	//Check changes:
	m_state = properties.ifState;
	m_ip = properties.ip;
	m_netmask = properties.netmask;
	m_gateway = properties.gateway;
	m_dnsservers = properties.dns;
	getUserConfig(true); //Function will updated userConfig
	*log << tr("Interface state of %1 has changed to %2").arg(m_dbusPath.path(),libnutclient::toStringTr(m_state));
	emit(stateChanged(m_state));
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
		if ( !dbusConnected(m_dbusConnection) ) {
			static_cast<CDeviceManager*>(parent()->parent()->parent())->dbusKilled();
			return false;
		}
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
		if ( !dbusConnected(m_dbusConnection) ) {
			static_cast<CDeviceManager*>(parent()->parent()->parent())->dbusKilled();
			return false;
		}
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
			if ( !dbusConnected(m_dbusConnection) ) {
				static_cast<CDeviceManager*>(parent()->parent()->parent())->dbusKilled();
				return m_userConfig;
			}
			qDebug() << tr("Error while getting user config at: %1").arg(m_dbusPath.path());
			m_userConfig = libnutcommon::IPv4UserConfig();
		}
	}
	return m_userConfig;
}

}
