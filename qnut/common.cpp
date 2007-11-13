#include "common.h"
#include <QObject>

namespace qnut {
	using namespace libnut;
	using namespace libnutws;
	
	QString iconFile(CDevice * device) {
		switch (device->type) {
		case DT_ETH:
			switch (device->state) {
			case DS_UP:             return QString(UI_ICON_ETH_UP);
			case DS_UNCONFIGURED:   return QString(UI_ICON_ETH_UNCONFIGURED);
			case DS_CARRIER:        return QString(UI_ICON_ETH_CARRIER);
			case DS_ACTIVATED:      return QString(UI_ICON_ETH_ACTIVATED);
			case DS_DEACTIVATED:    return QString(UI_ICON_ETH_DEACTIVATED);
			default:                break;
			}
		case DT_AIR:
			switch (device->state) {
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
		return device->name + ": " + toString(device->state) + ", " + activeIP(device);
	}

	QString activeIP(CDevice * device) {
		if (device->state < DS_UNCONFIGURED)
			return QString('-');
		
		QString result = QString("");
		
		foreach (CInterface * i, device->activeEnvironment->interfaces) {
			if (result.length() > 0) {
				result += " (...)";
				break;
			}
			else if (i->state != IFS_OFF) {
				result += i->ip.toString();
			}
		}
		
		if (result.length() > 0)
			return result;
		else
			return QString('-');
	}
	
	QString signalSummary(libnutws::wps_wext_signal_readable signal) {
		QString quality = QString::number(signal.quality.value) + '/' + QString::number(signal.quality.maximum);
		QString level/* = QString::number(signal.quality.level) +
			((signal.encoding & WSIG_LEVEL_REL)   ? '/' + QString::number(signal.maxquality.level) : QString())*/;
		QString noise/* = QString::number(signal.quality.noise) +
			((signal.encoding & WSIG_NOISE_REL)   ? '/' + QString::number(signal.maxquality.noise) : QString())*/;
		
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
