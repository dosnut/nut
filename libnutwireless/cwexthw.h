#ifndef LIBNUTWIRELESS_CWEXTHW_H
#define LIBNUTWIRELESS_CWEXTHW_H

#ifndef LIBNUT_NO_WIRELESS
#include "cwirelesshw.h"
#include <QTimerEvent>



namespace libnutwireless {

class CWextHW: public CWirelessHW {
	Q_OBJECT
	protected:
		QString m_ifname;
		int m_wextFd;

		QList<quint32> m_supportedFrequencies;

		//Signal Quality Stuff
		qint32 m_sqPollrate;
		qint32 m_sqTimeOutCount;
		qint32 m_sqTimerId;
		SignalQuality m_sq;

		//Scanning Stuff
		qint32 m_scPollrate;
		qint32 m_scTimeOutCount;
		qint32 m_scTimerId;
		QList<ScanResult> m_scanResults;


		/** Raw signal is contains the data from the kernel.
			For human readable format, it has to be converted to SignalQuality */
		struct	WextRawSignal {
			quint8 qual;	/* link quality (%retries, SNR, %missed beacons or better...) */
			quint8 level;		/* signal level (dBm) */
			quint8 noise;		/* noise level (dBm) */
			quint8 updated;	/* Flags to know if updated */
		};

		/**
			WextRawScan contains the data from the kernel.
			It's converted to WextScan.
		*/
		struct WextRawScan {
			QString ssid;
			libnutcommon::MacAddress bssid;
			WextRawSignal quality;
			WextRawSignal maxquality;
			WextRawSignal avgquality;
			int hasRange;
			int we_version_compiled;
			int freq;
			GroupCiphers group;
			PairwiseCiphers pairwise;
			KeyManagement keyManagement;
			Protocols protocols;
			OPMODE opmode;
			QList<qint32> bitrates;
			//Further information pending...
		};

	protected:

		void readSignalQuality();
		void readScanResults();
		SignalQuality convertValues(WextRawScan &scan);
		void parseWextIeWpa(unsigned char * iebuf, int buflen, WextRawScan * scan);
		void setScanResults(QList<WextRawScan> wextScanResults);

		virtual void timerEvent(QTimerEvent *event);

	public:
		CWextHW(QObject* parent, QString ifname);
		~CWextHW();
		virtual bool open();
		virtual void close();

		virtual void scan();
		virtual QList<ScanResult> getScanResults();
		virtual SignalQuality getSignalQuality();
		virtual void setSignalQualityPollRate(int msec);
		virtual int getSignalQualityPollRate();
		void setScanPollRate(int msec);
		int getScanPollRate();
		virtual QList<quint32> getSupportedChannels();
		virtual QList<quint32> getSupportedFrequencies();
};

}
#endif
#endif
