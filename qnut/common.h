#ifndef QNUT_COMMON_H
#define QNUT_COMMON_H

#include <QAction>
#include <QString>
#include <QHostAddress>
#include <libnutclient/libnut_client.h>
#include "constants.h"

namespace qnut {
	QString iconFile(libnutclient::CDevice * device);
	QAction * getSeparator(QObject * parent);
	QString shortSummary(libnutclient::CDevice * device);
	QString activeIP(libnutclient::CDevice * device);
	
	inline QString toStringDefault(QHostAddress address) {
		if (address.isNull())
			return QObject::tr("none");
		else
			return address.toString();
	}
	
	QString signalSummary(libnutwireless::WextSignal signal);
	
};

#endif
