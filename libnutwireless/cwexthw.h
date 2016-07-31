#ifndef LIBNUTWIRELESS_CWEXTHW_H
#define LIBNUTWIRELESS_CWEXTHW_H

#pragma once

#ifndef NUT_NO_WIRELESS
#include "cwirelesshw.h"
#include <QBasicTimer>

namespace libnutwireless {
	class CWextHW final: public CWirelessHW {
		Q_OBJECT
	public:
		explicit CWextHW(QObject* parent, QString ifname);
		~CWextHW();
		virtual bool open();
		virtual void close();

		virtual void scan();
		virtual QList<ScanResult> getScanResults() const;
		virtual SignalQuality getSignalQuality() const;
		virtual void setSignalQualityPollRate(int msec);
		virtual int getSignalQualityPollRate() const;

		void setScanPollRate(int msec);
		int getScanPollRate() const;
		virtual QList<quint32> getSupportedChannels() const;
		virtual QList<quint32> getSupportedFrequencies() const;

	private:
		/** Raw signal is contains the data from the kernel.
			For human readable format, it has to be converted to SignalQuality */
		struct WextRawSignal {
			quint8 qual{0};    /* link quality (%retries, SNR, %missed beacons or better...) */
			quint8 level{0};   /* signal level (dBm) */
			quint8 noise{0};   /* noise level (dBm) */
			quint8 updated{0}; /* Flags to know if updated */
		};

		/**
			WextRawScan contains the data from the kernel.
			It's converted to WextScan.
		*/
		struct WextRawScan {
			libnutcommon::SSID ssid;
			libnutcommon::MacAddress bssid;
			WextRawSignal quality;
			WextRawSignal maxquality;
			WextRawSignal avgquality;
			bool hasRange{false};
			int freq{-1};
			GroupCiphers group{GCI_UNDEFINED};
			PairwiseCiphers pairwise{PCI_UNDEFINED};
			KeyManagement keyManagement{KM_UNDEFINED};
			Protocols protocols{PROTO_UNDEFINED};
			OPMODE opmode{OPM_AUTO};
			QList<qint32> bitrates;
			//Further information pending...
		};

		void readSignalQuality();
		void readScanResults();
		SignalQuality convertValues(WextRawScan &scan);
		void parseWextIeWpa(unsigned char * iebuf, int buflen, WextRawScan * scan);
		void setScanResults(QList<WextRawScan> wextScanResults);

		void timerEvent(QTimerEvent *event) override;

	private:
		QString m_ifname;
		int m_wextFd{-1};

		QList<quint32> m_supportedFrequencies;

		//Signal Quality Stuff
		qint32 m_sqPollrate{10000};
		qint32 m_sqTimeOutCount{0};
		QBasicTimer m_sqTimer;
		SignalQuality m_sq;

		//Scanning Stuff
		qint32 m_scPollrate{1000};
		qint32 m_scTimeOutCount{10};
		QBasicTimer m_scTimer;
		QList<ScanResult> m_scanResults;
	};
}
#endif
#endif /* LIBNUTWIRELESS_CWEXTHW_H */
