#include "cconfigparser.h"
#include <QFile>
#include <QString>

extern "C" {
#include <stdio.h>
}
#include <iostream>

int configparserparse(libnutwireless::CConfigParser * cp);
extern FILE *configparserin;
extern libnutwireless::CConfigParser * libnutcconfigparser;
int configparserlex_destroy(void);

namespace libnutwireless {

	bool CConfigParser::parse() {
// 		configparserin = reinterpret_cast<FILE*>(this);
		configparserin = 0;
		libnutcconfigparser = this;
		configparserparse(this);
		configparserlex_destroy();
		return true;
	}

	void CConfigParser::parseError(int line_num, QString msg) {
		m_errors.append(QString("Error occured: %1 in Line %2").arg(msg,line_num));
	}

	int CConfigParser::readFromStream(char * buf, int max_size) {
		QString str = stream->read(max_size-1);
		int size = str.size();
		memcpy(buf,str.toLatin1().constData(),(size+1)*sizeof(char));
		std::cout << buf << std::endl;
		return size;
	}


	bool CConfigParser::newNetwork() {
		if (currentNetwork) return false;
		currentNetwork = new CNetworkConfig();
		std::cout << "NEW NETWORK CREATED" << std::endl;
		return true;
	}

	bool CConfigParser::finishNetwork() {
		std::cout << "FINISHED THE NETWORK" << std::endl;
		if (!currentNetwork)
			return false;
		m_configs.push_back(*currentNetwork);
		delete currentNetwork;
		currentNetwork = 0;
		return true;
	}

}
