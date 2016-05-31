#ifndef _NUTS_LOG_H
#define _NUTS_LOG_H

#pragma once

#include <QObject>
#include <QTextStream>
#include <QFile>

namespace nuts {
	extern QTextStream err, log;

	void LogInit();
	void LogDestroy();
}

#endif /* _NUTS_LOG_H */
