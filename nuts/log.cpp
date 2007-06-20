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
	Log::Log(int fd) {
		file = new QFile();
		file->open(fd, QIODevice::Append);
		setDevice(file);
	}
	Log::~Log() {
		delete file;
	}
	
	Log err(2), log(1);
};
