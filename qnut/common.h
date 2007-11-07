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

};

#endif
