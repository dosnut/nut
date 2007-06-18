
#ifndef _NUTS_HARDWARE_H
#define _NUTS_HARDWARE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QBitArray>

extern "C" {
	struct nl_handle;
};

namespace nuts {
	class HardwareManager;
};

namespace nuts {
	class HardwareManager : public QObject {
		Q_OBJECT
		private:
			int netlink_fd, ethtool_fd;
			struct nl_handle *nlh;
			QBitArray ifMonitor;
			
			bool init_netlink();
			void free_netlink(); 
			bool init_ethtool();
			void free_ethtool();
			
			bool ifup(const QString &ifname);
			bool ifdown(const QString &ifname);
			
			QString ifIndex2Name(int ifIndex);
			int ifName2Index(const QString &ifName);
			
			bool isMonitored(int ifIndex);
			
		private slots:
			void read_netlinkmsgs();
		public:
			HardwareManager();
			virtual ~HardwareManager();
			
			void setMonitor(int ifIndex);
			void clearMonitor(int ifIndex);
			
		signals:
			void gotCarrier(int ifIndex);
			void lostCarrier(int ifIndex);
	};
};

#endif
