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
	QTextStream err;
	QTextStream log;

	class LogInit {
		public:
			LogInit() {
				int fd1 = dup(1), fd2 = dup(2);
				close(fd1); close(fd2);
				if (fd1 == -1 || fd2 == -1) {
					QFile *f = new QFile("/var/log/nuts.log");
					f->open(QIODevice::Append);
					dup2(f->handle(), 2);
					dup2(f->handle(), 1);
					err.setDevice(f);
					log.setDevice(f);
				} else {
					QFile *ferr = new QFile(); ferr->open(2, QIODevice::WriteOnly);
					err.setDevice(ferr);
					QFile *fout = new QFile(); fout->open(1, QIODevice::WriteOnly);
					log.setDevice(fout);
				}
			}
	};
	
	static LogInit loginit;
};
