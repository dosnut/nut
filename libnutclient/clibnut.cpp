#include "clibnut.h"
#include "libnutcommon/common.h"
#include <QList>

namespace libnutclient {
	using namespace libnutcommon;

	QString toStringTr(DeviceState state) {
		switch (state) {
		case DeviceState::UP:             return QObject::tr("up");
		case DeviceState::UNCONFIGURED:   return QObject::tr("unconfigured");
		case DeviceState::CARRIER:        return QObject::tr("got carrier");
		case DeviceState::ACTIVATED:      return QObject::tr("activated");
		case DeviceState::DEACTIVATED:    return QObject::tr("deactivated");
		}
		return { };
	}

	QString toStringTr(DeviceType type) {
		switch (type) {
		case DeviceType::ETH:    return QObject::tr("Ethernet");
		case DeviceType::AIR:    return QObject::tr("Wireless");
		case DeviceType::PPP:    return QObject::tr("PPP");
		case DeviceType::BRIDGE: return QObject::tr("Bridge");
		}
		return { };
	}

	QString toStringTr(InterfaceState state) {
		switch (state) {
		case InterfaceState::OFF: return QObject::tr("off");
		case InterfaceState::STATIC: return QObject::tr("static");
		case InterfaceState::DHCP: return QObject::tr("dynamic");
		case InterfaceState::ZEROCONF: return QObject::tr("zeroconf");
		case InterfaceState::WAITFORCONFIG: return QObject::tr("wait for config");
		}
		return { };
	}

	QString toString(QDBusError const& error) {
		return QDBusError::errorString(error.type());
	}
}
