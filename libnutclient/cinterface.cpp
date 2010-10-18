#include "cinterface.h"
#include "libnutcommon/common.h"
#include "cenvironment.h"
#include "server_proxy.h"
#include "clog.h"

// #include <QDBusObjectPath>
// #include <QDBusError>

namespace libnutclient {
using namespace libnutcommon;

////////////
//CInterface
////////////
CInterface::CInterface(CEnvironment * parent, QDBusObjectPath dbusPath) :
	CLibNut(parent),
	/*parent(parent),*/
	m_dbusPath(dbusPath),
	log(parent->log),
	m_dbusInterface(0),
	m_needUserSetup(false),
	m_index(-1),
	m_propertiesFetched(false),
	m_configFetched(false),
	m_userConfigFetched(false),
	m_needUserSetupFeteched(false),
	m_initCompleted(false)
{
	//Attach to dbus
	m_dbusConnection = parent->m_dbusConnection;
	m_dbusConnectionInterface = parent->m_dbusConnectionInterface;
}//TODO::init all vars to default

void CInterface::init() {
	if (!serviceCheck()) {
		emit initializationFailed(this);
		emit dbusErrorOccured();
		return;
	}

	m_dbusInterface = new DBusInterfaceInterface_IPv4(NUT_DBUS_URL, m_dbusPath.path(), *m_dbusConnection, this);

	connect(m_dbusInterface, SIGNAL(errorOccured(QDBusError,QString)), this, SLOT(dbusret_errorOccured(QDBusError)));

	connect(m_dbusInterface,SIGNAL(gotProperties(libnutcommon::InterfaceProperties)), this, SLOT(dbusretGetProperties(libnutcommon::InterfaceProperties)));

	connect(m_dbusInterface,SIGNAL(gotConfig(libnutcommon::IPv4Config)),this,SLOT(dbusretGetConfig(libnutcommon::IPv4Config)));

	connect(m_dbusInterface,SIGNAL(gotUserConfig(libnutcommon::IPv4UserConfig)),this,SLOT(dbusretGetUserConfig(libnutcommon::IPv4UserConfig)));

	connect(m_dbusInterface,SIGNAL(gotNeedUserSetup(bool)),this,SLOT(dbusretGetNeedUserSetup(bool)));
	
	connect(m_dbusInterface,SIGNAL(gotSetUserConfig(bool)),this,SLOT(dbusretSetUserConfig(bool)));

	connect(m_dbusInterface, SIGNAL(stateChanged(libnutcommon::InterfaceProperties)), this, SLOT(dbusStateChanged(libnutcommon::InterfaceProperties)));

// 	*log << tr("Placing getProperties call at: %1").arg(m_dbusPath.path());
	m_dbusInterface->getProperties();

// 	*log << tr("Placing getConfig call at: %1").arg(m_dbusPath.path());
	m_dbusInterface->getConfig();

// 	*log << tr("Placing getUserConfig call at: %1").arg(m_dbusPath.path());
	m_dbusInterface->getUserConfig();

// 	*log << tr("Placing needUserSetup call at: %1").arg(m_dbusPath.path());
	m_dbusInterface->needUserSetup();
}

CInterface::~CInterface() {
}

void CInterface::checkInitCompleted() {
	if ( m_propertiesFetched && m_configFetched && m_userConfigFetched && m_needUserSetupFeteched && !m_initCompleted) {
		m_initCompleted = true;
		emit initializationCompleted(this);
	}
}

//dbus return functions
void CInterface::dbusretGetProperties(libnutcommon::InterfaceProperties properties) {
	m_state = properties.ifState;
	m_ip = properties.ip;
	m_netmask = properties.netmask;
	m_gateway = properties.gateway;
	m_dnsservers = properties.dns;

	m_propertiesFetched = true;
	checkInitCompleted();

	emit newDataAvailable();
}

void CInterface::dbusretGetConfig(libnutcommon::IPv4Config config) {
	m_config = config;
	
	m_configFetched = true;
	checkInitCompleted();
	
	emit newDataAvailable();
}


void CInterface::dbusretGetNeedUserSetup(bool need) {
	m_needUserSetup = need;

	m_needUserSetupFeteched = true;
	checkInitCompleted();
	
	emit newDataAvailable();
}

void CInterface::dbusretSetUserConfig(bool worked) {
	m_dbusInterface->getUserConfig(); //in case someone else set one and a cached one is wrong
	emit setUserConfig(worked);
}

void CInterface::dbusretGetUserConfig(libnutcommon::IPv4UserConfig config) {
	m_userConfig = config;
	
	m_userConfigFetched = true;
	checkInitCompleted();
	
	emit newDataAvailable();	
}

void CInterface::dbusret_errorOccured(QDBusError error, QString method) {
	*log << QString("Error occured in dbus: %s at %s").arg(QDBusError::errorString(error.type()).toAscii().data(), method.toAscii().data());
	if (!m_initCompleted) { //error during init
		emit initializationFailed(this);
	}
	if (!serviceCheck()) {
		emit dbusErrorOccured();
	}
}

//CInterface private functions:
void CInterface::refreshAll() {
// 	*log << tr("Placing getProperties call at: %1").arg(m_dbusPath.path());
	m_dbusInterface->getProperties();

// 	*log << tr("Placing getConfig call at: %1").arg(m_dbusPath.path());
	m_dbusInterface->getConfig();

// 	*log << tr("Placing getUserConfig call at: %1").arg(m_dbusPath.path());
	m_dbusInterface->getUserConfig();

// 	*log << tr("Placing needUserSetup call at: %1").arg(m_dbusPath.path());
	m_dbusInterface->needUserSetup();
}
//CInterface private slots
void CInterface::dbusStateChanged(libnutcommon::InterfaceProperties properties) {
	//Check changes:
	m_state = properties.ifState;
	m_ip = properties.ip;
	m_netmask = properties.netmask;
	m_gateway = properties.gateway;
	m_dnsservers = properties.dns;
	m_dbusInterface->needUserSetup(); //refresh needUserSetup information
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
	m_dbusInterface->needUserSetup();
	return m_needUserSetup;
}

void CInterface::setUserConfig(const libnutcommon::IPv4UserConfig &cuserConfig) {
	m_dbusInterface->setUserConfig(cuserConfig);
}

libnutcommon::IPv4UserConfig CInterface::getUserConfig(bool refresh) {
	if (refresh) {
		m_dbusInterface->getUserConfig();
	}
	return m_userConfig;
}

}
