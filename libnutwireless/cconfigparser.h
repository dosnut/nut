#ifndef LIBNUTWIRELESS_CCONFIGPARSER_H
#define LIBNUTWIRELESS_CCONFIGPARSER_H

#include <QTextStream>
#include "cnetworkconfig.h"

//http://pintday.org/kjell/hack/lex
//http://www.ibm.com/developerworks/opensource/library/l-flexbison.html#downloads
//http://flex.sourceforge.net/manual/Multiple-Input-Buffers.html#Scanning%20Strings
//http://flex.sourceforge.net/manual/Generated-Scanner.html



namespace libnutwireless {

class CConfigParser {
	
	QList<CNetworkConfig> m_configs;
	QStringList m_errors;
	CNetworkConfig * currentNetwork;
	QTextStream * stream;
	QString fileName;
	qint64 m_maxRead;
	
	public:
		/** Stream has to be valid when calling parse() */
		CConfigParser(QTextStream * stream) : currentNetwork(0), stream(stream), m_maxRead(128*1024*1024)  {}
		CConfigParser(const QString fileName) : currentNetwork(0), stream(0), fileName(fileName), m_maxRead(128*1024*1024) {if (currentNetwork) delete currentNetwork;}
		
		
		bool parse();
		bool newNetwork();
		bool finishNetwork();
		
		inline QList<CNetworkConfig> getConfigs() { return m_configs; };
		inline QStringList& getErrors() { return m_errors; }
		inline void setMaxRead(qint64 v) { m_maxRead = v; }
		int readFromStream(char * buf,int max_size);
		inline CNetworkConfig * getCurrentNetwork() { return currentNetwork; }
		void parseError(int line_num, QString msg);
};

}
#endif