//
// C++ Implementation: common
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
/*
        TRANSLATOR qnut::QObject
*/
#include "common.h"

#include <libnutclient/client.h>
#include <libnutwireless/wpa_supplicant.h>
#include <libnutwireless/cwireless.h>

#include "constants.h"

namespace qnut {
	using namespace libnutcommon;
	using namespace libnutclient;
	using namespace libnutwireless;

	QString iconFile(CDevice * device, bool stateAware) {
		libnutcommon::DeviceState state = stateAware ? device->getState() : DeviceState::ACTIVATED;

		switch (device->getType()) {
		case DeviceType::ETH:
			switch (state) {
			case DeviceState::UP:             return QString(UI_ICON_ETH_UP);
			case DeviceState::UNCONFIGURED:   return QString(UI_ICON_ETH_UNCONFIGURED);
			case DeviceState::CARRIER:        return QString(UI_ICON_ETH_CARRIER);
			case DeviceState::ACTIVATED:      return QString(UI_ICON_ETH_ACTIVATED);
			case DeviceState::DEACTIVATED:    return QString(UI_ICON_ETH_DEACTIVATED);
			default:                break;
			}
		case DeviceType::AIR:
			switch (state) {
			case DeviceState::UP:             return QString(UI_ICON_AIR_UP);
			case DeviceState::UNCONFIGURED:   return QString(UI_ICON_AIR_UNCONFIGURED);
			case DeviceState::CARRIER:        return QString(UI_ICON_AIR_CARRIER);
			case DeviceState::ACTIVATED:      return QString(UI_ICON_AIR_ACTIVATED);
			case DeviceState::DEACTIVATED:    return QString(UI_ICON_AIR_DEACTIVATED);
			default:                break;
			}
		case DeviceType::BRIDGE:
			switch (state) {
			case DeviceState::UP:             return QString(UI_ICON_BRIDGE_UP);
			case DeviceState::UNCONFIGURED:   return QString(UI_ICON_BRIDGE_UNCONFIGURED);
			case DeviceState::CARRIER:        return QString(UI_ICON_BRIDGE_CARRIER);
			case DeviceState::ACTIVATED:      return QString(UI_ICON_BRIDGE_ACTIVATED);
			case DeviceState::DEACTIVATED:    return QString(UI_ICON_BRIDGE_DEACTIVATED);
			default:                break;
			}
		default:
			break;
		}
		return QString();
	}

	QAction * getSeparator(QObject * parent) {
		QAction * separator = new QAction(parent);
		separator->setSeparator(true);
		return separator;
	}

	QString shortSummary(CDevice * device) {
		QString result = device->getName() + ": " + toStringTr(device->getState());

		if (device->getState() > DeviceState::ACTIVATED)
			result += ' ' + ('(' + currentNetwork(device, false)) + ')';

		if (device->getState() > DeviceState::CARRIER)
			result += ", " + activeIP(device);

		return result;
	}

	QString detailsSummary(CDevice * device) {
		QString result = QObject::tr("Type: %1").arg(toStringTr(device->getType())) + '\n' +
			QObject::tr("State: %1").arg(toStringTr(device->getState()));

		if (device->getState() >= DeviceState::UNCONFIGURED) {
			result += ' ';
			result += '(' + activeIP(device) + ')';
			if (device->getType() == DeviceType::AIR)
				result += '\n' + QObject::tr("Connected to: %1").arg(currentNetwork(device));
		}

		return result;
	}

	QString currentNetwork(CDevice * device, bool appendQuality) {
		switch (device->getType()) {
		case DeviceType::ETH:
			return QObject::tr("local");
		case DeviceType::AIR:
#ifdef NUT_NO_WIRELESS
			return device->getEssid(); // this is buggy on newer kernels
#else
			if (device->getWireless()) {
				SignalQuality signal = device->getWireless()->getHardware()->getSignalQuality();
				QString result = signal.ssid.autoQuoteHexString();

				if (result.isEmpty()) {
					result = device->getEssid();
				}

				if (result.isEmpty()) {
					result = QObject::tr("unknown Network");
				}

				if (appendQuality && !signal.ssid.data().isEmpty()) {
					result += " (" +
						QString::number(signal.quality.value) + '/'+
						QString::number(signal.quality.maximum) + ')';
				}

				return result;
			}
#endif
		default:
			return QObject::tr("unknown Network");
		}
	}

	QString activeIP(CDevice* device) {
		if (device->getState() < DeviceState::UNCONFIGURED)
			return QString('-');

		QString result;

		for(auto i: device->getActiveEnvironment()->getInterfaces()) {
			if (InterfaceState::OFF != i->getState()) {
				if (result.length() == 0)
					result = i->getIp().toString();
				else {
					result += " (...)";
					break;
				}
			}
		}

		if (result.length() > 0)
			return result;
		else
			return QString('-');
	}
}
