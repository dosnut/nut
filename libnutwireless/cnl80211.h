#ifndef LIBNUTWIRELESS_CNL80211_H
#define LIBNUTWIRELESS_CNL80211_H

#ifndef NUT_NO_WIRELESS
#include "cwirelesshw.h"

#include <QBasicTimer>

#include <libnutnetlink/netlink_generic.h>

class QSocketNotifier;

namespace libnutwireless {

	class CNL80211: public CWirelessHW {
	Q_OBJECT
	private:
		QString m_ifname;

		//Netlink connection stuff
		netlink::generic_netlink_sock m_nlSocket;
		bool m_connected = false;
		std::unique_ptr<QSocketNotifier> m_nlSn;

		QList<quint32> m_supportedFrequencies;

		//Signal Quality Stuff
		QBasicTimer m_sqTimer;
		qint32 m_sqPollrate = 10000;
		SignalQuality m_sq;

		//Scan stuff
		bool m_scanRunning{false};
		bool m_scanPending{false};
		bool m_scanFoundCurrentSignal{false};
		QList<ScanResult> m_accumScanResults;
		QList<ScanResult> m_scanResults;

	protected:
		void timerEvent(QTimerEvent* event) override;
		void readSignalQuality() {}

	protected slots:
		void readNlMessage();

	public:
		explicit CNL80211(QObject* parent, QString ifname);
		~CNL80211();

		bool open() override;
		void close() override;
		void scan() override;
		QList<ScanResult> getScanResults() override;
		SignalQuality getSignalQuality() override;
		void setSignalQualityPollRate(int msec) override;
		int getSignalQualityPollRate() override;
		QList<quint32> getSupportedChannels() override;
		QList<quint32> getSupportedFrequencies() override;

	private:
		int parseNlScanResult(nl_msg* msg);
	};

}
#endif /* NUT_NO_WIRELESS */
#endif /* LIBNUTWIRELESS_CNL80211_H */
