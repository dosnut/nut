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

#include <QList>
#include <QFile>

namespace nuts {
	Events::Events(QObject *parent)
	: QObject(parent) {
	}

	Events::~Events() {
	}

	void Events::start(QStringList &environment, const QString &event, const QString &device, const QString &env, int iface) {
		QStringList args;
		args << event << device;
		if (!env.isNull()) args << env;
		if (iface != -1) args << QString("%1").arg(iface);
		QProcess *process = new QProcess(this);
		environment << QString("NUT_EVENT=%1").arg(event);
		process->setEnvironment(environment);
		process->start("/etc/nuts/dispatch", args);
		connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), process, &QObject::deleteLater);
		connect(process, static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error), process, &QObject::deleteLater);
	}

	void setupEnvironment(QStringList &environment) {
		environment = QProcess::systemEnvironment();
	}

	void setupEnvironment(QStringList &environment, Device *dev) {
		environment
			<< QString("NUT_DEVICE=%1").arg(dev->getName())
			<< QString("NUT_HAS_WLAN=%1").arg(dev->isAir())
			<< QString("NUT_ESSID=%1").arg(dev->getEssid())
			;
	}

	void setupEnvironment(QStringList &environment, Environment *env) {
		environment
			<< QString("NUT_ENVIRONMENT=%1").arg(env->getName())
			<< QString("NUT_ENVIRONMENT_INDEX=%1").arg(env->getID())
			;
	}

	void setupEnvironment(QStringList &environment, Interface_IPv4* iface) {
		environment
			<< QString("NUT_IP=%1").arg(iface->getIP().toString())
			<< QString("NUT_NETMASK=%1").arg(iface->getNetmask().toString())
			<< QString("NUT_LOCALDOMAIN=%1").arg(iface->getLocalDomain())
			<< QString("NUT_STATUS=%1").arg(libnutcommon::toString(iface->getState()));
		if (!iface->getGateway().isNull())
			environment << QString("NUT_GATEWAY=%1").arg(iface->getGateway().toString());
		QStringList dnsServers;
		foreach (QHostAddress dns, iface->getDnsServers())
			dnsServers << dns.toString();
		environment << QString("NUT_DNSSERVER=%1").arg(dnsServers.join(","));
	}

	void Events::deviceAdded(QString devName, Device *dev) {
		QStringList e;
		setupEnvironment(e);
		setupEnvironment(e, dev);
		start(e, "deviceAdd", devName);
	}
	void Events::deviceRemoved(QString devName, Device *dev) {
		QStringList e;
		setupEnvironment(e);
		setupEnvironment(e, dev);
		start(e, "deviceRemove", devName);
	}
	void Events::stateChanged(libnutcommon::DeviceState newState, libnutcommon::DeviceState oldState, Device* dev) {
		QStringList e;
		setupEnvironment(e);
		setupEnvironment(e, dev);
		QString nState = libnutcommon::toString(newState), oState = libnutcommon::toString(oldState);
		e << QString("NUT_OLD_STATE=%1").arg(oState)
		  << QString("NUT_NEW_STATE=%1").arg(nState)
		  << QString("NUT_STATE=%1").arg(nState);
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
		QStringList e;
		Environment *env = iface->getEnvironment();
		Device *dev = env->getDevice();
		setupEnvironment(e);
		setupEnvironment(e, dev);
		setupEnvironment(e, env);
		setupEnvironment(e, iface);
		start(e, "ifup", dev->getName(), env->getName(), iface->getIndex());
	}
}
