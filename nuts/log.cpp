#include "log.h"
extern "C" {
#include <unistd.h>
}

namespace nuts {
	QTextStream err;
	QTextStream log;

	class CLogInit {
	public:
		CLogInit() {
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
				ferr = new QFile(); ferr->open(2, QIODevice::WriteOnly);
				err.setDevice(ferr);
				fout = new QFile(); fout->open(1, QIODevice::WriteOnly);
				log.setDevice(fout);
			}
		}

		~CLogInit() {
			delete ferr;
			delete fout;
		}

	private:
		QFile *ferr = nullptr;
		QFile *fout = nullptr;
	};

	static CLogInit *loginit;
	void LogInit() {
		loginit = new CLogInit();
	}

	void LogDestroy() {
		delete loginit;
		loginit = nullptr;
	}
}
