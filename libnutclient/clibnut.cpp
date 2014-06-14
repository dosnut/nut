/*
		TRANSLATOR libnutclient::CLibNut
*/
#include "clibnut.h"
#include "libnutcommon/common.h"
#include <QList>

namespace libnutclient {
using namespace libnutcommon;
QString toStringTr(DeviceState state) {
	switch (state) {
		case DeviceState::UP:             return CLibNut::tr("up");
		case DeviceState::UNCONFIGURED:   return CLibNut::tr("unconfigured");
		case DeviceState::CARRIER:        return CLibNut::tr("got carrier");
		case DeviceState::ACTIVATED:      return CLibNut::tr("activated");
		case DeviceState::DEACTIVATED:    return CLibNut::tr("deactivated");
		default:                return QString();
	}
}
QString toStringTr(DeviceType type) {
	switch (type) {
		case DeviceType::ETH:    return CLibNut::tr("Ethernet");
		case DeviceType::AIR:    return CLibNut::tr("Wireless");
		case DeviceType::PPP:    return CLibNut::tr("PPP");
		case DeviceType::BRIDGE: return CLibNut::tr("Bridge");
		default:        return QString();
	}
}
QString toStringTr(InterfaceState state) {
	switch (state) {
		case InterfaceState::OFF: return CLibNut::tr("off");
		case InterfaceState::STATIC: return CLibNut::tr("static");
		case InterfaceState::DHCP: return CLibNut::tr("dynamic");
		case InterfaceState::ZEROCONF: return CLibNut::tr("zeroconf");
		case InterfaceState::WAITFORCONFIG: return CLibNut::tr("wait for config");
		default: return QString();
	}
}

QString toString(QDBusError error) {
	return QDBusError::errorString(error.type());
}

////////////////
//CLibNut
///////////////
//Check if service up
bool CLibNut::serviceCheck() {
	if (!m_dbusConnection) {
		return false;
	}
	else if (!m_dbusConnection->isConnected()) {
		return false;
	}

	if (m_dbusConnectionInterface) {
		QDBusReply<bool> reply = m_dbusConnectionInterface->isServiceRegistered(NUT_DBUS_URL);
		if (reply.isValid()) {
			if (!reply.value()) {
				return false;
			}
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
	return true;
}

}
