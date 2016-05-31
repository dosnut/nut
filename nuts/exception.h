#ifndef _NUTS_EXCEPTION_H
#define _NUTS_EXCEPTION_H

#pragma once

#include <QString>

#include <exception>

namespace nuts {
	class Exception : public std::exception {
	public:
		explicit Exception(QString const& msg) : _msg(msg) { }
		~Exception() throw() = default;

		const char* what() const throw() override {
			return _msg.toUtf8().constData();
		}

		const QString& msg() const throw() {
			return _msg;
		}

	private:
		QString _msg;
	};

	class NetlinkInitException : public Exception {
	public:
		explicit NetlinkInitException(QString const& msg) : Exception(msg) { }
	};

	class EthtoolException : public Exception {
	public:
		explicit EthtoolException(QString const& msg) : Exception(msg) { }
	};
	class EthtoolInitException : public EthtoolException {
	public:
		explicit EthtoolInitException(QString const& msg) : EthtoolException(msg) { }
	};
}

#endif /* _NUTS_EXCEPTION_H */
