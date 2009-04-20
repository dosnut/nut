#include "cenvironment.h"
#include "libnutcommon/common.h"
#include "server_proxy.h"
#include "clog.h"
#include "client_exceptions.h"

namespace libnutclient {
using namespace libnutcommon;

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

}
