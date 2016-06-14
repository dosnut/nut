#ifndef _NUTS_EVENTS_H
#define _NUTS_EVENTS_H

#pragma once

#include <QObject>

#include <libnutcommon/common.h>

namespace nuts {
	class ProcessManager;

	class Device;
	class Environment;
	class Interface_IPv4;
}

namespace nuts {
	class Events final : public QObject {
		Q_OBJECT
	public:
		explicit Events(ProcessManager* processManager);

	private:
		void start(QProcessEnvironment&& environment, QString const& event, QString const& device, QString const& env = QString(), int iface = -1);

	public:
		void stateChanged(libnutcommon::DeviceState newState, libnutcommon::DeviceState oldState, Device* device);
		void interfaceStatusChanged(libnutcommon::InterfaceState state, Interface_IPv4* iface);

	public slots:
		void deviceAdded(QString devName, Device* dev);
		void deviceRemoved(QString devName, Device* dev);

	private:
		ProcessManager* m_processManager = nullptr;
	};
}

#include "device.h"

#endif /* _NUTS_EVENTS_H */
