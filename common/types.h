#ifndef libnut_libnut_types
#define libnut_libnut_types

#include <QString>
#include <QTranslator>
#include <QDBusArgument>
#include <QHostAddress>
#include <QtDBus>
#include <QMetaType>

#include "macaddress.h"


//Functions that need to be defined outside of own namespace:

//This function is needed as QDBusObjectPath does not define any comparison operator (QString is inherited private)
inline bool operator== (const QDBusObjectPath &p1, const QDBusObjectPath &p2){
	return (p1.path() == p2.path());
}
inline uint qHash(const QDBusObjectPath &key) {
	return qHash(key.path());
}

namespace common {
	void init();
}

namespace libnut {

	enum SelectFlags {SF_USER=0, SF_ARP=1, SF_ESSID=2};
	struct SelectConfig {
		bool selected;
		int flags;
		bool useMac;
		nut::MacAddress macAddress;
		QHostAddress arpIP;
		QString essid;
	};
	
	QDBusArgument &operator<< (QDBusArgument &argument, const SelectConfig & selconf);
	const QDBusArgument &operator>> (const QDBusArgument &argument, SelectConfig &selconf);
	
	
	//Sollte eigtl. von Qt gemacht werden, da libnut_* als Metatyp bekannt.
	//QDBusArgument &operator<< (QDBusArgument &argument, const QList<InterfaceProperties> &selconflist);
	//const QDBusArgument &operator>> (const QDBusArgument &argument, QList<InterfaceProperties> &selconflist);
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
	
	enum WlanEncryptionType {none=0, wep=2, wpa1=4, wpa2=8, other=16};
	struct WlanScanresult {
		QString essid;
		int channel;
		QByteArray bssid;
		int flags;
		int signallevel;
		WlanEncryptionType encryption;
	};


	QDBusArgument &operator<< (QDBusArgument &argument, const WlanScanresult &scanres);
	const QDBusArgument &operator>> (const QDBusArgument &argument, WlanScanresult &scanres);
	
	struct WlanNetworkProperties {
		WlanScanresult scanresult;
		QString password;
		QString proto;
		QString key_mgmt;
	};
	QDBusArgument &operator<< (QDBusArgument &argument, const WlanNetworkProperties &wlanprop);
	const QDBusArgument &operator>> (const QDBusArgument &argument, WlanNetworkProperties &wlanprop);
	
	struct EnvironmentProperties {
		QString name;
	};
	
	QDBusArgument &operator<< (QDBusArgument &argument, const EnvironmentProperties &envprop);
	const QDBusArgument &operator>> (const QDBusArgument &argument, EnvironmentProperties &envprop);
	
	struct InterfaceProperties {
		bool isStatic;
		bool active;
		bool userDefineable;
		QHostAddress ip;
		QHostAddress netmask;
		QHostAddress gateway;
	};
	
	QDBusArgument &operator<< (QDBusArgument &argument, const InterfaceProperties &ifprop);
	const QDBusArgument &operator>> (const QDBusArgument &argument, InterfaceProperties &ifprop);
};

Q_DECLARE_METATYPE(libnut::SelectConfig)
Q_DECLARE_METATYPE(QList<libnut::SelectConfig>)
Q_DECLARE_METATYPE(libnut::DeviceProperties)
Q_DECLARE_METATYPE(libnut::DeviceState)
Q_DECLARE_METATYPE(libnut::WlanScanresult)
Q_DECLARE_METATYPE(QList<libnut::WlanScanresult>)
Q_DECLARE_METATYPE(libnut::WlanNetworkProperties)
Q_DECLARE_METATYPE(libnut::EnvironmentProperties)
Q_DECLARE_METATYPE(libnut::InterfaceProperties)

#endif
