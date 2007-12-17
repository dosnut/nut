//
// C++ Implementation: common
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include "common.h"

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
};
