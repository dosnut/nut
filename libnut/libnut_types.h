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

struct libnut_DeviceProperties {
    QString name;
    bool activeEnvironment;
    bool enabled;
};

QDBusArgument &operator<< (QDBusArgument &argument, const libnut_DeviceProperties &devprop);
const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_DeviceProperties &devprop);

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

/*
int metatype_id_libnut_DeviceProperties = qRegisterMetaType<libnut::libnut_DeviceProperties>("libnut_DeviceProperties");
int metatype_id_libnut_SelectConfig = qRegisterMetaType<libnut::libnut_SelectConfig>("libnut_SelectConfig");
int metatype_id_libnut_EnvironmentProperties = qRegisterMetaType<libnut::libnut_EnvironmentProperties>("libnut_EnvironmentProperties");
int metatype_id_libnut_InterfaceProperties = qRegisterMetaType<libnut::libnut_InterfaceProperties>("libnut_InterfaceProperties");
*/
}

Q_DECLARE_METATYPE(libnut::libnut_SelectConfig)
Q_DECLARE_METATYPE(libnut::libnut_DeviceProperties)
Q_DECLARE_METATYPE(libnut::libnut_EnvironmentProperties)
Q_DECLARE_METATYPE(libnut::libnut_InterfaceProperties)
Q_DECLARE_METATYPE(QList<libnut::libnut_SelectConfig>)
#endif
