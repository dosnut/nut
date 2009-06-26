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
#include <libnutclient/cdevice.h>
#include <libnutclient/cenvironment.h>
#include <libnutclient/cinterface.h>
#include <libnutwireless/wpa_supplicant.h>
#include "common.h"
#include "constants.h"

namespace qnut {
	using namespace libnutcommon;
	using namespace libnutclient;
	using namespace libnutwireless;
	
	QString iconFile(CDevice * device) {
		switch (device->getType()) {
		case DT_ETH:
			switch (device->getState()) {
			case DS_UP:             return QString(UI_ICON_ETH_UP);
			case DS_UNCONFIGURED:   return QString(UI_ICON_ETH_UNCONFIGURED);
			case DS_CARRIER:        return QString(UI_ICON_ETH_CARRIER);
			case DS_ACTIVATED:      return QString(UI_ICON_ETH_ACTIVATED);
			case DS_DEACTIVATED:    return QString(UI_ICON_ETH_DEACTIVATED);
			default:                break;
			}
		case DT_AIR:
			switch (device->getState()) {
			case DS_UP:             return QString(UI_ICON_AIR_UP);
			case DS_UNCONFIGURED:   return QString(UI_ICON_AIR_UNCONFIGURED);
			case DS_CARRIER:        return QString(UI_ICON_AIR_CARRIER);
			case DS_ACTIVATED:      return QString(UI_ICON_AIR_ACTIVATED);
			case DS_DEACTIVATED:    return QString(UI_ICON_AIR_DEACTIVATED);
			default:                break;
			}
		case DT_BRIDGE:
			switch (device->getState()) {
			case DS_UP:             return QString(UI_ICON_BRIDGE_UP);
			case DS_UNCONFIGURED:   return QString(UI_ICON_BRIDGE_UNCONFIGURED);
			case DS_CARRIER:        return QString(UI_ICON_BRIDGE_CARRIER);
			case DS_ACTIVATED:      return QString(UI_ICON_BRIDGE_ACTIVATED);
			case DS_DEACTIVATED:    return QString(UI_ICON_BRIDGE_DEACTIVATED);
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
		if (device->getState() < DS_UNCONFIGURED)
			return device->getName() + ": " + toStringTr(device->getState());
		else
			return device->getName() + ": " + toStringTr(device->getState()) + ", " + activeIP(device);
	}

	QString detailsSummary(CDevice * device) {
		QString result = QObject::tr("Type: %1").arg(toStringTr(device->getType())) + '\n' +
			QObject::tr("State: %1").arg(toStringTr(device->getState()));
		
		if (device->getState() >= DS_UNCONFIGURED) {
			result += ' ';
			result += '(' + activeIP(device) + ')';
			if (device->getType() == DT_AIR)
				result += '\n' + QObject::tr("Connected to: %1").arg(currentNetwork(device));
		}
		
		return result;
	}
	
	QString currentNetwork(CDevice * device, bool appendQuality) {
#ifndef QNUT_NO_WIRELESS
		if (appendQuality && device->getWpaSupplicant()) {
			WextSignal signal = device->getWpaSupplicant()->getSignalQuality();
			return device->getEssid() + " (" +
				QString::number(signal.quality.value) + '/'+
				QString::number(signal.quality.maximum) + ')';
		}
		else
#endif
			return (device->getType() == DT_AIR) ? device->getEssid() : QObject::tr("local");
	}
	
	QString activeIP(CDevice * device) {
		if (device->getState() < DS_UNCONFIGURED)
			return QString('-');
		
		QString result = QString("");
		
		foreach (CInterface * i, device->getActiveEnvironment()->getInterfaces()) {
			if (i->getState() != IFS_OFF) {
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
