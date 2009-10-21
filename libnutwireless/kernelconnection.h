#ifndef LIBNUTWIRELESS_kernelconnection_H
#define LIBNUTWIRELESS_kernelconnection_H
#ifndef LIBNUT_NO_WIRELESS

#include <QObject>
#include <QString>
#include "types.h"


namespace libnutwireless {
	CKernelConnection : public QObject {
		Q_OBJECT
		friend class CWpaSupplicantBase;
		private:
			QString m_deviceName;
			int m_signalQualityPollRate;
			bool m_has80211cfg;
			//Timers
			int m_signalQualityTimerId;
			int m_signalQualityPollRate;
			int m_signalQualityTimeoutCount;
			
			//File descriptors
			int m_wext;
			
			//parsed Results
			QList<ScanResult> m_scanResults;
			WextSignal m_signalQuality;
			
			//Wpa state
			bool m_wpaConnected;
			
			//Wireless extension specific
			void parseWextIeWpa(unsigned char * iebuf, int buflen, WextRawScan * scan);
		public:
			CKernelConnection(QObject * parent, QString * deviceName);
			~CKernelConnection();
			bool connectToKernel();
			bool disconnectFromKernel();
			
			void setSignalQualityPollRate(int msec);
			int getSignalQualityPollRate();
			
			WextSignal getSignalQuality();
			QList<ScanResult> getScanResults();
			
			
		public slots:
			void scan();
			
		signals:
			void scanCompleted();
			void signalQualityUpdated(libnutwireless::WextSignal signal);
		
};
#endif
#endif