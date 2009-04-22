#include "cdevicemanager.h"
#include "libnutcommon/common.h"
#include "server_proxy.h"
#include "cdevice.h"
#include "clog.h"
#include "client_exceptions.h"

namespace libnutclient {
using namespace libnutcommon;

////////////////
//CDeviceManager
///////////////
CDeviceManager::CDeviceManager(QObject * parent) :
	CLibNut(parent),
	m_dbusDevmgr(0),
	m_dbusConnection(QDBusConnection::connectToBus(QDBusConnection::SystemBus, QString::fromLatin1("libnut_system_bus"))),
	log(0),
	m_dbusTimerId(-1), m_dbusPath("/manager"),
	m_dbusMonitor(this),
	m_dbusInterface(0) 
{
	
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
	m_dbusDevmgr = new DBusDeviceManagerInterface(NUT_DBUS_URL, m_dbusPath,m_dbusConnection, this);

	connect(m_dbusConnectionInterface, SIGNAL(serviceOwnerChanged(const QString &, const QString &, const QString &)), this, SLOT(dbusServiceOwnerChanged(const QString &, const QString &, const QString &)));
	//Connect dbus-signals to own slots:
	connect(m_dbusDevmgr, SIGNAL(deviceAdded(const QDBusObjectPath&)), this, SLOT(dbusDeviceAdded(const QDBusObjectPath&)));
	connect(m_dbusDevmgr, SIGNAL(deviceRemoved(const QDBusObjectPath&)), this, SLOT(dbusDeviceRemoved(const QDBusObjectPath&)));
	connect(m_dbusDevmgr, SIGNAL(gotDeviceList(QList<QDBusObjectPath>)), this, SLOT(dbusretGetDeviceList(QList<QDBusObjectPath>)));
	connect(m_dbusDevmgr, SIGNAL(errorOccured(QDBusError)), this, SLOT(dbusretErrorOccured(QDBusError)));


	if (m_nutsstate) {
		setInformation();
		emit(stateChanged(true));
	}
	return m_nutsstate;
}

void CDeviceManager::deviceInitializationFailed(CDevice * device) {
	m_dbusDevices.take(device->m_dbusPath);
	if (m_devices.removeAll(device) > 0 ) { //Check if device got into devlist (should not happen)
	//Rebuild device->index for qnut
		foreach(CDevice * dev, m_devices) {
			dev->m_index = m_devices.indexOf(dev);
		}
	}
// 	emit(deviceRemoved(device));
	device->deleteLater();
}

void CDeviceManager::deviceInitializationCompleted(CDevice * device) {
	m_devices.append(device);
	device->m_index = m_devices.indexOf(device); // Set device index;
	emit(deviceAdded(device));
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
//rebuilds the device list and populates it on start-up
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
		device = new CDevice(this, i);
		m_dbusDevices.insert(i, device);
		connect(device,SIGNAL(initializationFailed(CDevice*)), this, SLOT(deviceInitializationFailed(CDevice*)));
		connect(device,SIGNAL(initializationCompleted(CDevice*)),this,SLOT(deviceInitializationCompleted(CDevice*)));
		device->init();
	}
}

void CDeviceManager::setInformation() {
	//get devicelist etc.
	qDebug() << "setInformation()";
// 	QDBusReply<QList<QDBusObjectPath> > replydevs;
// 	replydevs = m_dbusDevmgr->getDeviceList();
// 	if (!replydevs.isValid()) {
// 		//This is the first time we're trying to connect to nuts.
// 		//If we're not allowed to do that, we should break here and print out a warning
// 		if (QDBusError::AccessDenied == replydevs.error().type()) {
// 			*log << tr("You are not allowed to connect to nuts.");
// 			*log << tr("Please make sure you are in the correct group");
// 			return;
// 		}
// 		else if (QDBusError::InvalidSignature == replydevs.error().type()) { //Workaround qt returning wrong Error (Should be AccessDenied)
// 			*log << tr("(%1) Failed to get DeviceList").arg(toString(replydevs.error()));
// 			*log << tr("Maybe you don't have sufficient rights");
// 		}
// 		else {
			if ( !dbusConnected(&m_dbusConnection) ) {
				dbusKilled();
				return;
			}
// 			qWarning() << tr("(%1) Failed to get DeviceList").arg(toString(replydevs.error()));
// 		}
// 	}
	qDebug() << "Placing getDeviceList Call";
	m_dbusDevmgr->getDeviceList();
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
			connect(m_dbusDevmgr, SIGNAL(gotDeviceList(QList<QDBusObjectPath>)), this, SLOT(dbusretGetDeviceList(QList<QDBusObjectPath>)));
			connect(m_dbusDevmgr, SIGNAL(errorOccured(QDBusError)), this, SLOT(dbusretErrorOccured(QDBusError)));
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
			disconnect(m_dbusDevmgr, SIGNAL(gotDeviceList(QList<QDBusObjectPath>)), this, SLOT(dbusretGetDeviceList(QList<QDBusObjectPath>)));
			disconnect(m_dbusDevmgr, SIGNAL(errorOccured(QDBusError)), this, SLOT(dbusretErrorOccured(QDBusError)));
			//start the dbus was killed timer TODO:replace with inotify
			if (-1 == m_dbusTimerId) {
				m_dbusTimerId = startTimer(5000);
			}
 		}
	}
}
//DBUS CALL AND ERROR FUNCTIONS
void CDeviceManager::dbusretGetDeviceList(QList<QDBusObjectPath> devices) {
		//If a device is missing or there are too many m_devices in our Hash
		//Then rebuild complete tree, otherwise just call refresh on child
		bool equal = true;
		if ( (m_dbusDevices.size() == devices.size()) ) {
			foreach(QDBusObjectPath i, devices) {
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
				rebuild(devices);
				return;
			}
		}
		else {
			rebuild(devices);
			return;
		}
}

void CDeviceManager::dbusretErrorOccured(QDBusError error) {
	qDebug() << "Error occured in dbus: " << QDBusError::errorString(error.type());
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

bool CDeviceManager::createBridge(QList<CDevice *> devices) {
	return false;
// 	//check if devices are non-bridges and assemble dbus path list
// 	QList<QDBusObjectPath> devpaths;
// 	foreach(CDevice * device, devices) {
// 		if (device->m_type & DT_BRIDGE)
// 			return false;
// 		devpaths.append(device->m_dbusPath);
// 	}
// 	QDBusReply<bool> reply = m_dbusDevmgr->createBridge(devpaths);
// 	if (reply.isValid()) {
// 		return reply.value();
// 	}
// 	else {
// 		if ( !dbusConnected(&m_dbusConnection) ) {
// 			dbusKilled();
// 		}
// 		qWarning() << tr("(%1) Could not create bridge").arg(toString(reply.error()));
// 		return false;
// 	}
}

bool CDeviceManager::destroyBridge(CDevice * device) {
	return false;
// 	//Check if device is a bridge:
// 	if (DT_BRIDGE != device->m_type) {
// 		return false;
// 	}
// 	else {
// 		QDBusReply<bool> reply = m_dbusDevmgr->destroyBridge(device->m_dbusPath);
// 		if (reply.isValid()) {
// 			return reply.value();
// 		}
// 		else {
// 			if ( !dbusConnected(&m_dbusConnection) ) {
// 				dbusKilled();
// 			}
// 			qWarning() << tr("(%1) Could not create bridge").arg(toString(reply.error()));
// 			return false;
// 		}
// 	}
}


//CDeviceManager SLOTS
void CDeviceManager::refreshAll() {
	//Only refresh when nuts is available
	if (!m_nutsstate) {
		return;
	}
	//Get our current DeviceList
	m_dbusDevmgr->getDeviceList();
}

// void CDeviceManager::rebuild() {
// 	//Do not rebuild if nuts is not running
// 	if (!m_nutsstate) {
// 		return;
// 	}
// 	QDBusReply<QList<QDBusObjectPath> > replydevs = m_dbusDevmgr->getDeviceList();
// 	if (replydevs.isValid()) {
// 		rebuild(replydevs.value());
// 	}
// 	else {
// 		if ( !dbusConnected(&m_dbusConnection) ) {
// 			dbusKilled();
// 			return;
// 		}
// 		qWarning() << tr("(%1) Error while retrieving device list").arg(toString(replydevs.error()));
// 	}
// }

}
