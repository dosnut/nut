#include <QString>
#include <QDBusArgument>
#include <QHostAddress>
namespace libnut {

enum libnut_SelectType {user=0, arp=1, essid=2};
struct libnut_SelectConfig {
    libnut_SelectType type;
    bool useMAC;
    QHostAddress arpIP;
    QString essid;
};

QT_DECLARE_METATYPE(libnut_SelectConfig)

QDBusArgument &operator<< (QDBusArgument &argument, const libnut_SelectConfig & selconf);
const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_SelectConfig &selconf);

struct libnut_DeviceProperties {
    QString name;
    bool activeEnvironment;
    bool enabled;
};

QT_DECLARE_METATYPE(libnut_DeviceProperties)

QDBusArgument &operator<< (QDBusArgument &argument, const libnut_DeviceProperties &devprop);
const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_DeviceProperties &devprop);

struct libnut_EnvironmentProperties {
    bool active;
    QString name;
    libnut_SelectConfig currentSelection;
};
QT_DECLARE_METATYPE(libnut_EnvironmentSelection)
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
QT_DECLARE_METATYPE(libnut_InterfaceProperties)
QDBusArgument &operator<< (QDBusArgument &argument, const libnut_InterfaceProperties &ifprop);
const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_InterfaceProperties &ifprop);

}