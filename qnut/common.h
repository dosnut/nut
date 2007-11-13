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
	
	QString signalSummary(libnut::wps_wext_signal_readable signal);
	
	inline QString qualitySummary(libnut::wps_wext_signal_readable signal) {
		return signalSummary(signal);
	}
};

#endif
