#ifndef QNUT_COMMON_H
#define QNUT_COMMON_H

#include <QAction>
#include <QString>
#include <QHostAddress>
#include <libnut/libnut_cli.h>
#include "constants.h"

namespace qnut {
	QString iconFile(libnut::CDevice * device);
	QAction * getSeparator(QObject * parent);
	QString shortSummary(libnut::CDevice * device);
	QString activeIP(libnut::CDevice * device);
	
	inline QString toStringDefault(QHostAddress address) {
		if (address.isNull())
			return QObject::tr("none");
		else
			return address.toString();
	}
	
	inline QString signalSummary(libnut::wps_wext_scan signal) {
		return QString("%1/%2, %3/%4dBm, %5/%6dBm")
			.arg(signal.quality.qual)
			.arg(signal.maxquality.qual)
			.arg(signal.quality.level)
			.arg(signal.maxquality.level)
			.arg(signal.quality.noise)
			.arg(signal.maxquality.noise);
	}
	
	inline QString qualitySummary(libnut::wps_signal_quality quality) {
		return QString("%1, %2dBm, %3dBm")
			.arg(quality.qual)
			.arg(quality.level)
			.arg(quality.noise);
	}
};

#endif
