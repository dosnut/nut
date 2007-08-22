#ifndef libnut_libnut_types
#define libnut_libnut_types

#include <QString>
#include <QDBusArgument>
#include <QHostAddress>
#include <QtDBus>
#include <QMetaType>
//#define NUT_DBUS_URL de_unistg_nut

namespace libnut {
    class libnut_MacAddress {
        public:
            libnut_MacAddress();
            libnut_MacAddress(const QString &str);
            libnut_MacAddress(const quint8 *d);
            quint8 data[6];
            
            inline bool operator==(const libnut_MacAddress &ma) {
                for (int i = 0; i < 6; i++)
                    if (data[i] != ma.data[i])
                        return false;
                return true;
            }
            inline bool operator!=(const libnut_MacAddress &ma) {
                return !(*this == ma);
            }
            inline QString toString() {
                char buf[sizeof("00:00:00:00:00:00")];
                sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", data[0],data[1],data[2],data[3],data[4],data[5]);
                return QString(buf);
            }
            inline QByteArray toQByteArray() {
                return QByteArray((char*) data,6);
            }
    };
    
    enum libnut_SelectFlags {user=0, arp=1, essid=2};
    struct libnut_SelectConfig {
        bool selected;
        int flags;
        bool useMac;
        libnut_MacAddress macAddress;
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
        QDBusObjectPath activeEnvironment;
        bool enabled;
        int type;
    };
    
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
        bool active;
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
    };
};

Q_DECLARE_METATYPE(libnut::libnut_SelectConfig)
Q_DECLARE_METATYPE(QList<libnut::libnut_SelectConfig>)
Q_DECLARE_METATYPE(libnut::libnut_DeviceProperties)
Q_DECLARE_METATYPE(libnut::libnut_wlanScanresult)
Q_DECLARE_METATYPE(QList<libnut::libnut_wlanScanresult>)
Q_DECLARE_METATYPE(libnut::libnut_wlanNetworkProperties)
Q_DECLARE_METATYPE(libnut::libnut_EnvironmentProperties)
Q_DECLARE_METATYPE(libnut::libnut_InterfaceProperties)


#endif
