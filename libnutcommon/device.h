#ifndef NUT_COMMON_DEVICE_H
#define NUT_COMMON_DEVICE_H

#include <QString>
#include <QDBusArgument>
#include <QHostAddress>
#include <QtDBus>
#include <QMetaType>
#include <QList>

#include "macaddress.h"

namespace libnutcommon {
	enum DeviceState  { DS_DEACTIVATED, DS_ACTIVATED, DS_CARRIER, DS_UNCONFIGURED, DS_UP };
	enum DeviceType { DT_ETH=0, DT_AIR=1, DT_PPP=2 };
	struct DeviceProperties {
		QString name;
		QString activeEnvironment;
		DeviceState state;
		DeviceType type;
	};
	
	QDBusArgument &operator<< (QDBusArgument &argument, const DeviceProperties &devprop);
	const QDBusArgument &operator>> (const QDBusArgument &argument, DeviceProperties &devprop);
	
	struct EnvironmentProperties {
		QString name;
		bool active;
	};
	
	QDBusArgument &operator<< (QDBusArgument &argument, const EnvironmentProperties &envprop);
	const QDBusArgument &operator>> (const QDBusArgument &argument, EnvironmentProperties &envprop);
	
	enum InterfaceState { IFS_OFF, IFS_STATIC, IFS_DHCP, IFS_ZEROCONF, IFS_WAITFORCONFIG };
	
	struct InterfaceProperties {
		InterfaceState ifState;
		QHostAddress ip;
		QHostAddress netmask;
		QHostAddress gateway;
		QList<QHostAddress> dns;
	};
	
	QDBusArgument &operator<< (QDBusArgument &argument, const InterfaceProperties &ifprop);
	const QDBusArgument &operator>> (const QDBusArgument &argument, InterfaceProperties &ifprop);

	QString toString(enum DeviceState state);
	QString toString(enum DeviceType type);
	QString toString(enum InterfaceState state);
}

Q_DECLARE_METATYPE(libnutcommon::DeviceProperties);
Q_DECLARE_METATYPE(libnutcommon::DeviceState);
Q_DECLARE_METATYPE(libnutcommon::EnvironmentProperties);
Q_DECLARE_METATYPE(libnutcommon::InterfaceProperties);

#endif
