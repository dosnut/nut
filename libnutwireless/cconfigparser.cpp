#include "cconfigparser.h"
#include <QFile>
#include <QString>
#include <QTextCodec>

#include <iostream>

int configparserparse(libnutwireless::CConfigParser * cp);
extern FILE *configparserin;
extern libnutwireless::CConfigParser * libnutcconfigparser;
int configparserlex_destroy(void);

namespace libnutwireless {
	bool CConfigParser::parse() {
		// read as latin1; converting it back to latin1 below (should be binary/UTF-8 safe)
		m_stream->setCodec(QTextCodec::codecForName("ISO-8859-1"));
		configparserin = nullptr;
		libnutcconfigparser = this;
		configparserparse(this);
		configparserlex_destroy();
		libnutcconfigparser = nullptr;
		return true;
	}

	void CConfigParser::parseError(int line_num, QString msg) {
		m_errors.append(QString("Error occured: %1 in line %2").arg(msg, line_num));
	}

	int CConfigParser::readFromStream(char* buf, int max_size) {
		QString str = m_stream->read(max_size - 1);
		int size = str.size();
		::memcpy(buf, str.toLatin1().constData(), size + 1);
		// std::cout << buf << std::endl;
		return size;
	}

	bool CConfigParser::newNetwork() {
		if (m_currentNetwork) return false;
		m_currentNetwork.reset(new CNetworkConfig());
		// std::cout << "NEW NETWORK CREATED" << std::endl;
		return true;
	}

	bool CConfigParser::finishNetwork() {
		// std::cout << "FINISHED THE NETWORK" << std::endl;
		if (!m_currentNetwork) return false;
		m_configs.push_back(std::move(*m_currentNetwork));
		m_currentNetwork.reset();
		return true;
	}
}
