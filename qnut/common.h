#ifndef QNUT_COMMON_H
#define QNUT_COMMON_H

#include <libnut/libnut_cli.h>
#include <QString>
#include <QAction>
#include <QHostAddress>
#include <QDir>
#include "constants.h"

namespace qnut {
	using namespace libnut;
	QString iconFile(CDevice * device);
	QAction * getSeparator(QObject * parent);
	QString shortSummary(CDevice * device);
	QString activeIP(CDevice * device);
	inline QString toStringDefault(QHostAddress address) {
		if (address.isNull())
			return QObject::tr("none");
		else
			return address.toString();
	}
	
	inline QString signalSummary(wps_wext_scan signal) {
		return QString("%1/%2, %3/%4dBm, %5/%6dBm")
			.arg(signal.quality.qual)
			.arg(signal.maxquality.qual)
			.arg(signal.quality.level)
			.arg(signal.maxquality.level)
			.arg(signal.quality.noise)
			.arg(signal.maxquality.noise);
	}
	
	inline QString qualitySummary(wps_signal_quality quality) {
		return QString("%1, %2dBm, %3dBm")
			.arg(quality.qual)
			.arg(quality.level)
			.arg(quality.noise);
	}
};

#endif
