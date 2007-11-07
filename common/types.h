#ifndef NUT_COMMON_TYPES_H
#define NUT_COMMON_TYPES_H

#include <QString>
#include <QTranslator>
#include <QDBusArgument>
#include <QHostAddress>
#include <QtDBus>
#include <QMetaType>
#include <QHash>
#include "macaddress.h"

//Functions that need to be defined outside of own namespace:

//This function is needed as QDBusObjectPath does not define any comparison operator (QString is inherited private)
inline bool operator== (const QDBusObjectPath &p1, const QDBusObjectPath &p2){
	return (p1.path() == p2.path());
}
inline uint qHash(const QDBusObjectPath &key) {
	return qHash(key.path());
}


QDBusArgument &operator<< (QDBusArgument &argument, const QHostAddress &data);
const QDBusArgument &operator>> (const QDBusArgument &argument, QHostAddress &data);

namespace common {
	void init();
}

namespace libnut {
	enum DeviceState  { DS_DEACTIVATED, DS_ACTIVATED, DS_CARRIER, DS_UNCONFIGURED, DS_UP };
	enum DeviceType {DT_ETH=0, DT_AIR=1, DT_PPP=2};
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
};

namespace nut {
	QString toString(enum libnut::DeviceState state);
	QString toString(enum libnut::DeviceType type);
	QString toString(enum libnut::InterfaceState state);
}

Q_DECLARE_METATYPE(libnut::DeviceProperties);
Q_DECLARE_METATYPE(libnut::DeviceState);
Q_DECLARE_METATYPE(libnut::EnvironmentProperties);
Q_DECLARE_METATYPE(libnut::InterfaceProperties);
Q_DECLARE_METATYPE(QHostAddress);
Q_DECLARE_METATYPE(QList<QHostAddress>);
#endif
