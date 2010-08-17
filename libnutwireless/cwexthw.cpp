#include "cwexthw.h"
#include "conversion.h"
namespace libnutwireless {


CWextHW::CWextHW(QObject* parent, QString ifname) :
CWirelessHW(parent),
m_ifname(ifname),
m_wextFd(-1),
m_sqPollrate(10000),
m_sqTimeOutCount(0),
m_sqTimerId(-1),
m_scPollrate(1000),
m_scTimeOutCount(10),
m_scTimerId(-1) {
}

CWextHW::~CWextHW(){
	close();
}

void CWextHW::timerEvent(QTimerEvent *event) {
	if (m_wextFd != -1) {
		if (event->timerId() == m_sqTimerId){
			readSignalQuality();
		}
		else if (event->timerId() == m_scTimerId) {
			readScanResults();
		}
	}
}

bool CWextHW::open() {
	if (m_wextFd > 0 ) return true;
	//Get file Descriptor to NET kernel
	if ( (m_wextFd = iw_sockets_open()) < 0) {
		m_wextFd = -1;
		qWarning() << tr("ERROR: Could not open socket to net kernel");
		return false;
	}
	else { //Socket is set up, now set SocketNotifier
		qDebug() << QString("File Descriptor for Wext is: %1").arg(QString::number(m_wextFd));
		//Start timer for reading wireless info (like in /proc/net/wireless)
		m_sqTimerId = startTimer(m_sqPollrate);
		return true;
	}
}

void CWextHW::close() {
	if (-1 != m_sqTimerId) {
		killTimer(m_sqTimerId);
		m_sqTimerId = -1;
	}
	if (-1 != m_scTimerId) {
		killTimer(m_scTimerId);
		m_scTimerId = -1;
	}
	if (-1 != m_wextFd) {
		iw_sockets_close(m_wextFd);
		m_wextFd = -1;
	}
}


void CWextHW::scan() {
	if (0 > m_scTimerId) {
		m_scTimeOutCount = 0;
		m_scTimerId = startTimer(m_scPollrate);
	}
}

QList<ScanResult> CWextHW::getScanResults() {
	return m_scanResults;
}

SignalQuality CWextHW::getSignalQuality() {
	return m_sq;
}

void CWextHW::setSignalQualityPollRate(int msec) {
	m_sqTimeOutCount = 0;
	if (m_sqTimerId != -1) {
		killTimer(m_sqTimerId);
	}
	m_sqPollrate = msec;
	m_sqTimerId = startTimer(m_sqPollrate);
}

int CWextHW::getSignalQualityPollRate() {
	return m_sqPollrate;
}

void CWextHW::setScanPollRate(int msec) {
	m_scTimeOutCount = 0;
	if (-1 != m_scTimerId) {
		killTimer(m_scTimerId);
	}
	m_scPollrate = msec;
	m_scTimerId = startTimer(m_scPollrate);
}

int CWextHW::getScanPollRate() {
	return m_scPollrate;
}

QList<quint32> CWextHW::getSupportedChannels() {
	QList<quint32> chans;
	qint32 chan;
	foreach(qint32 freq, m_supportedFrequencies) {
		chan = frequencyToChannel(freq);
		if (chan > 0)
			chans.append(chan);
	}
	return  chans;
}

QList<quint32> CWextHW::getSupportedFrequencies() {
	return m_supportedFrequencies;
}
}
