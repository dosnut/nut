#include "cdevice.h"
#include "libnutcommon/common.h"
#include "cdevicemanager.h"
#include "clog.h"
#include "server_proxy.h"
#include "client_exceptions.h"
#include "libnutwireless/wpa_supplicant.h"

namespace libnutclient {
using namespace libnutcommon;

/////////
//CDevice
/////////
CDevice::CDevice(CDeviceManager * parent, QDBusObjectPath m_dbusPath) : 
	CLibNut(parent),
	/*parent(parent),*/
	m_dbusPath(m_dbusPath),
	log(parent->log),
	m_dbusDevice(0),
	m_needWpaSupplicant(false),
	m_pendingRemoval(false),
	m_lockCount(0),
	m_propertiesFetched(false),
	m_environmentsFetched(false),
	m_essidFetched(false),
	m_configFetched(false),
	m_activeEnvFetched(false),
	m_initCompleted(false),
	m_state(libnutcommon::DS_UNCONFIGURED),
	m_type(libnutcommon::DT_ETH),
	m_activeEnvironment(0),
	m_wpaSupplicant(0),
	m_index(-1)
{} //TODO:Init all! pointers to 0


void CDevice::init() {
	//get m_dbusConnection from parent:
	m_dbusConnection = &(qobject_cast<CDeviceManager*>(parent())->m_dbusConnection);
	m_dbusConnectionInterface = qobject_cast<CDeviceManager*>(parent())->m_dbusConnectionInterface;
	//Service check
	serviceCheck(m_dbusConnectionInterface); //TODO:Remove exception garbage
	//connect to dbus-object
	m_dbusDevice = new DBusDeviceInterface(NUT_DBUS_URL, m_dbusPath.path(),*m_dbusConnection, this);
	
	//DBus interface is available: connect:
	connect(m_dbusDevice, SIGNAL(errorOccured(QDBusError)), this, SLOT(dbusret_errorOccured(QDBusError)));

	connect(m_dbusDevice, SIGNAL(gotEnvironments(QList<QDBusObjectPath>)), this, SLOT(dbusretGetEnvironments(QList<QDBusObjectPath>)));
	connect(m_dbusDevice, SIGNAL(gotProperties(libnutcommon::DeviceProperties)), this, SLOT(dbusretGetProperties(libnutcommon::DeviceProperties)));
	connect(m_dbusDevice, SIGNAL(gotEssid(QString)), this, SLOT(dbusretGetEssid(QString)));
	connect(m_dbusDevice, SIGNAL(gotConfig(libnutcommon::DeviceConfig)), this, SLOT(dbusretGetConfig(libnutcommon::DeviceConfig)));
	connect(m_dbusDevice, SIGNAL(gotActiveEnvironment(QString)), this, SLOT(dbusretGetActiveEnvironment(QString)));

	connect(m_dbusDevice, SIGNAL(environmentChangedActive(const QString &)),
			this, SLOT(environmentChangedActive(const QString &)));

	connect(m_dbusDevice, SIGNAL(stateChanged(int , int)),
			this, SLOT(dbusStateChanged(int, int)));


	*log << tr("Placing getProperties call at: %1").arg(m_dbusPath.path());
	m_dbusDevice->getProperties();

	*log << tr("Placing getConfig call at %1").arg(m_dbusPath.path());
	m_dbusDevice->getConfig();

	*log << tr("Placing getEnvironments call at: %1").arg(m_dbusPath.path());
	m_dbusDevice->getEnvironments();
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
	*log << tr("Refreshing device");
	*log << tr("Placing getProperties call at: %1").arg(m_dbusPath.path());
	m_dbusDevice->getProperties();

	*log << tr("Placing getEnvironments call at: %1").arg(m_dbusPath.path());
	m_dbusDevice->getEnvironments();

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


void CDevice::checkInitCompleted() {
	if ( m_propertiesFetched && m_environmentsFetched && m_essidFetched && m_configFetched && m_activeEnvFetched && !m_initCompleted) {
		m_initCompleted = true;
		qDebug() << "Device init completed:" << m_name;
		emit initializationCompleted(this);
	}
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
		m_dbusDevice->getEssid();
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

//CDevice private slots for dbus communication
void CDevice::dbusretGetProperties(libnutcommon::DeviceProperties props) {
	m_name = props.name;
	m_type = props.type;
	m_state = (DeviceState) props.state;
	*log << tr("Device properties fetched");
	*log << tr("Name : %1").arg(m_name);
	*log << tr("Type: %1").arg(libnutclient::toStringTr(m_type));
	*log << tr("State: %1").arg(libnutclient::toStringTr(m_state));


	if (DT_AIR == m_type) {
		m_dbusDevice->getEssid();
	}
	else {
		m_essidFetched = true;
		qDebug() << "essid feteched";
	}

	m_propertiesFetched = true;
	qDebug() << "Properties feteched";
	checkInitCompleted();

	emit gotProperties(props);
	emit newDataAvailable();
}


void CDevice::dbusretGetEssid(QString essid) {
	m_essid = essid;

	m_essidFetched = true;
	qDebug() << "essid feteched";
	checkInitCompleted();
	
	emit gotEssid(essid);
	emit newDataAvailable();
}

void CDevice::dbusretGetEnvironments(QList<QDBusObjectPath> envs) {
	//Compare local with remote list:
	//if they are equal just refresh otherwise rebuild
	bool envequal = (m_dbusEnvironments.size() == envs.size());
	if (envequal) {
		foreach(QDBusObjectPath i, envs) {
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
		rebuild(envs);
	}
	
	//Set active env:
	m_dbusDevice->getActiveEnvironment();

	m_environmentsFetched = true;
	qDebug() << "envs feteched";
	checkInitCompleted();

	emit gotEnvironments();
	emit newDataAvailable();
}

void CDevice::dbusretGetActiveEnvironment(QString activeEnv) {
		if (activeEnv.isEmpty()) { //active env is not set
			m_activeEnvironment = 0;
		}
		else {
			m_activeEnvironment = m_dbusEnvironments.value(QDBusObjectPath(activeEnv), NULL);
		}
		*log << tr("Active Environement: %1").arg(activeEnv);

		m_activeEnvFetched = true;
		qDebug() << "activeenv fetched";
		checkInitCompleted();

		emit gotActiveEnvironment(m_activeEnvironment);
		emit newDataAvailable();
}

void CDevice::dbusretGetConfig(libnutcommon::DeviceConfig config) {

	m_config = config;
	#ifndef LIBNUT_NO_WIRELESS
	m_needWpaSupplicant = !(m_config.wpaConfigFile().isEmpty());

	//Only use wpa_supplicant if we need one
	if (m_needWpaSupplicant) {
		*log << tr("(%2) wpa_supplicant config file at: %1").arg(m_config.wpaConfigFile(),m_name);
	
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
	#endif

	m_configFetched = true;
	qDebug() << "config feteched";
	checkInitCompleted();

	emit gotConfig(m_config);
	emit newDataAvailable();
}


void CDevice::dbusret_errorOccured(QDBusError error, QString method) {
	qDebug() << "Error occured in dbus: " << QDBusError::errorString(error.type()) << "at" << method;
	if (!m_initCompleted) { //error during init
		emit initializationFailed(this);
	}
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

}
