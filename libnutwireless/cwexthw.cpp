#include "cwexthw.h"
#include "conversion.h"

#include <QTimerEvent>

#include <iwlib.h>

namespace libnutwireless {
	CWextHW::CWextHW(QObject* parent, QString ifname)
	: CWirelessHW(parent)
	, m_ifname(ifname) {
	}

	CWextHW::~CWextHW() {
		close();
	}

	void CWextHW::timerEvent(QTimerEvent *event) {
		if (m_wextFd != -1) {
			if (event->timerId() == m_sqTimer.timerId()) {
				readSignalQuality();
			}
			else if (event->timerId() == m_scTimer.timerId()) {
				readScanResults();
			}
		}
	}

	bool CWextHW::open() {
		if (m_wextFd > 0 ) return true;
		//Get file Descriptor to NET kernel
		if ( (m_wextFd = iw_sockets_open()) < 0) {
			m_wextFd = -1;
			qWarning("%s", qPrintable(tr("ERROR: Could not open socket to net kernel")));
			return false;
		}
		else { //Socket is set up, now set SocketNotifier
			qDebug("File Descriptor for Wext is: %i", m_wextFd);
			//Start timer for reading wireless info (like in /proc/net/wireless)
			if (m_wextFd != -1) readSignalQuality();
			m_sqTimer.start(m_sqPollrate, this);
			return true;
		}
	}

	void CWextHW::close() {
		m_sqTimer.stop();
		m_scTimer.stop();
		if (-1 != m_wextFd) {
			iw_sockets_close(m_wextFd);
			m_wextFd = -1;
		}
	}


	void CWextHW::scan() {
		if (!m_scTimer.isActive()) {
			m_scTimeOutCount = 0;
			m_scTimer.start(m_scPollrate, this);
		}
	}

	QList<ScanResult> CWextHW::getScanResults() const {
		return m_scanResults;
	}

	SignalQuality CWextHW::getSignalQuality() const {
		return m_sq;
	}

	void CWextHW::setSignalQualityPollRate(int msec) {
		m_sqTimeOutCount = 0;
		m_sqTimer.stop();
		m_sqPollrate = msec;
		m_sqTimer.start(m_sqPollrate, this);
	}

	int CWextHW::getSignalQualityPollRate() const {
		return m_sqPollrate;
	}

	void CWextHW::setScanPollRate(int msec) {
		m_scTimeOutCount = 0;
		m_scTimer.stop();
		m_scPollrate = msec;
		m_scTimer.start(m_scPollrate, this);
	}

	int CWextHW::getScanPollRate() const {
		return m_scPollrate;
	}

	QList<quint32> CWextHW::getSupportedChannels() const {
		QList<quint32> chans;
		for (qint32 freq: m_supportedFrequencies) {
			qint32 chan = frequencyToChannel(freq);
			if (chan > 0) chans.append(chan);
		}
		return  chans;
	}

	QList<quint32> CWextHW::getSupportedFrequencies() const {
		return m_supportedFrequencies;
	}
}
