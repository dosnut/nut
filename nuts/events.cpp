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
		QProcess *process = new QProcess();
		environment << QString("NUT_EVENT=%1").arg(event);
		process->setEnvironment(environment);
		process->start("/etc/nuts/dispatch", args);
		connect(process, SIGNAL(finished( int )), process, SLOT(deleteLater()));
	}
	
	void setupEnvironment(QStringList &environment) {
		environment = QProcess::systemEnvironment();
	}
	
	void setupEnvironment(QStringList &environment, Device *dev) {
		environment
			<< QString("NUT_DEVICE=%1").arg(dev->getName())
			<< QString("NUT_HAS_WLAN=%1").arg(dev->hasWLAN())
			<< QString("NUT_ESSID=%1").arg(dev->essid())
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
			<< QString("NUT_IP=%1").arg(iface->ip.toString())
			<< QString("NUT_NETMASK=%1").arg(iface->netmask.toString())
			<< QString("NUT_LOCALDOMAIN=%1").arg(iface->localdomain)
			<< QString("NUT_STATUS=%1").arg(nut::toString(iface->getState()));
		if (!iface->gateway.isNull())
			environment << QString("NUT_GATEWAY=%1").arg(iface->gateway.toString());
		QStringList dnsservers;
		foreach (QHostAddress dns, iface->dnsserver)
			dnsservers << dns.toString();
		environment << QString("NUT_DNSSERVER=%1").arg(dnsservers.join(","));
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
	void Events::stateChanged(libnut::DeviceState newState, libnut::DeviceState oldState, Device* dev) {
		QStringList e;
		setupEnvironment(e);
		setupEnvironment(e, dev);
		QString nState = nut::toString(newState), oState = nut::toString(oldState);
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
	void Events::interfaceUp(Interface_IPv4* iface) {
		QStringList e;
		Environment *env = iface->getEnvironment();
		Device *dev = env->getDevice();
		setupEnvironment(e);
		setupEnvironment(e, dev);
		setupEnvironment(e, env);
		setupEnvironment(e, iface);
		start(e, "ifup", dev->getName(), env->getName(), iface->getIndex());
	}
	void Events::interfaceDown(Interface_IPv4* iface) {
		QStringList e;
		Environment *env = iface->getEnvironment();
		Device *dev = env->getDevice();
		setupEnvironment(e);
		setupEnvironment(e, dev);
		setupEnvironment(e, env);
		setupEnvironment(e, iface);
		start(e, "ifdown", dev->getName(), env->getName(), iface->getIndex());
	}
}
