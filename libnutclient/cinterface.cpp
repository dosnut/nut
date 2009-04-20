#include "cinterface.h"
#include "libnutcommon/common.h"
#include "cenvironment.h"
#include "server_proxy.h"
#include "client_exceptions.h"
#include "clog.h"

// #include <QDBusObjectPath>
// #include <QDBusError>

namespace libnutclient {
using namespace libnutcommon;

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
