#include "dbusmonitor.h"
#include <QDebug>
#include <fstream>
extern "C" {
#include <sys/inotify.h>
#include <errno.h>
#include <unistd.h>
}
namespace libnutcommon {

	CDBusMonitor::CDBusMonitor(QObject * parent, QString dbusPidFileDir, QString dbusPidFileName) : QObject(parent), m_dbusPidFileDir(dbusPidFileDir), m_dbusPidFileName(dbusPidFileName), m_dbusPid(-1), m_inotifyFd(-1), m_inWatchPidDirFd(-1), m_inotifiySocketNotifier(0) {
		qDebug() << "File dir to pidfile is" << m_dbusPidFileDir;
		qDebug() << "File name of pidfile is " << m_dbusPidFileName;
	}
	CDBusMonitor::CDBusMonitor(QObject * parent) : QObject(parent), m_dbusPid(-1), m_inotifyFd(-1), m_inWatchPidDirFd(-1), m_inotifiySocketNotifier(0)
	{}

	CDBusMonitor::~CDBusMonitor() {
		if (-1 != m_inotifyFd) {
			if (-1 != m_inWatchPidDirFd) {
				inotify_rm_watch(m_inotifyFd,m_inWatchPidDirFd);
				m_inWatchPidDirFd = -1;
			}
		}
	}

	bool CDBusMonitor::setPidFileDir(QString dir) {
		qDebug() << "Trying to set watch dir to " << dir;
		if (QFile::exists(dir)) {
			m_dbusPidFileDir = dir;
			return true;
		}
		else {
			qDebug() << "Failed setting watchdir";
			return false;
		}
	}

	void CDBusMonitor::setPidFileName(QString name) {
		qDebug() << "Pid file name is " << name;
		m_dbusPidFileName = name;
	}

	void CDBusMonitor::setEnabled(bool enabled) {
		if (enabled) {
			if ( !(!m_dbusPidFileDir.isEmpty() && !m_dbusPidFileName.isEmpty()) ) {
				qDebug() << "Failed enabling monitor";
				return;
			}
			if (-1 == m_inotifyFd) {
				m_inotifyFd = inotify_init();
			}
			qDebug() << "Filedescriptor for inotify is " << m_inotifyFd;

			if ( (-1 != m_inotifyFd) && (NULL == m_inotifiySocketNotifier) ) {

				m_inotifiySocketNotifier = new QSocketNotifier(m_inotifyFd,QSocketNotifier::Read,this);
				if (0 != m_inotifiySocketNotifier) {
					connect(m_inotifiySocketNotifier,SIGNAL(activated( int )),this,SLOT(inotifyEvent(int)));
					m_inotifiySocketNotifier->setEnabled(true);
					qDebug() << "Connected SocketNotifier for inotify";
				//Set dbus pid and add the watches
				setDBusPid();
				setInotifier();
				}
				else {
					qWarning() << "Could not initalize SocketNotifier for inotify";
				}
			}
			else {
				qWarning() << "Disabling inotify support";
			}
		}
		else {
			//Kill socket notifier
			if (m_inotifiySocketNotifier) {
				delete m_inotifiySocketNotifier;
			}
			//remove watch
			if (-1 != m_inWatchPidDirFd) {
				inotify_rm_watch(m_inotifyFd,m_inWatchPidDirFd);
				m_inWatchPidDirFd = -1;
			}
		}
	}

	void CDBusMonitor::setDBusPid() {
		//Check in /var/run first then in /var/run/dbus
		int buffer = 0;
		QString dbusPidFilePath = m_dbusPidFileDir + "/" + m_dbusPidFileName;
		if ( !m_dbusPidFileDir.isEmpty() && !m_dbusPidFileName.isEmpty() ) {
			QByteArray path = dbusPidFilePath.toAscii();
			std::ifstream pidfile;
			pidfile.open(path.constData());
			pidfile >> buffer;
			pidfile.close();
			if ( (0 != buffer) && QFile::exists(QString("/proc/%1").arg(QString::number(buffer))) ) {
				m_dbusPid = buffer;
			}
			else {
				m_dbusPid = -1;
			}
		}
		else {
			m_dbusPid = -1;
		}
		qDebug() << "Set dbus pid to:" << m_dbusPid;
	}

	void CDBusMonitor::setInotifier() {
		qDebug() << "(Inotify) Setting up watches";
		//Setup watch
		if ( -1 != m_inotifyFd ) {
			//Setup pidfile directory watch
			if ( QFile::exists(m_dbusPidFileDir) && (-1 == m_inWatchPidDirFd) ) {
				m_inWatchPidDirFd = inotify_add_watch(m_inotifyFd,m_dbusPidFileDir.toAscii().constData(), IN_DELETE | IN_MODIFY);
				qDebug() << "Setup Pid file dir watch with " << m_inWatchPidDirFd;
			}
		}
	}

	void CDBusMonitor::inotifyEvent(int socket) {
		qDebug() << "(Inotify) Event occured";
		if (socket == m_inotifyFd) {
	// 		struct inotify_event;
			unsigned char * buffer = NULL;
			unsigned char * newbuf;
			int bufferlen = sizeof(struct inotify_event) + 16;
			int datalen = 0;
			while (datalen == 0) {
				//allocate buffer;
				newbuf = (unsigned char *) realloc(buffer,bufferlen);
				if (NULL != newbuf) {
					buffer = newbuf;
					memset(buffer,0,bufferlen);
				}
				else {
					if (buffer) {
						free(buffer);
						buffer = NULL;
					}
					qDebug() << "Error allocating buffer";
					return;
				}
				qDebug() << "(Inotify) Reading";
				datalen = read(m_inotifyFd,buffer,bufferlen);
				qDebug() << "(Inotify) Read" << strerror(errno) << datalen;
				if ( datalen <= 0) {
					datalen = 0;
					bufferlen *= 2;
				}
			}
			int currentIndex = 0;
			struct inotify_event * event;
			QString filename;
			while (currentIndex+1 < datalen) {
				//Cast and set next event
				event = (struct inotify_event *) (buffer+currentIndex);
				currentIndex += sizeof(struct inotify_event) + event->len;
				filename = "";
				if (event->len > 0) {
					filename = QString::fromAscii(event->name);
					qDebug() << "Filename is";
					qDebug() << filename;
					qDebug() << "Size: " << filename.size();
					//Print out event type
					if (event->mask & IN_CREATE) {
						qDebug() << "(Inotify) created file";
					}
					if (event->mask & IN_DELETE) {
						qDebug() << "File in dir was deleted";
					}
					if (event->mask & IN_MODIFY) {
						qDebug() << "(Inotify) modify file";
					}
					if (event->mask & IN_DELETE_SELF) {
						qDebug() << "(Inotify) deleted directory";
					}
				}
				//process events

				//Check watch:
				qDebug() << "Watch file descriptor is " << m_inWatchPidDirFd << "from event" <<  event->wd;

				if (event->wd == m_inWatchPidDirFd) { //Pid file
					//Check file name
					if ( (filename == m_dbusPidFileName) && (-1 != m_inWatchPidDirFd) ) {
						qDebug() << "Filename is pid file name";
						//This is a work around untill taskstats interface is ready:
						if (event->mask & IN_MODIFY) {
							//set new dbuspid and set inotifier watch
							setDBusPid();
							setInotifier();
							qDebug() << "(Inotify) Dbus was started";
							emit started();
						}
						else if (event->mask & IN_DELETE) {
							qDebug() << "(Inotify) Dbus was stopped";
							emit stopped();
						}
					}
				}
			}
			//Free buffer
			if (buffer) {
				free(buffer);
			}
		}
		else {
			qWarning() << "Unknown socket in inotify event: " << m_inotifyFd << socket;
		}
		qDebug() << "(Inotify) Done processing event";
	}

}

