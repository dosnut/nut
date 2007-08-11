#ifndef libnut_libnut_types
#define libnut_libnut_types

#include <QString>
#include <QDBusArgument>
#include <QHostAddress>
#include <QtDBus>
#include <QMetaType>
//#define NUT_DBUS_URL de_unistg_nut

namespace libnut {

enum libnut_SelectType {user=0, arp=1, essid=2};
struct libnut_SelectConfig {
    int type;
    bool useMAC;
    QHostAddress arpIP;
    QString essid;
};

QDBusArgument &operator<< (QDBusArgument &argument, const libnut_SelectConfig & selconf);
const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_SelectConfig &selconf);


//Sollte eigtl. von Qt gemacht werden, da libnut_* als Metatyp bekannt.
//QDBusArgument &operator<< (QDBusArgument &argument, const QList<libnut_InterfaceProperties> &selconflist);
//const QDBusArgument &operator>> (const QDBusArgument &argument, QList<libnut_InterfaceProperties> &selconflist);

enum libnut_DeviceType {ethernet=0, wlan=1, ppp=2};
struct libnut_DeviceProperties {
    QString name;
    bool activeEnvironment;
    bool enabled;
    int type;
};

QDBusArgument &operator<< (QDBusArgument &argument, const libnut_DeviceProperties &devprop);
const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_DeviceProperties &devprop);

enum libnut_wlanEncryptionType {none=0, wep=1, wpa1=2, wpa2=3, other=4};
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
    bool active;
    QString name;
    libnut_SelectConfig currentSelection;
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

//Registerd Ids


/*
int metatype_id_libnut_DeviceProperties = qRegisterMetaType<libnut::libnut_DeviceProperties>("libnut_DeviceProperties");
int metatype_id_libnut_SelectConfig = qRegisterMetaType<libnut::libnut_SelectConfig>("libnut_SelectConfig");
int metatype_id_libnut_EnvironmentProperties = qRegisterMetaType<libnut::libnut_EnvironmentProperties>("libnut_EnvironmentProperties");
int metatype_id_libnut_InterfaceProperties = qRegisterMetaType<libnut::libnut_InterfaceProperties>("libnut_InterfaceProperties");
*/
class libnut_metatype_id_storage {
public:
    static int metatype_id_libnut_DeviceProperties;
    static int metatype_id_libnut_SelectConfig;
    static int metatype_id_libnut_EnvironmentProperties;
    static int metatype_id_libnut_InterfaceProperties;
    static int metatype_id_libnut_SelectConfigList;
    static int metatype_id_libnut_wlanScanresult;
    static int metatype_id_libnut_wlanScanresultList;
    static int metatype_id_libnut_wlanNetworkProperties;
    static void libnut_register_all_metatypes();
    libnut_metatype_id_storage();
};
}

Q_DECLARE_METATYPE(libnut::libnut_SelectConfig)
Q_DECLARE_METATYPE(QList<libnut::libnut_SelectConfig>)
Q_DECLARE_METATYPE(libnut::libnut_DeviceProperties)
Q_DECLARE_METATYPE(libnut::libnut_wlanScanresult)
Q_DECLARE_METATYPE(QList<libnut::libnut_wlanScanresult>)
Q_DECLARE_METATYPE(libnut::libnut_wlanNetworkProperties)
Q_DECLARE_METATYPE(libnut::libnut_EnvironmentProperties)
Q_DECLARE_METATYPE(libnut::libnut_InterfaceProperties)


#endif
