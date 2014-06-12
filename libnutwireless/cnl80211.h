#ifndef LIBNUTWIRELESS_CNL80211_H
#define LIBNUTWIRELESS_CNL80211_H

#ifndef LIBNUT_NO_WIRELESS
#include "cwirelesshw.h"

#ifndef CONFIG_LIBNL20
#  define nl_sock nl_handle
#endif


struct nl_cb;
struct nl_sock;
struct nl_cache;
struct genl_family;
struct nl_msg;
class QSocketNotifier;

namespace libnutwireless {

class CNL80211: public CWirelessHW {
	Q_OBJECT
	private:
		QString m_ifname;

		//Netlink connection stuff
		nl_cb * m_nlCallback;
		nl_sock * m_nlSocket;
		nl_cache * m_nlCache;
		genl_family * m_nlFamily;
		bool m_connected;
		int m_nlFd;
		QSocketNotifier * m_nlSn;

		QList<quint32> m_supportedFrequencies;

		//Signal Quality Stuff
		qint32 m_sqPollrate;
		qint32 m_sqTimeOutCount;
		qint32 m_sqTimerId;
		SignalQuality m_sq;

		//Scan stuff
		QList<ScanResult> m_scanResults;

	protected:
		virtual void timerEvent(QTimerEvent *event);
		void readSignalQuality() {};
	protected slots:
		void readNlMessage();

	public:
		CNL80211(QObject* parent, QString ifname);
		~CNL80211();
		virtual bool open();
		virtual void close();
		virtual void scan();
		virtual QList<ScanResult> getScanResults();
		virtual SignalQuality getSignalQuality();
		virtual void setSignalQualityPollRate(int msec);
		virtual int getSignalQualityPollRate();
		virtual QList<quint32> getSupportedChannels();
		virtual QList<quint32> getSupportedFrequencies();

		int parseNlScanResult(nl_msg * msg);
};

}
#endif
#endif