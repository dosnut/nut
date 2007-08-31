//
// C++ Interface: log
//
// Description: 
//
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef _NUTS_LOG_H
#define _NUTS_LOG_H

#include <QObject>

namespace nuts {
	class Log;
};

#include <QObject>
#include <QTextStream>
#include <QFile>

namespace nuts {
	/**
		@author Stefan Bühler <stbuehler@web.de>
	*/
	extern QTextStream err, log;
};

#endif
