#ifndef NUT_COMMON_DBUSMONITOR_H
#define NUT_COMMON_DBUSMONITOR_H
#include <QObject>
#include <QFile>
#include <QSocketNotifier>
#include <QDBusConnection>
#include <QString>
#include <fstream>

extern "C" {
#include <sys/inotify.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
}

namespace libnutcommon {

	class DBusMonitor: public QObject {
		Q_OBJECT
		private:
			QString m_dbusPidFileDir;
			QString m_dbusPidFileName;
			int m_dbusPid;
			int m_inotifyFd;
			int m_inWatchPidDirFd;
			QSocketNotifier * m_inotifiySocketNotifier;

			void setDBusPid();
			void setInotifier();

		private slots:
			void inotifyEvent(int socket);
		public slots:
			void setNetlinkFd(int socket);
			void setEnabled(bool enabled=true);
		public:
			DBusMonitor(QObject * parent, QString dbusPidFileDir, QString dbusPidFileName);
			DBusMonitor(QObject * parent) : QObject(parent) {}
			~DBusMonitor();
			void setPidFileDir(QString dir);
			void setPidFileName(QString name);
		signals:
			void stopped();
			void started();
	};
}
#endif
