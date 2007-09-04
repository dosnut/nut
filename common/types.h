#ifndef libnut_libnut_types
#define libnut_libnut_types

#include <QString>
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

    enum libnut_SelectFlags {user=0, arp=1, essid=2};
    struct libnut_SelectConfig {
        bool selected;
        int flags;
        bool useMac;
        nut::MacAddress macAddress;
        QHostAddress arpIP;
        QString essid;
    };
    
    QDBusArgument &operator<< (QDBusArgument &argument, const libnut_SelectConfig & selconf);
    const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_SelectConfig &selconf);
    
    
    //Sollte eigtl. von Qt gemacht werden, da libnut_* als Metatyp bekannt.
    //QDBusArgument &operator<< (QDBusArgument &argument, const QList<libnut_InterfaceProperties> &selconflist);
    //const QDBusArgument &operator>> (const QDBusArgument &argument, QList<libnut_InterfaceProperties> &selconflist);
    enum DeviceState  { DS_DEACTIVATED, DS_ACTIVATED, DS_CARRIER, DS_UNCONFIGURED, DS_UP };
    enum libnut_DeviceType {DT_ETH=0, DT_AIR=1, DT_PPP=2};
    struct libnut_DeviceProperties {
        QString name;
        QString activeEnvironment;
        int state;
        int type;
    };
    
    QDBusArgument &operator<< (QDBusArgument &argument, const DeviceState &devstate);
    const QDBusArgument &operator>> (const QDBusArgument &argument, DeviceState &devstate);

    QDBusArgument &operator<< (QDBusArgument &argument, const libnut_DeviceProperties &devprop);
    const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_DeviceProperties &devprop);
    
    enum libnut_wlanEncryptionType {none=0, wep=2, wpa1=4, wpa2=8, other=16};
    struct libnut_wlanScanresult {
        QString essid;
        int channel;
        QByteArray bssid;
        int flags;
        int signallevel;
        int encryption;
    };
    QDBusArgument &operator<< (QDBusArgument &argument, const libnut_wlanEncryptionType &scanres);
    const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_wlanEncryptionType &scanres);
    
    struct libnut_wlanNetworkProperties {
        libnut_wlanScanresult scanresult;
        QString password;
        QString proto;
        QString key_mgmt;
    };
    QDBusArgument &operator<< (QDBusArgument &argument, const libnut_wlanNetworkProperties &wlanprop);
    const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_wlanNetworkProperties &wlanprop);
    
    struct libnut_EnvironmentProperties {
        QString name;
    };
    
    QDBusArgument &operator<< (QDBusArgument &argument, const libnut_EnvironmentProperties &envprop);
    const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_EnvironmentProperties &envprop);
    
    struct libnut_InterfaceProperties {
        bool isStatic;
        bool active;
        bool userDefineable;
        QHostAddress ip;
        QHostAddress netmask;
        QHostAddress gateway;
    };
    
    QDBusArgument &operator<< (QDBusArgument &argument, const libnut_InterfaceProperties &ifprop);
    const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_InterfaceProperties &ifprop);
};

Q_DECLARE_METATYPE(libnut::libnut_SelectConfig)
Q_DECLARE_METATYPE(QList<libnut::libnut_SelectConfig>)
Q_DECLARE_METATYPE(libnut::libnut_DeviceProperties)
Q_DECLARE_METATYPE(libnut::DeviceState)
Q_DECLARE_METATYPE(libnut::libnut_wlanScanresult)
Q_DECLARE_METATYPE(QList<libnut::libnut_wlanScanresult>)
Q_DECLARE_METATYPE(libnut::libnut_wlanNetworkProperties)
Q_DECLARE_METATYPE(libnut::libnut_EnvironmentProperties)
Q_DECLARE_METATYPE(libnut::libnut_InterfaceProperties)

#endif
