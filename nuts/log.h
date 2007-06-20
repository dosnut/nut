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

#include <QTextStream>
#include <QFile>

namespace nuts {
	/**
		@author Stefan Bühler <stbuehler@web.de>
	*/
	class Log : public QTextStream {
		private:
			QFile *file;
		public:
			Log(int fd);
			virtual ~Log();
	};
	extern Log err, log;
};

#endif
