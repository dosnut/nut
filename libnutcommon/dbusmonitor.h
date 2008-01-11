#ifndef NUT_COMMON_DBUSMONITOR_H
#define NUT_COMMON_DBUSMONITOR_H
#include <QObject>
#include <QFile>
#include <QSocketNotifier>
#include <QDBusConnection>
#include <QString>


namespace libnutcommon {
	class CDBusMonitor;
}

namespace libnutcommon {

	class CDBusMonitor: public QObject {
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
			void setEnabled(bool enabled=true);
		public:
			CDBusMonitor(QObject * parent, QString dbusPidFileDir, QString dbusPidFileName);
			CDBusMonitor(QObject * parent);
			~CDBusMonitor();
			void setPidFileDir(QString dir);
			void setPidFileName(QString name);
		signals:
			void stopped();
			void started();
	};
}
#endif
