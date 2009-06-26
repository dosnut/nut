/*
        TRANSLATOR libnutclient::CLibNut
*/
#include "clibnut.h"
#include "libnutcommon/common.h"

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
		case DT_ETH:    return CLibNut::tr("Ethernet");
		case DT_AIR:    return CLibNut::tr("Wireless");
		case DT_PPP:    return CLibNut::tr("PPP");
		case DT_BRIDGE: return CLibNut::tr("Bridge");
		default:        return QString();
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
