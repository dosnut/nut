//
// C++ Interface: events
//
// Description:
//
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef NUTSEVENTS_H
#define NUTSEVENTS_H

#include <QObject>

#include <libnutcommon/common.h>

namespace nuts {
	class ProcessManager;

	class Device;
	class Environment;
	class Interface_IPv4;
}

namespace nuts {
	/**
		@author Stefan Bühler <stbuehler@web.de>
	*/
	class Events : public QObject {
		Q_OBJECT
		public:
			explicit Events(ProcessManager* processManager);
			virtual ~Events();

		private:
			void start(QProcessEnvironment const& environment, QString const& event, QString const& device, QString const& env = QString(), int iface = -1);

		public:
			void stateChanged(libnutcommon::DeviceState newState, libnutcommon::DeviceState oldState, Device* device);
			void interfaceStatusChanged(libnutcommon::InterfaceState state, Interface_IPv4* iface);

		public slots:
			void deviceAdded(QString devName, Device *dev);
			void deviceRemoved(QString devName, Device *dev);

		private:
			ProcessManager* m_processManager;
	};
}

#include "device.h"

#endif
