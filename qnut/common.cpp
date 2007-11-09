#include "common.h"
#include <QObject>

namespace qnut {
	using namespace libnut;
	
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
	
	QString signalSummary(libnut::wps_wext_scan_readable signal) {
		QString quality = QString::number(signal.quality.qual)  +
			((signal.encoding & WSIG_QUALITY_REL) ? '/' + QString::number(signal.maxquality.qual)  : QString());
		QString level   = QString::number(signal.quality.level) +
			((signal.encoding & WSIG_LEVEL_REL)   ? '/' + QString::number(signal.maxquality.level) : QString());
		QString noise   = QString::number(signal.quality.noise) +
			((signal.encoding & WSIG_NOISE_REL)   ? '/' + QString::number(signal.maxquality.noise) : QString());
		
		return QString("%1, %2dBm, %3dBm").arg(quality, level, noise);
	}
};
