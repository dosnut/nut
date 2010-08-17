#ifndef LIBNUTWIRELESS_CWIRELESSHW_H
#define LIBNUTWIRELESS_CWIRELESSHW_H

#ifndef LIBNUT_NO_WIRELESS
#include "cnetworkconfig.h"
#include "hwtypes.h"

namespace libnutwireless {
	class CWirelessHW: public QObject {
		Q_OBJECT
		public:
			CWirelessHW(QObject* parent) : QObject(parent) {};
			~CWirelessHW() {};
			virtual bool open()=0;
			virtual void close()=0;
			virtual void scan()=0;
			virtual QList<ScanResult> getScanResults()=0;
			virtual SignalQuality getSignalQuality()=0;
			virtual void setSignalQualityPollRate(int msec)=0;
			virtual int getSignalQualityPollRate()=0;
			virtual QList<quint32> getSupportedChannels()=0;
			virtual QList<quint32> getSupportedFrequencies()=0;
		signals:
			void scanCompleted();
			void signalQualityUpdated(libnutwireless::SignalQuality sq);
			void message(QString msg);
	};
}
#endif
#endif
