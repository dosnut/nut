#include "cconfigparser.h"
#include <QFile>
#include <QString>

extern "C" {
#include <stdio.h>
}

void configparserparse(libnutwireless::CConfigParser * cp);
extern FILE *configparserin;
extern libnutwireless::CConfigParser * libnutcconfigparser;

namespace libnutwireless {

	bool CConfigParser::parse() {
// 		configparserin = reinterpret_cast<FILE*>(this);
		configparserin = 0;
		libnutcconfigparser = this;
		configparserparse(this);
		return true;
	}
	
	void CConfigParser::parseError(int line_num, QString msg) {
		m_errors.append(QString("Error occured: %1 in Line %2").arg(msg,line_num));
	}
	
	int CConfigParser::readFromStream(char * buf, int /*max_size*/) {
		QString str = stream->read(m_maxRead);
		int size = str.size();
		memccpy(buf,str.toAscii().constData(),size,sizeof(char));
		return size;
	}


	bool CConfigParser::newNetwork() {
		if (currentNetwork)
			return false;
		currentNetwork = new CNetworkConfig();
		return true;
	}

	bool CConfigParser::finishNetwork() {
		if (!currentNetwork)
			return false;
		m_configs.push_back(*currentNetwork);
		delete currentNetwork;
		currentNetwork = 0;
		return true;
	}

}
