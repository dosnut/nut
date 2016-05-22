//
// C++ Implementation: events
//
// Description:
//
//
// Author: Stefan Bühler <stbuehler@web.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "events.h"

#include "processmanager.h"

#include <QList>
#include <QFile>

namespace nuts {
	Events::Events(ProcessManager* processManager)
	: m_processManager(processManager) {
	}

	Events::~Events() {
	}

	void Events::start(QProcessEnvironment const& environment, QString const& event, QString const& device, QString const& env, int iface) {
		QStringList args;
		args << event << device;
		if (!env.isNull()) args << env;
		if (iface != -1) args << QString("%1").arg(iface);

		m_processManager->startProgram(environment, "/etc/nuts/dispatch", args);
	}

	namespace {
		void setupEnvironment(QProcessEnvironment &environment) {
			environment = QProcessEnvironment::systemEnvironment();
		}

		void setupEnvironment(QProcessEnvironment &environment, Device *dev) {
			environment.insert("NUT_DEVICE", dev->getName());
			environment.insert("NUT_HAS_WLAN", QString::number(dev->isAir()));
			environment.insert("NUT_ESSID", dev->getEssid());
		}

		void setupEnvironment(QProcessEnvironment &environment, Environment *env) {
			environment.insert("NUT_ENVIRONMENT",env->getName());
			environment.insert("NUT_ENVIRONMENT_INDEX", QString::number(env->getID()));
		}

		void setupEnvironment(QProcessEnvironment &environment, Interface_IPv4* iface) {
			environment.insert("NUT_IP", iface->getIP().toString());
			environment.insert("NUT_NETMASK", iface->getNetmask().toString());
			environment.insert("NUT_LOCALDOMAIN", iface->getLocalDomain());
			environment.insert("NUT_STATUS", libnutcommon::toString(iface->getState()));
			if (!iface->getGateway().isNull())
				environment.insert("NUT_GATEWAY", iface->getGateway().toString());
			QStringList dnsServers;
			foreach (QHostAddress dns, iface->getDnsServers())
				dnsServers << dns.toString();
			environment.insert("NUT_DNSSERVER", dnsServers.join(","));
		}
	}

	void Events::deviceAdded(QString devName, Device *dev) {
		QProcessEnvironment e;
		setupEnvironment(e);
		setupEnvironment(e, dev);
		start(e, "deviceAdd", devName);
	}
	void Events::deviceRemoved(QString devName, Device *dev) {
		QProcessEnvironment e;
		setupEnvironment(e);
		setupEnvironment(e, dev);
		start(e, "deviceRemove", devName);
	}
	void Events::stateChanged(libnutcommon::DeviceState newState, libnutcommon::DeviceState oldState, Device* dev) {
		QProcessEnvironment e;
		setupEnvironment(e);
		setupEnvironment(e, dev);
		QString nState = libnutcommon::toString(newState), oState = libnutcommon::toString(oldState);
		e.insert("NUT_OLD_STATE", oState);
		e.insert("NUT_NEW_STATE", nState);
		e.insert("NUT_STATE", nState);
		QString envName;
		int e_idx = dev->getEnvironment();
		if (e_idx >= 0) {
			Environment *env = dev->getEnvironments()[e_idx];
			setupEnvironment(e, env);
			envName = env->getName();
		}
		start(e, nState, dev->getName(), envName);
	}
	void Events::interfaceStatusChanged(libnutcommon::InterfaceState state, Interface_IPv4* iface) {
		QString event;
		switch (state) {
			case libnutcommon::InterfaceState::OFF:
				event = "ifdown";
				break;
			case libnutcommon::InterfaceState::STATIC:
			case libnutcommon::InterfaceState::DHCP:
			case libnutcommon::InterfaceState::ZEROCONF:
				event = "ifup";
				break;
			case libnutcommon::InterfaceState::WAITFORCONFIG:
				event = "ifwaitforconfig";
				break;
			default:
				return;
		}
		QProcessEnvironment e;
		Environment *env = iface->getEnvironment();
		Device *dev = env->getDevice();
		setupEnvironment(e);
		setupEnvironment(e, dev);
		setupEnvironment(e, env);
		setupEnvironment(e, iface);
		start(e, "ifup", dev->getName(), env->getName(), iface->getIndex());
	}
}
