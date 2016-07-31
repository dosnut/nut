#ifndef LIBNUTWIRELESS_CCONFIGPARSER_H
#define LIBNUTWIRELESS_CCONFIGPARSER_H

#pragma once

#include <memory>

#include <QTextStream>

#include "cnetworkconfig.h"

//http://pintday.org/kjell/hack/lex
//http://www.ibm.com/developerworks/opensource/library/l-flexbison.html#downloads
//http://flex.sourceforge.net/manual/Multiple-Input-Buffers.html#Scanning%20Strings
//http://flex.sourceforge.net/manual/Generated-Scanner.html

namespace libnutwireless {
	class CConfigParser final {
	public:
		/** Stream has to be valid when calling parse() */
		explicit CConfigParser(QTextStream* stream)
		: m_stream(stream) {}

		QList<CNetworkConfig> const& getConfigs() const { return m_configs; }
		QStringList const& getErrors() const { return m_errors; }

		/* parser internals */
		bool parse();
		bool newNetwork();
		bool finishNetwork();
		CNetworkConfig* getCurrentNetwork() { return m_currentNetwork.get(); }
		int readFromStream(char* buf, int max_size);
		void parseError(int line_num, QString msg);

	private:
		QList<CNetworkConfig> m_configs;
		QStringList m_errors;
		std::unique_ptr<CNetworkConfig> m_currentNetwork;

		QTextStream* const m_stream{nullptr};
	};
}
#endif /* LIBNUTWIRELESS_CCONFIGPARSER_H */
