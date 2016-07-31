#ifndef LIBNUTWIRELESS_CWIRELESSHW_H
#define LIBNUTWIRELESS_CWIRELESSHW_H

#pragma once

#ifndef NUT_NO_WIRELESS
#include "hwtypes.h"

namespace libnutwireless {
	class CWirelessHW: public QObject {
		Q_OBJECT
	protected:
		explicit CWirelessHW(QObject* parent) : QObject(parent) {}

	public:
		virtual bool open() = 0;
		virtual void close() = 0;
		virtual void scan() = 0;
		virtual QList<ScanResult> getScanResults() const = 0;
		virtual SignalQuality getSignalQuality() const = 0;
		virtual void setSignalQualityPollRate(int msec) = 0;
		virtual int getSignalQualityPollRate() const = 0;
		virtual QList<quint32> getSupportedChannels() const = 0;
		virtual QList<quint32> getSupportedFrequencies() const = 0;

	signals:
		void scanCompleted();
		void signalQualityUpdated(libnutwireless::SignalQuality const& sq);
		void message(QString const& msg);
	};
}
#endif
#endif /* LIBNUTWIRELESS_CWIRELESSHW_H */
