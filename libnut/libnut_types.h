#include <QString>
#include <QDBusArgument>
#include <QHostAddress>
#include <QtDBus>
#include <QMetaType>

namespace libnut {

enum libnut_SelectType {user=0, arp=1, essid=2};
struct libnut_SelectConfig {
    libnut_SelectType type;
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


}
