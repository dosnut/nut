#ifndef LIBNUT_EXCEPTIONS_H
#define LIBNUT_EXCEPTIONS_H

#include <exception>
#include <QString>

namespace libnut {
//Copied from ../nuts/exception.h
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
    class CLI_ConnectionInitException : public Exception {
        public:
            CLI_ConnectionInitException(const QString &msg) : Exception(msg) {}
    };
    class CLI_ConnectionException : public Exception {
        public:
            CLI_ConnectionException(const QString &msg) : Exception(msg) {}
    };
    class CLI_DevConnectionException : public CLI_ConnectionException {
        public:
            CLI_DevConnectionException(const QString &msg) : CLI_ConnectionException(msg) {}
    };
    class CLI_EnvConnectionException : public CLI_ConnectionException {
        public:
            CLI_EnvConnectionException(const QString &msg) : CLI_ConnectionException(msg) {}
    };

}
#endif
