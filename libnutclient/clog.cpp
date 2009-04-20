#include "clog.h"
namespace libnutclient {
////////////////
//CLog
///////////////
CLog::CLog(QObject * parent, QString fileName) : QObject(parent), m_file(fileName) {
	m_fileLoggingEnabled = m_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
	
	if (m_fileLoggingEnabled) {
		m_outStream.setDevice(&m_file);
	}
}

void CLog::operator<<(QString text) {
	emit printed(text);
	
	if (m_fileLoggingEnabled) {
		m_outStream << text << endl;
	}
}

}
