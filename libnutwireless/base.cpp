#include "base.h"
#include <QDebug>

namespace libnutwireless {
	
	//CWpa_supplicant
	
	
	//Wpa_supplicant control commands:
	QString CWpaSupplicantBase::wpaCtrlCommand(QString cmd = "PING") {
		//int wpa_ctrl_request(struct wpa_ctrl *ctrl, const char *cmd, size_t cmd_len,
		//	     char *reply, size_t *reply_len,
		//	     void (*msg_cb)(char *msg, size_t len));
		//	     
		//Check if we have a control interface:
		if (cmd_ctrl == NULL || event_ctrl == NULL) {
			return QString();
		}
		if (!m_wpaConnected or m_inConnectionPhase) {
			return QString();
		}
		//First Check if wpa_supplicant is running:
		QByteArray command("PING");
		char reply[4096];
		size_t reply_len = sizeof(reply);
		
		int status = wpa_ctrl_request(cmd_ctrl, command.constData(), command.size(), reply, &reply_len,NULL);
		if ( (status != 0) or (QString::fromUtf8(reply, reply_len) != "PONG\n") ) {
			qDebug() << QString("(status=%2)PING COMMAND RESPONSE: %1").arg(QString::fromUtf8(reply, reply_len),QString::number(status));
			closeWpa("wpaCtrlCommand/nopong");
			return QString();
		}
		if (cmd != "PING") {
			size_t reply_len = sizeof(reply);

			command = cmd.toAscii();
			
			status = wpa_ctrl_request(cmd_ctrl, command.constData(), command.size(), reply, &reply_len,NULL);
			if (0 == status) {
				qDebug() << cmd + " : " + QString::fromUtf8(reply, reply_len) + " EOC";
				if (reply_len > 0) {
					return QString::fromUtf8(reply, reply_len);
				}
				else {
					return QString();
				}
			}
			else {
				return QString();
			}
		}
		else { //PING command requested
			qDebug() << cmd << ":" << QString::fromUtf8(reply, reply_len) << "EOC";
			return QString::fromUtf8(reply, reply_len);
		}
	}
	
	
	
	
	
	
// 	void CWpaSupplicantBase::printMessage(QString msg) {
// 		if (m_logEnabled) {
// 			emit(message(msg));
// 		}
// 	}
	
	
	//Private slots:
	//Reads messages from wpa_supplicant
	void CWpaSupplicantBase::readFromWpa(int socket) {
		if (socket == m_wpaFd) {
			if (m_wpaConnected) {
				//check if wpa_supplicant is still running (i.e. file exists)
				if (! QFile::exists(m_wpaSupplicantPath)) {
					closeWpa("readFromWpa/no wpa interface on event");
					return;
				}
				//status: 1 = msg available; 0 = no messages, -1 = error
				int status = wpa_ctrl_pending(event_ctrl);
				if (1 == status) {
					//Receive Messages
					char reply[512];
					size_t reply_len = sizeof(reply);
					wpa_ctrl_recv(event_ctrl, reply, &reply_len);
					eventDispatcher(QString::fromUtf8(reply, reply_len));
				}
				else if (-1 == status) {
					qWarning() << tr("Error while trying to receive messages from wpa_supplicant");
				}
			}
		}
	}
	
	
	/*
	//CTRL-RSP-<field name>-<network id>-<value>
	*/
	/*
	CTRL-EVENT-DISCONNECTED
	CTRL-EVENT-CONNECTED
	*/
	void CWpaSupplicantBase::eventDispatcher(Request req) {
		if (req.type != REQ_FAIL) {
			emit(request(req));
		}
	}
	void CWpaSupplicantBase::eventDispatcher(EventType event, QString str) {
		if (event == EVENT_CONNECTED) {
			emit(connectionStateChanged(true,parseEventNetworkId(str)));
			emit(networkListUpdated());
		}
		else if (event == EVENT_DISCONNECTED) {
			emit(connectionStateChanged(false,parseEventNetworkId(str)));
		}
		else if (event == EVENT_TERMINATING) {
			closeWpa("event-dispatcher/wpa-TERMINATING");
		}
		else {
			emit(eventMessage(event));
		}
	}
	
	void CWpaSupplicantBase::eventDispatcher(QString event) {
		QStringList str_list = event.split('\n',QString::KeepEmptyParts);
		InteractiveType type;
		foreach(QString str, str_list) {
			type = parseInteract(str);
			switch (type) {
				case (INTERACT_REQ):
					eventDispatcher(parseReq(str));
					break;
				case (INTERACT_EVENT):
					eventDispatcher(parseEvent(str),str);
					break;
				default:
					printMessage(str);
					break;
			}
		}
	}
	
	//Public functions:
	CWpaSupplicantBase::CWpaSupplicantBase(QObject * parent, QString m_ifname) : QObject(parent), cmd_ctrl(0), event_ctrl(0), m_wpaSupplicantPath("/var/run/wpa_supplicant/"+m_ifname), m_wpaFd(-1), m_wextFd(-1), m_eventSn(0), m_connectTimerId(-1), m_wextTimerId(-1), m_scanTimerId(-1), m_wextTimerRate(10000), m_ifname(m_ifname), m_scanTimeoutCount(0),m_wextPollTimeoutCount(0) {
		m_wpaConnected = false;
		m_timerCount = 0;
		m_logEnabled = true;

		//Workaround as constructor of subclass is not beeing called
		m_apScanDefault = -1;
		qDebug() << (QString("Constructor set ap_scan=%1").arg(QString::number(m_apScanDefault)));
		m_lastWasAdHoc = false;
		qDebug() << (QString("Constructor set m_lastWasAdHoc=%1").arg((m_lastWasAdHoc) ? "true" : "false"));
		//

		connect(QCoreApplication::instance(),SIGNAL(aboutToQuit ()),this,SLOT(detachWpa()));
	}
	CWpaSupplicantBase::~CWpaSupplicantBase() {
		closeWpa("destructor");
	}
	void CWpaSupplicantBase::openWpa(bool) {
		if (m_connectTimerId != -1) {
			killTimer(m_connectTimerId);
			m_connectTimerId = -1;
		}
		if (m_wextTimerId != -1) {
			killTimer(m_wextTimerId);
			m_wextTimerId = -1;
		}
		if (m_wpaConnected) return;
		int status;
		//Open wpa_supplicant control interface
		if (!QFile::exists(m_wpaSupplicantPath)) {
			qWarning() << tr("Could not open wpa_supplicant socket: %1").arg(QString::number(m_timerCount));
			m_inConnectionPhase = true;
			m_connectTimerId = startTimer(dynamicTimerTime(m_timerCount));
			return;
		}
		cmd_ctrl = wpa_ctrl_open(m_wpaSupplicantPath.toAscii().constData());
		event_ctrl = wpa_ctrl_open(m_wpaSupplicantPath.toAscii().constData());
		if (cmd_ctrl == NULL and event_ctrl == NULL) {
			qWarning() << tr("Could not open wpa_supplicant control interface");
			m_inConnectionPhase = true;
			m_connectTimerId = startTimer(dynamicTimerTime(m_timerCount));
			return;
		}
		if (cmd_ctrl == NULL) {
			wpa_ctrl_close(event_ctrl);
			event_ctrl = NULL;
			qWarning() << tr("Could not open wpa_supplicant control interface");
			m_inConnectionPhase = true;
			m_connectTimerId = startTimer(dynamicTimerTime(m_timerCount));
			return;
		}
		if (event_ctrl == NULL) {
			wpa_ctrl_close(cmd_ctrl);
			cmd_ctrl = NULL;
			qWarning() << tr("Could not open wpa_supplicant control interface");
			m_inConnectionPhase = true;
			m_connectTimerId = startTimer(dynamicTimerTime(m_timerCount));
			return;
		}
			
		//Atach event monitor
		status = wpa_ctrl_attach(event_ctrl);
		//Status : 0 = succ; -1 = fail, -2 = timeout
		if (status != 0) {
			wpa_ctrl_close(event_ctrl);
			wpa_ctrl_close(cmd_ctrl);
			event_ctrl = NULL;
			cmd_ctrl = NULL;
			qWarning() << tr("Could not attach to wpa_supplicant");
			m_inConnectionPhase = true;
			m_connectTimerId = startTimer(dynamicTimerTime(m_timerCount));
			return;
		}
		m_inConnectionPhase = false;
		m_timerCount = 0;
		//Set socket notifier
		m_wpaFd = wpa_ctrl_get_fd(event_ctrl);
		m_eventSn  = new QSocketNotifier(m_wpaFd, QSocketNotifier::Read,NULL);
		connect(m_eventSn,SIGNAL(activated(int)),this,SLOT(readFromWpa(int)));
		m_eventSn->setEnabled(true);
		m_wpaConnected = true;
	
		//NEW FEATURE: Signal level retrieving:
		
		//Get file Descriptor to NET kernel
		if ( (m_wextFd = iw_sockets_open()) < 0) {
			m_wextFd = -1;
			qWarning() << tr("ERROR: Could not open socket to net kernel");
		}
		else { //Socket is set up, now set SocketNotifier
			qDebug() << QString("File Descriptor for Wext is: %1").arg(QString::number(m_wextFd));
			
	
			//Start timer for reading wireless info (like in /proc/net/wireless)
			m_wextTimerId = startTimer(m_wextTimerRate);
		}
		//Set ap_scan default
		setApScanDefault();
		
		//Continue of old features:
		emit(stateChanged(true));
		printMessage(tr("wpa_supplicant connection established"));
		return;
	}
	
	bool CWpaSupplicantBase::closeWpa(QString call_func, bool internal) {
		if (m_connectTimerId != -1) {
			killTimer(m_connectTimerId);
			m_connectTimerId = -1;
			m_inConnectionPhase = false;
			m_timerCount = 0;
		}
		if (m_wextTimerId != -1) {
			killTimer(m_wextTimerId);
			m_wextTimerId = -1;
		}
		if (m_wpaConnected) {
			if (m_eventSn != NULL) {
				disconnect(m_eventSn,SIGNAL(activated(int)),this,SLOT(readFromWpa(int)));
				delete m_eventSn;
				m_eventSn = NULL;
			}
			if (-1 != m_wextFd) {
				iw_sockets_close(m_wextFd);
				m_wextFd = -1;
			}
			//Detaching takes place if program is about to quit
			//Close control connections
			wpa_ctrl_close(event_ctrl);
			wpa_ctrl_close(cmd_ctrl);
			event_ctrl = NULL;
			cmd_ctrl = NULL;
			m_wpaConnected = false;
			emit(stateChanged(false));
		}
		printMessage(tr("(%1)[%2] wpa_supplicant disconnected").arg(((internal) ? "internal" : "external"),call_func));
		return true;
	}
	int CWpaSupplicantBase::dynamicTimerTime(int m_timerCount) {
		if (m_timerCount > 0) { 
			if (m_timerCount <= 5) {
				return 1000;
			}
			if (m_timerCount <= 10) {
				return 3000;
			}
			if (m_timerCount <= 15) {
				return 10000;
			}
			return 30000;
		}
		else {
			return 0;
		}
	}
	
	
	//Slot is executed when aplication is about to quit;
	void CWpaSupplicantBase::detachWpa() {
		if (event_ctrl != NULL) {
			wpa_ctrl_detach(event_ctrl);
		}
	}
	
	void CWpaSupplicantBase::setScanResults(QList<WextRawScan> wextScanResults) {
		QString response = wpaCtrlCmd_SCAN_RESULTS();
		if (response.isEmpty()) {
			m_wpaScanResults = QList<ScanResult>();
			return;
		}
		else {
			//Set scan results from wpa_supplicant:
			m_wpaScanResults = parseScanResult(sliceMessage(response));
			//This may not be possible as qHash references an namespace that is unknown to qt TODO:CHECK!
			QHash<QString,WextScan> wextScanHash; //Namespace problem
			WextScan wextScan;
			foreach(WextRawScan i, wextScanResults) {
				wextScan.ssid = i.ssid;
				wextScan.bssid = i.bssid;
				wextScan.signal = convertValues(i);
				wextScan.hasRange = i.hasRange;
				wextScan.we_version_compiled = i.we_version_compiled;
				wextScan.freq = i.freq;
				wextScan.group = i.group;
				wextScan.pairwise = i.pairwise;
				wextScan.keyManagement = i.keyManagement;
				wextScan.protocols = i.protocols;
				wextScan.opmode = i.opmode;
				wextScan.bitrates = i.bitrates;
				wextScanHash.insert(i.bssid.toString(), wextScan);
			}
			WextScan dummy;
			int count = 0;
			QHash<QString,WextScan>::iterator wextScanHashIter;
			//Set the signal quality and so on
			for (QList<ScanResult>::Iterator i = m_wpaScanResults.begin(); i != m_wpaScanResults.end(); ++i ) {
				if (wextScanHash.contains(i->bssid.toString())) { //We do have this network in our wext list
					wextScanHashIter = wextScanHash.find(i->bssid.toString());
					//Workaround for some cards that send wrong information via wext.
					//If ssid is hidden (empty), then we do not set the information from the wext.
					i->bssid = wextScanHashIter.value().bssid;
					if (!wextScanHashIter.value().ssid.isEmpty() && i->ssid.isEmpty()) {
						i->ssid = "<hidden>";
					}
					else if (!wextScanHashIter.value().ssid.isEmpty()) { 
						i->ssid = wextScanHashIter.value().ssid;
					}
					if (-1 != frequencyToChannel(wextScanHashIter.value().freq)) {
						i->freq = wextScanHashIter.value().freq;
					}
					i->signal = wextScanHashIter.value().signal;
					i->group = wextScanHashIter.value().group;
					i->pairwise = wextScanHashIter.value().pairwise;
					i->keyManagement = wextScanHashIter.value().keyManagement;
					i->protocols = wextScanHashIter.value().protocols;
					i->opmode = wextScanHashIter.value().opmode;
					i->bitrates = wextScanHashIter.value().bitrates;
					wextScanHash.erase(wextScanHashIter);
					count++;
				}
				else { //Network is unknown to wext, add level type information
					if (m_signalQuality.type != WSR_UNKNOWN) {
						i->signal.type = m_signalQuality.type;
					}
				}
			}
			ScanResult scanresult;
			//Now add all remaining networks:
			for(QHash<QString,WextScan>::iterator i = wextScanHash.begin(); i != wextScanHash.end(); ++i) {
				scanresult.bssid = i.value().bssid;
				scanresult.ssid = i.value().ssid;
				scanresult.freq = i.value().freq;
				scanresult.signal = i.value().signal;
				scanresult.group = i.value().group;
				scanresult.pairwise = i.value().pairwise;
				scanresult.keyManagement = i.value().keyManagement;
				scanresult.protocols = i.value().protocols;
				scanresult.opmode = i.value().opmode;
				scanresult.bitrates = i.value().bitrates;
				m_wpaScanResults.append(scanresult);
			}

			qDebug() << "Found " << count << "BSSIDs; " << wextScanHash.size() << " in Hash; " << m_wpaScanResults.size() << " in wpa_supplicant list";
			//The complete list is done;
			emit(scanCompleted());
		}
	}
	
	
	
	void CWpaSupplicantBase::timerEvent(QTimerEvent *event) {
		if (event->timerId() == m_wextTimerId) {
			if ( (m_wextFd != -1) && m_wpaConnected) {
				readWirelessInfo();
			}
		}
		else if (event->timerId() == m_connectTimerId) {
			if (!m_wpaConnected) {
				m_timerCount++;
				openWpa(true);
			} else {
				killTimer(m_connectTimerId);
				m_connectTimerId = -1;
				m_timerCount = 0;
			}
		}
		else if (event->timerId() == m_scanTimerId) {
			if ( (m_wextFd != -1) && m_wpaConnected) {
				tryScanResults();
			}
		}
	}
	
	bool CWpaSupplicantBase::connected() {
		if (wpaCtrlCmd_PING() == "PONG\n") {
			return true;
		}
		else {
			return false;
		}
	}
	
	//Public slots
	int CWpaSupplicantBase::getSignalQualityPollRate() {
		return m_wextTimerRate;
	}
	WextSignal CWpaSupplicantBase::getSignalQuality() {
		return m_signalQuality;
	}
	
	void CWpaSupplicantBase::setLog(bool enabled) {
		m_logEnabled = enabled;
	}
	
	QList<ScanResult> CWpaSupplicantBase::scanResults() {
		return m_wpaScanResults;
	}
	
	void CWpaSupplicantBase::scan() {
		if (0 == wpaCtrlCmd_SCAN().indexOf("OK")) {
			//Check if we're already scanning:
			if (m_scanTimerId != -1) {
				killTimer(m_scanTimerId);
				m_scanTimerId = -1;
				printMessage("Already scanning!");
			}
			m_scanTimeoutCount = 0;
			m_scanTimerId = startTimer(CWPA_SCAN_TIMER_TIME);
		}
	}
}

	

