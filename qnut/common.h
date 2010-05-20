/*
	TRANSLATOR qnut::QObject
*/
//
// C++ Interface: common
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_COMMON_H
#define QNUT_COMMON_H

#include <QAction>
#include <QString>
#include <QHostAddress>

#include <libnutclient/cenvironment.h>

namespace libnutclient {
	class CDevice;
}

namespace qnut {
	/// @brief Simple structure for a command string and it's enabled state
	struct ToggleableCommand {
		bool enabled;
		QString path;
		
		ToggleableCommand() : enabled(false) {}
	};
	
	/**
	 * @brief Returns a QString of the filename for the given device according to its state.
	 * @param device pointer to an existing instance of a CDevice
	 */
	QString iconFile(libnutclient::CDevice * device, bool stateAware = true);
	/**
	 * @brief Returns the pointer to the instance of a new action separator.
	 * @param parent parent object
	 */
	QAction * getSeparator(QObject * parent);
	/**
	 * @brief Returns a short summary string for the given device: "<name>: <state>, <current ip>".
	 * @param device pointer to an existing instance of a CDevice
	 */
	QString shortSummary(libnutclient::CDevice * device);
	/**
	 * @brief Returns a summary string of device details for the given device: "Type: <type>\nState: <state> (<current ip>)\nConnected to: <network>".
	 * @param device pointer to an existing instance of a CDevice
	 */
	QString detailsSummary(libnutclient::CDevice * device);
	/**
	 * @brief Returns the current network name the given device is connected to.
	 * @param device pointer to an existing instance of a CDevice
	 */
	QString currentNetwork(libnutclient::CDevice * device, bool appendQuality = false);
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
		return address.isNull() ? QObject::tr("none") : address.toString();
	}
	
	inline QString getNameDefault(libnutclient::CEnvironment * environment) {
		return environment->getName().isEmpty() ? QObject::tr("default") : environment->getName();
	}
}

#endif
