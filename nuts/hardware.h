#ifndef _NUTS_HARDWARE_H
#define _NUTS_HARDWARE_H

#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QVector>

#include <libnutcommon/macaddress.h>

extern "C" {
	struct nl_sock;
	struct nl_cache;
	struct ifreq;
}

namespace nuts {
	class HardwareManager final : public QObject {
		Q_OBJECT
	public:
		explicit HardwareManager();
		~HardwareManager();

		bool controlOn(int ifIndex, bool force = false);
		bool controlOff(int ifIndex);
		bool controlOn(QString const& ifName, bool force = false);
		bool controlOff(QString const& ifName);

		/* initial scan for interfaces */
		void discover();

		QString ifIndex2Name(int ifIndex);
		int ifName2Index(QString const& ifName);

		struct ::nl_sock* getNLHandle();

		libnutcommon::MacAddress getMacAddress(QString const& ifName);

		bool hasWLAN(QString const& ifName);
		bool getEssid(QString const& ifName, QString& essid);

	signals:
		void gotCarrier(QString const& ifName, int ifIndex, QString const& essid);
		void lostCarrier(QString const& ifName);

		void newDevice(QString const& ifName, int ifIndex);
		void delDevice(QString const& ifName);

	private slots:
		void read_netlinkmsgs();

	private:
		bool controlOn(int ifIndex, QString const& ifName, bool force);
		bool controlOff(int ifIndex, QString const& ifName);

		bool init_netlink();
		void free_netlink();
		bool init_ethtool();
		void free_ethtool();

		bool ifup(QString const& ifname, bool force = false);
		bool ifdown(QString const& ifname);

		bool isControlled(int ifIndex);

		bool ifreq_init(struct ifreq& ifr, QString const& ifname);
		void ifreq_init(struct ifreq& ifr);

	private: /* vars */
		int netlink_fd{-1};
		int ethtool_fd{-1};
		struct ::nl_sock* nlh{nullptr};
		struct ::nl_cache* nlcache{nullptr};
		struct ifstate {
			explicit ifstate() = default;

			void on();
			void off();

			bool active{false};
			bool carrier{false};
			bool exists{false};
			QString name;
		};
		QVector<struct ifstate> ifStates;
	};
}

#endif /* _NUTS_HARDWARE_H */
