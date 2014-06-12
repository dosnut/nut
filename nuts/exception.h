//
// C++ Interface: exception
//
// Description:
//
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef _NUTS_EXCEPTION_H
#define _NUTS_EXCEPTION_H

#include <QString>

namespace nuts {
	class Exception;
}

#include <exception>

namespace nuts {
	class Exception : public std::exception {
		private:
			QString _msg;
		public:
			Exception(const QString &msg) : _msg(msg) { }
			virtual ~Exception() throw() { }
			virtual const char* what() const throw() {
				return _msg.toUtf8().constData();
			}
			virtual const QString& msg() const throw() {
				return _msg;
			}
	};

	class NetlinkInitException : public Exception {
		public:
			NetlinkInitException(const QString &msg) : Exception(msg) { }
	};

	class EthtoolException : public Exception {
		public:
			EthtoolException(const QString &msg) : Exception(msg) { }
	};
	class EthtoolInitException : public EthtoolException {
		public:
			EthtoolInitException(const QString &msg) : EthtoolException(msg) { }
	};
}


#endif
