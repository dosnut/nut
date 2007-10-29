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

#include <common/common.h>

namespace nuts {
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
			Events(QObject *parent = 0);
			virtual ~Events();
		
		private:
			void start(QStringList &environment, const QString &event, const QString &device, const QString &env = QString(), int iface=-1);
		
		public slots:
			void deviceAdded(QString devName, Device *dev);
			void deviceRemoved(QString devName, Device *dev);
			void stateChanged(libnut::DeviceState newState, libnut::DeviceState oldState, Device* device);
			void interfaceUp(Interface_IPv4* iface);
			void interfaceDown(Interface_IPv4* iface);
	};
}

#include "device.h"

#endif
