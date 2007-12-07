/*
	TRANSLATOR qnut::QObject
*/
#ifndef QNUT_COMMON_H
#define QNUT_COMMON_H

#include <QAction>
#include <QString>
#include <QHostAddress>
#include <libnutclient/client.h>
#include "constants.h"

namespace qnut {
	/**
	 * @brief Returns a QString of the filename for the given device according to its state.
	 * @param device pointer to an existing instance of a CDevice
	 */
	QString iconFile(libnutclient::CDevice * device);
	/**
	 * @brief Returns the pointer to the instance of a new action separator.
	 * @param parent parent object
	 */
	QAction * getSeparator(QObject * parent);
	/**
	 * @brief Returns a summary string for the given device: "<name>: <state>, <current ip>".
	 * @param device pointer to an existing instance of a CDevice
	 */
	QString shortSummary(libnutclient::CDevice * device);
	/**
	 * @brief Returns the active ip address for the given device ("(...)" will be added if there is more than one ip address).
	 * @param device pointer to an existing instance of a CDevice
	 */
	QString activeIP(libnutclient::CDevice * device);
	
	/**
	 * @brief Returns the string representation of a valid (not "NULL") QHostAddress. If the given address is not valid "none" will be returned.
	 * @param address the host address to convert
	 */
	inline QString toStringDefault(QHostAddress address) {
		if (address.isNull())
			return QObject::tr("none");
		else
			return address.toString();
	}
	
	/**
	 * @brief Returns a summary for the given signal statistics: "<quality>, <level>dBm, <noise>dBm".
	 * @param signal the signal statistics
	 */
	QString signalSummary(libnutwireless::WextSignal signal);
	
};

#endif
