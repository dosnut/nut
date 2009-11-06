#ifndef LIBNUTCLIENT_CLIBNUT_H
#define LIBNUTCLIENT_CLIBNUT_H

#include <QObject>

#include "libnutcommon/common.h"

namespace libnutclient {

	QString toStringTr(libnutcommon::DeviceState state);
	QString toStringTr(libnutcommon::DeviceType type);
	QString toStringTr(QDBusError error);
	QString toStringTr(libnutcommon::InterfaceState state);
	QString toString(QDBusError error);

	/** @brief CLibNut is the base class for all libnutclient classes

		The class provides very basic functions and members that all derived classes have in common
	*/
	class CLibNut : public QObject {
		Q_OBJECT
		protected:
			QDBusConnectionInterface * m_dbusConnectionInterface;
			QDBusConnection * m_dbusConnection;
			/** Function to check if nuts is running */
			bool serviceCheck();
			inline bool dbusConnected() { return m_dbusConnection->isConnected(); }
		public:
			CLibNut(QObject * parent) : QObject(parent), m_dbusConnectionInterface(0), m_dbusConnection(0) {}

		signals:
			void dbusErrorOccured(QDBusError error = QDBusError());
	};

}

#endif
