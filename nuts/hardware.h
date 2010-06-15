
#ifndef _NUTS_HARDWARE_H
#define _NUTS_HARDWARE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVector>

#include <libnutcommon/macaddress.h>

extern "C" {
	struct nl_handle;
	struct nl_cache;
	struct ifreq;
}

namespace nuts {
	class HardwareManager;
}

namespace nuts {
	class HardwareManager : public QObject {
		Q_OBJECT
		private:
			int netlink_fd, ethtool_fd;
			struct nl_handle *nlh;
			struct nl_cache *nlcache;
			struct ifstate {
				bool active, carrier, exists;
				ifstate() : active(false), carrier(false), exists(false) { }
				ifstate(bool active) : active(active), carrier(false), exists(true) { }
			};
			QVector<struct ifstate> ifStates;
			
			bool init_netlink();
			void free_netlink(); 
			bool init_ethtool();
			void free_ethtool();
			
			bool ifup(const QString &ifname, bool force = false);
			bool ifdown(const QString &ifname);
			
			bool isControlled(int ifIndex);
			
			bool ifreq_init(struct ifreq &ifr, const QString &ifname);
			void ifreq_init(struct ifreq &ifr);
			
		private slots:
			void read_netlinkmsgs();
		public:
			HardwareManager();
			virtual ~HardwareManager();
			
			bool controlOn(int ifIndex, bool force = false);
			bool controlOff(int ifIndex);
			bool controlOn(const QString &ifName, bool force = false);
			bool controlOff(const QString &ifName);
			
			QString ifIndex2Name(int ifIndex);
			QList<QString> get_ifNames();
			int ifName2Index(const QString &ifName);
			
			struct nl_handle *getNLHandle();
			
			libnutcommon::MacAddress getMacAddress(const QString &ifName);
			
			bool ifExists(const QString &ifName);
			bool hasWLAN(const QString &ifName);
			bool getEssid(const QString &ifName, QString &essid);
			
		signals:
			void gotCarrier(const QString &ifName, int ifIndex, const QString &essid);
			void lostCarrier(const QString &ifName);
			
			void newDevice(const QString &ifName, int ifIndex);
			void delDevice(const QString &ifName);
	};
}

#endif
