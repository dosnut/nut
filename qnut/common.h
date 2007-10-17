#ifndef QNUT_COMMON_H
#define QNUT_COMMON_H

#include <libnut/libnut_cli.h>
#include <QString>
#include <QAction>
#include <QDir>
#include "constants.h"

namespace qnut {
	using namespace libnut;
	QString iconFile(CDevice * device);
	QAction * getSeparator(QObject * parent);
	QString shortSummary(CDevice * device);
	QString activeIP(CDevice * device);
};

#endif
