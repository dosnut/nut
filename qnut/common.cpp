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
			return device->getName() + ": " + toStringTr(device->getState()) + ", " + activeIP(device);
		else
			return device->getName() + ": " + toStringTr(device->getState());
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
	
	QString signalSummary(libnutwireless::WextSignal signal) {
		QString quality = QString::number(signal.quality.value) + '/' + QString::number(signal.quality.maximum);
		QString level;
		QString noise;
		
		switch (signal.type) {
		case WSR_RCPI:
			level = QString::number(signal.level.rcpi);
			noise = QString::number(signal.noise.rcpi);
			break;
		case WSR_ABSOLUTE:
			level = QString::number(signal.level.nonrcpi.value);
			noise = QString::number(signal.noise.nonrcpi.value);
			break;
		case WSR_RELATIVE:
			level = QString::number(signal.level.nonrcpi.value) + '/' + QString::number(signal.level.nonrcpi.maximum);
			noise = QString::number(signal.noise.nonrcpi.value) + '/' + QString::number(signal.noise.nonrcpi.maximum);
			break;
		default:
			level = '-';
			noise = '-';
		}
		
		return QString("%1, %2 dBm, %3 dBm").arg(quality, level, noise);
	}
};
