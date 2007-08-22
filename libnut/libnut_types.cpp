#include "libnut_types.h"

namespace libnut {
    inline int read_hexdigit(char c) {
        if ('0' <= c && c <= '9')
            return c - '0';
        if ('a' <= c && c <= 'f')
            return c - 'a' + 10;
        if ('A' <= c && c <= 'F')
            return c - 'A' + 10;
        return -1;
    }
    
    inline char* hex2quint8(char* msg, quint8 &val) {
        int i;
        val = 0;
        if (!msg || !*msg) return msg;
        if ((i = read_hexdigit(*msg)) == -1)
            return msg;
        msg++;
        if (!*msg) return msg;
        val = ((quint8) i) << 4;
        if ((i = read_hexdigit(*msg)) == -1)
            return msg;
        msg++;
        val |= i;
        return msg;
    }
    
    libnut_MacAddress::libnut_MacAddress() {
        for (int i = 0; i < 6; i++)
            data[i] = 0;
    }
    libnut_MacAddress::libnut_MacAddress(const QString &str) {
        QByteArray buf = str.toUtf8();
        char *s = buf.data();
        quint8 val;
        char *s2;
        for (int i = 0; i < 6; i++) {
            if (!*s) return;
            if ((s2 = hex2quint8(s, val)) == s) return;
            s = s2;
            if (*s && *s != ':') return;
            data[i] = val;
        }
    }
    
    libnut_MacAddress::libnut_MacAddress(const quint8 *d) {
        if (d == 0) {
            libnut_MacAddress();
        } else {
            for (int i = 0; i < 6; i++)
                data[i] = d[i];
        }
    }
    
    QDBusArgument &operator<< (QDBusArgument &argument, const libnut_SelectConfig & selconf) {
        argument.beginStructure();
        argument << selconf.selected << selconf.flags << selconf.useMAC << selconf.macAddress.toQByteArray << selconf.arpIP << selconf.essid;
        argument.endStructure();
        return argument;
    }
    const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_SelectConfig &selconf) {
        quint32 hostaddress;
        argument.beginStructure();
        argument >> selconf.selected >> selconf.flags >> selconf.useMAC >> selconf.macAddress.toQByteArray >> selconf.arpIP >> selconf.essid;
        argument.endStructure();
        return argument;
    }
    
    QDBusArgument &operator<< (QDBusArgument &argument, const libnut_DeviceProperties & devprop) {
        argument.beginStructure();
        argument << devprop.name << devprop.activeEnvironment << devprop.enabled << devprop.type;
        argument.endStructure();
        return argument;
    }
    const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_DeviceProperties &devprop) {
        argument.beginStructure();
        argument >> devprop.name >> devprop.activeEnvironment >> devprop.enabled >> devprop.type;
        argument.endStructure();
        return argument;
    }
    
    QDBusArgument &operator<< (QDBusArgument &argument, const libnut_wlanScanresult &scanres) {
        argument.beginStructure();
        argument << scanres.essid << scanres.channel << scanres.bssid << scanres.flags << scanres.signallevel << scanres.encryption;
        argument.endStructure();
        return argument;
    }
    const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_wlanScanresult &scanres) {
        argument.beginStructure();
        argument >> scanres.essid >> scanres.channel >> scanres.bssid >> scanres.flags >> scanres.signallevel >> scanres.encryption;
        argument.endStructure();
        return argument;
    }
    
    QDBusArgument &operator<< (QDBusArgument &argument, const libnut_wlanNetworkProperties &wlanprop) {
        argument.beginStructure();
        argument << wlanprop.scanresult << wlanprop.password << wlanprop.proto << wlanprop.key_mgmt;
        argument.endStructure();
        return argument;
    }
    const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_wlanNetworkProperties &wlanprop) {
        argument.beginStructure();
        argument >> wlanprop.scanresult >> wlanprop.password >> wlanprop.proto >> wlanprop.key_mgmt;
        argument.endStructure();
        return argument;
    }
    
    QDBusArgument &operator<< (QDBusArgument &argument, const libnut_EnvironmentProperties &envprop) {
        argument.beginStructure();
        argument << envprop.active << envprop.name;
        argument.endStructure();
        return argument;
    };
    const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_EnvironmentProperties &envprop) {
        quint32 hostaddress;
        argument.beginStructure();
        argument >> envprop.active >> envprop.name;
        return argument;
    }
    QDBusArgument &operator<< (QDBusArgument &argument, const libnut_InterfaceProperties &ifprop) {
        argument.beginStructure();
        argument << ifprop.active << ifprop.userDefineable << ifprop.isStatic;
        argument << ifprop.ip.toIPv4Address() << ifprop.netmask.toIPv4Address() << ifprop.gateway.toIPv4Address();
        argument.endStructure();
        return argument;
    }
    const QDBusArgument &operator>> (const QDBusArgument &argument, libnut_InterfaceProperties &ifprop) {
        quint32 hostaddress;
        argument.beginStructure();
        argument >> ifprop.active >> ifprop.userDefineable >> ifprop.isStatic;
        argument >> hostaddress;
        ifprop.ip = QHostAddress::QHostAddress(hostaddress);
        argument >> hostaddress;
        ifprop.netmask = QHostAddress::QHostAddress(hostaddress);
        argument >> hostaddress;
        ifprop.gateway = QHostAddress::QHostAddress(hostaddress);
        argument.endStructure();
        return argument;
    }
    
    int libnut_metatype_id_storage::metatype_id_libnut_DeviceProperties = -1;
    int libnut_metatype_id_storage::metatype_id_libnut_SelectConfig = -1;
    int libnut_metatype_id_storage::metatype_id_libnut_EnvironmentProperties = -1;
    int libnut_metatype_id_storage::metatype_id_libnut_InterfaceProperties = -1;
    int libnut_metatype_id_storage::metatype_id_libnut_SelectConfigList = -1;
    int libnut_metatype_id_storage::metatype_id_libnut_wlanScanresult = -1;
    int libnut_metatype_id_storage::metatype_id_libnut_wlanScanresultList = -1;
    int libnut_metatype_id_storage::metatype_id_libnut_wlanNetworkProperties = -1;
    
    void libnut_metatype_id_storage::libnut_register_all_metatypes() {
        metatype_id_libnut_DeviceProperties = qRegisterMetaType<libnut::libnut_DeviceProperties>("libnut_DeviceProperties");
        metatype_id_libnut_SelectConfig = qRegisterMetaType<libnut::libnut_SelectConfig>("libnut_SelectConfig");
        
        metatype_id_libnut_EnvironmentProperties = qRegisterMetaType<libnut::libnut_EnvironmentProperties>("libnut_EnvironmentProperties");
        
        metatype_id_libnut_InterfaceProperties = qRegisterMetaType<libnut::libnut_InterfaceProperties>("libnut_InterfaceProperties");
        
        metatype_id_libnut_SelectConfigList = qRegisterMetaType<QList<libnut::libnut_SelectConfig> >("libnut_SelectConfigList");
        
        metatype_id_libnut_wlanScanresult = qRegisterMetaType<libnut::libnut_wlanScanresult>("libnut_wlanScanresult");
        
        metatype_id_libnut_wlanScanresultList = qRegisterMetaType<QList<libnut::libnut_wlanScanresult> >("libnut_wlanScanresultList");
        
        metatype_id_libnut_wlanNetworkProperties = qRegisterMetaType<libnut::libnut_wlanNetworkProperties>("libnut_wlanNetworkProperties");
    }
}

