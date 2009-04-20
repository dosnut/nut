#ifndef LIBNUTCLIENT_CLOG_H
#define LIBNUTCLIENT_CLOG_H

#include <QObject>
#include <QList>
#include <QFile>
#include <QTextStream>

namespace libnutclient {
	/** @brief CLog provides a logging facility for the client
		
		The Log can be used to save the logging information to a file.
		You can get access to the text via the printed signal,
		which is called every time the logging functions are invoked.
	*/
	class CLog : public QObject {
		Q_OBJECT
	private:
		QFile m_file;
		QTextStream m_outStream;
		bool m_fileLoggingEnabled;
	public:
		CLog(QObject * parent, QString fileName);
		inline QFile::FileError error() const {
			return m_file.error();
		}
		inline bool getFileLoggingEnabled() const {
			return m_fileLoggingEnabled;
		}
		inline void setFileLoggingEnabled(bool isEnabled) {
			m_fileLoggingEnabled = isEnabled && (m_file.error() == QFile::NoError);
		}
		void operator<<(QString text);
	public slots:
		void log(QString text) { operator<<(text); }
	signals:
		void printed(const QString & line);
	};
}
#endif
