#ifndef LIBNUTWIRELESS_CWIRELESS_H
#define LIBNUTWIRELESS_CWIRELESS_H

#ifndef LIBNUT_NO_WIRELESS
#include "wpa_supplicant.h"
#include "cwexthw.h"
#include "cnl80211.h"

namespace libnutwireless {
	class CWireless: public QObject {
		Q_OBJECT
		private:
			QString m_ifname;
			CWpaSupplicant * m_wpa_supplicant;
			CWirelessHW * m_wireless_hw;

		public:
			CWireless(QObject * parent, QString ifname);
			~CWireless();
			void open();
			void close();
			inline CWpaSupplicant * getWpaSupplicant() { return m_wpa_supplicant; }
			inline CWirelessHW * getHardware() { return m_wireless_hw; }
		public slots:
			void scan();
		signals:
			void message(QString);
	};
}
#endif
#endif
