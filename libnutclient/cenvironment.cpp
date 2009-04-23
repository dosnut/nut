#include "cenvironment.h"
#include "libnutcommon/common.h"
#include "server_proxy.h"
#include "clog.h"
#include "client_exceptions.h"
#include "cdevice.h"
#include "cinterface.h"

namespace libnutclient {
using namespace libnutcommon;

//////////////
//CEnvironment
//////////////

CEnvironment::CEnvironment(CDevice * parent, QDBusObjectPath dbusPath) :
	CLibNut(parent),
	/*parent(parent),*/
	m_dbusPath(dbusPath),
	log(parent->log),
	m_state(false),
	m_propertiesFetched(false),
	m_interfacesFetched(false),
	m_configFetched(false),
	m_selectResultFetched(false),
	m_selectResultsFetched(false),
	m_initCompleted(false)
{
	m_dbusConnection = parent->m_dbusConnection;
	m_dbusConnectionInterface = parent->m_dbusConnectionInterface;
} //TODO:Init all vars to default

//Initializes the environment
void CEnvironment::init() {
	if (!serviceCheck()) {
		emit initializationFailed(this);
		emit dbusErrorOccured();
		return;
	}
	m_dbusEnvironment = new DBusEnvironmentInterface(NUT_DBUS_URL, m_dbusPath.path(),*m_dbusConnection,this);

	connect(m_dbusEnvironment, SIGNAL(errorOccured(QDBusError,QString)), this, SLOT(dbusret_errorOccured(QDBusError)));

	connect(m_dbusEnvironment,SIGNAL(gotProperties(libnutcommon::EnvironmentProperties)), this, SLOT(dbusretGetProperties(libnutcommon::EnvironmentProperties)));
	connect(m_dbusEnvironment,SIGNAL(gotConfig(libnutcommon::EnvironmentConfig)),this,SLOT(dbusretGetConfig(libnutcommon::EnvironmentConfig)));
	connect(m_dbusEnvironment,SIGNAL(gotInterfaces(QList< QDBusObjectPath >)),this,SLOT(dbusretGetInterfaces(QList<QDBusObjectPath>)));
	connect(m_dbusEnvironment,SIGNAL(gotSelectResult(libnutcommon::SelectResult)),this,SLOT(dbusretGetSelectResult(libnutcommon::SelectResult)));
	connect(m_dbusEnvironment,SIGNAL(gotSelectResults(QVector< libnutcommon :: SelectResult >)),this,SLOT(dbusretGetSelectResults(QVector<libnutcommon::SelectResult>)));

	connect(m_dbusEnvironment, SIGNAL(stateChanged(bool )), this, SLOT(dbusStateChanged(bool )));


	*log << tr("Placing getProperties call at: %1").arg(m_dbusPath.path());
	m_dbusEnvironment->getProperties();

	*log << tr("Placing getConfig call at: %1").arg(m_dbusPath.path());
	m_dbusEnvironment->getConfig();

	*log << tr("Placing getInterfaces call at: %1").arg(m_dbusPath.path());
	m_dbusEnvironment->getInterfaces();

	*log << tr("Placing getSelectResult call at: %1").arg(m_dbusPath.path());
	m_dbusEnvironment->getSelectResult();

	*log << tr("Placing getSelectResults call at: %1").arg(m_dbusPath.path());
	m_dbusEnvironment->getSelectResults();
}

CEnvironment::~CEnvironment() {

	disconnect(m_dbusEnvironment, SIGNAL(stateChanged(bool )), this, SLOT(dbusStateChanged(bool )));
	disconnect(m_dbusEnvironment, SIGNAL(errorOccured(QDBusError)), this, SLOT(dbusret_errorOccured(QDBusError)));

	disconnect(m_dbusEnvironment,SIGNAL(gotProperties(libnutcommon::EnvironmentProperties)), this, SLOT(dbusretGetProperties(libnutcommon::EnvironmentProperties)));
	disconnect(m_dbusEnvironment,SIGNAL(gotConfig(libnutcommon::EnvironmentConfig)),this,SLOT(dbusretGetConfig(libnutcommon::EnvironmentConfig)));
	disconnect(m_dbusEnvironment,SIGNAL(gotInterfaces(QList< QDBusObjectPath >)),this,SLOT(dbusretGetInterfaces(QList<QDBusObjectPath>)));
	disconnect(m_dbusEnvironment,SIGNAL(gotSelectResult(libnutcommon::SelectResult)),this,SLOT(dbusretGetSelectResult(libnutcommon::SelectResult)));
	disconnect(m_dbusEnvironment,SIGNAL(gotSelectResults(QVector< libnutcommon :: SelectResult >)),this,SLOT(dbusretGetSelectResults(QVector<libnutcommon::SelectResult>)));
	
	CInterface * interface;
	while (!m_interfaces.isEmpty()) {
		interface = m_interfaces.takeFirst();
		emit(interfacesUpdated());
		delete interface;
	}
}

void CEnvironment::checkInitCompleted() {
	if ( m_propertiesFetched && m_interfacesFetched && m_configFetched && m_selectResultFetched && 	m_selectResultsFetched && !m_initCompleted) {
		m_initCompleted = true;
		qDebug() << "EnvironmentInit completed:" << m_name;
		emit initializationCompleted(this);
	}
}


void CEnvironment::dbusretGetInterfaces(QList<QDBusObjectPath> interfaces) {
	//Check if we need to rebuild the interface list or just refresh them:
	bool ifequal = (interfaces.size() == m_dbusInterfaces.size());
	if (ifequal) {
		foreach(QDBusObjectPath i, interfaces) {
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
			rebuild(interfaces);
			return;
		}
	}
	else {
		rebuild(interfaces);
		return;
	}
}

void CEnvironment::dbusretGetProperties(libnutcommon::EnvironmentProperties properties) {
	if (qobject_cast<CDevice*>(parent())->m_dbusEnvironments.size() == 0) { //standard environment
		m_name = tr("default");
	}
	else {
		m_name = properties.name;
		//environment untitled
		if (m_name.length() == 0)
			m_name = tr("untitled (%1)").arg(qobject_cast<CDevice*>(parent())->m_dbusEnvironments.size());
	}
	qDebug() << QString("Environmentname: %1").arg(m_name);
	m_state = properties.active;

	m_propertiesFetched = true;
	checkInitCompleted();

	emit newDataAvailable();
}

void CEnvironment::dbusretGetConfig(libnutcommon::EnvironmentConfig config) {
	m_config = config;
	
	m_configFetched = true;
	checkInitCompleted();
	
	emit newDataAvailable();
}

void CEnvironment::dbusretGetSelectResult(libnutcommon::SelectResult selectResult) {
	m_selectResult = selectResult;
	
	m_selectResultFetched = true;
	checkInitCompleted();
	
	emit newDataAvailable();
}

void CEnvironment::dbusretGetSelectResults(QVector<libnutcommon::SelectResult> selectResults) {
	m_selectResults = selectResults;

	m_selectResultsFetched = true;
	checkInitCompleted();
	
	emit newDataAvailable();
}

void CEnvironment::dbusret_errorOccured(QDBusError error, QString method) {
	qDebug() << "Error occured in dbus: " << QDBusError::errorString(error.type()) << "at" << method;
	if (!m_initCompleted) { //error during init
		emit initializationFailed(this);
	}
	if (!serviceCheck()) {
		emit dbusErrorOccured();
	}
}


void CEnvironment::interfaceInitializationFailed(CInterface * interface) {
	if (!m_initCompleted) { //failure in init phase
		emit initializationFailed(this);
	}
}

void CEnvironment::interfaceInitializationCompleted(CInterface * interface) {
	m_interfaces.append(interface);
	interface->m_index = m_interfaces.indexOf(interface);

	if (m_interfaces.size() == m_dbusInterfaces.size()) { //check if all interfaces are ready
		qDebug() << "Feteched all Interfaces";
		m_interfacesFetched = true;
		checkInitCompleted();
	}
// 	else {
// 		qDebug() << "Interface count" << m_interfaces.size() << m_dbusInterfaces.size(); 
// 	}
	emit newDataAvailable();
}


//CEnvironment private functions

void CEnvironment::refreshAll() {

	*log << tr("Placing getProperties call at: %1").arg(m_dbusPath.path());
	m_dbusEnvironment->getProperties();

	*log << tr("Placing getConfig call at: %1").arg(m_dbusPath.path());
	m_dbusEnvironment->getConfig();

	*log << tr("Placing getInterfaces call at: %1").arg(m_dbusPath.path());
	m_dbusEnvironment->getInterfaces();

	*log << tr("Placing getSelectResult call at: %1").arg(m_dbusPath.path());
	m_dbusEnvironment->getSelectResult();

	*log << tr("Placing getSelectResults call at: %1").arg(m_dbusPath.path());
	m_dbusEnvironment->getSelectResults();
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
		interface = new CInterface(this,i);
		m_dbusInterfaces.insert(i,interface);
		connect(interface,SIGNAL(initializationFailed(CInterface*)),this,SLOT(interfaceInitializationFailed(CInterface*)));
		connect(interface,SIGNAL(initializationCompleted(CInterface*)),this,SLOT(interfaceInitializationCompleted(CInterface*)));
		interface->init();
	}
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
	qobject_cast<CDevice *>(parent())->setEnvironment(this);
}
libnutcommon::EnvironmentConfig& CEnvironment::getConfig() {
	return m_config;
}

libnutcommon::SelectResult& CEnvironment::getSelectResult(bool refresh) {
	if (refresh) {
		m_dbusEnvironment->getSelectResult();
	}
	return m_selectResult;
}

QVector<libnutcommon::SelectResult>& CEnvironment::getSelectResults(bool refresh) {
	if (refresh) {
		m_dbusEnvironment->getSelectResults();
	}
	return m_selectResults;
}

}
