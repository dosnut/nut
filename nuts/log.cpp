//
// C++ Implementation: log
//
// Description: 
//
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "log.h"

namespace nuts {
	void Log_Init(QTextStream &s, int fd) {
		QFile* file = new QFile();
		file->open(fd, QIODevice::WriteOnly);
		s.setDevice(file);
	}

	QTextStream err, log;
};
