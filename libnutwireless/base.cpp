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
					if (!wextScanHashIter.value().ssid.isEmpty()) { 
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
	
	
	void CWpaSupplicantBase::setSignalQualityPollRate(int msec) {
		m_wextPollTimeoutCount = 0;
		if (m_wextTimerId != -1) {
			qDebug() << "Setting signal quality polling (restarting timer)";
			killTimer(m_wextTimerId);
			m_wextTimerRate = msec;
			m_wextTimerId = startTimer(m_wextTimerRate);
		}
		else {
			qDebug() << "Setting signal quality polling (starting timer)";
			m_wextTimerRate = msec;
		}
	}
	int CWpaSupplicantBase::getSignalQualityPollRate() {
		return m_wextTimerRate;
	}
	WextSignal CWpaSupplicantBase::getSignalQuality() {
		return m_signalQuality;
	}
	
	void CWpaSupplicantBase::setLog(bool enabled) {
		m_logEnabled = enabled;
	}

	
	void CWpaSupplicantBase::scan() {
		if (0 == wpaCtrlCmd_SCAN().indexOf("OK")) {
			//Check if we're already scanning:
			if (m_scanTimerId != -1) {
				killTimer(m_scanTimerId);
				m_scanTimerId = -1;
				printMessage("We're already scanning!");
			}
			//Return scanresults from wpa_supplicant immediately
			setScanResults(QList<WextRawScan>());
			m_scanTimeoutCount = 0;
			m_scanTimerId = startTimer(CWPA_SCAN_TIMER_TIME);
		}
	}
	
	
	//noise levels, returned by this functions, are invalid, don't use them
	void CWpaSupplicantBase::tryScanResults() {
		if (m_wextFd == -1 || !m_wpaConnected) {
			return;
		}
		//Kill Timer:
		if (m_scanTimerId != -1) {
			killTimer(m_scanTimerId);
			m_scanTimerId = -1;
		}
		else { //This is not a timer call; this should not happen
			qWarning() << tr("Warning, no timer present while trying to get scan results");
			return;
		}
		struct iwreq wrq;
		memset(&wrq,0,sizeof(struct iwreq));
		unsigned char * buffer = NULL;		/* Results */
		int buflen = IW_SCAN_MAX_DATA; /* Min for compat WE<17 */
		struct iw_range range;
		memset(&range,0,sizeof(struct iw_range));
		int has_range;
		QList<WextRawScan> res = QList<WextRawScan>();
		WextRawScan singleres;
		singleres.bssid = libnutcommon::MacAddress();
		singleres.group = GCI_UNDEFINED;
		singleres.pairwise = PCI_UNDEFINED;
		singleres.protocols = PROTO_UNDEFINED;
		singleres.keyManagement = KM_UNDEFINED;
		
		/* workaround */
		struct wireless_config wifiConfig;
		memset(&wifiConfig,0,sizeof(struct wireless_config));
	
		/* Get basic information */ 
		if(iw_get_basic_config(m_wextFd, m_ifname.toAscii().constData(), &wifiConfig) < 0) {
			/* If no wireless name : no wireless extensions */ 
			/* But let's check if the interface exists at all */ 
			struct ifreq ifr;
			memset(&ifr,0,sizeof(struct ifreq)); 
		
			strncpy(ifr.ifr_name, m_ifname.toAscii().data(), IFNAMSIZ); 
			if(ioctl(m_wextFd, SIOCGIFFLAGS, &ifr) < 0) 
				qWarning() << tr("(Wireless Extension) No device present");
			else
				qWarning() << tr("(Wireless Extension) Device not supported");
			return;
		}
		qDebug() << "Fetched basic config.";
		
		/* Get AP address */
		if(iw_get_ext(m_wextFd, m_ifname.toAscii().data(), SIOCGIWAP, &wrq) >= 0) {
			qDebug() << "Got AP";
		}
		
		/* Get bit rate */
		if(iw_get_ext(m_wextFd, m_ifname.toAscii().data(), SIOCGIWRATE, &wrq) >= 0) {
			qDebug() << "Got bit rate";
		}
		
		/* Get Power Management settings */
		wrq.u.power.flags = 0;
		if(iw_get_ext(m_wextFd, m_ifname.toAscii().data(), SIOCGIWPOWER, &wrq) >= 0) {
			qDebug() << "Got power";
		}
		/* workaround */
	
		/* Get range stuff */
		/* Get range stuff */
		qDebug() << QString("Getting range stuff for %1").arg(m_ifname.toAscii().data());
		if (iw_get_range_info(m_wextFd, m_ifname.toAscii().data(), &range) >= 0) {
			has_range = 1;
			qDebug() << "Success readWirelessInfo getrange" << strerror(errno);
		}
		else { //This is VERY strange: we always get the "operation not permitted" error, although iw_get_range() worked
			qDebug() << QString("Error \"hasRange == 0\" (%1)").arg(strerror(errno));
		}
		singleres.hasRange = has_range;

		//If we get a timeout for more than 5 times, stop polling.
		if (errno == EAGAIN) {
			qDebug() << "Scan results not available yet";
			m_scanTimeoutCount++;
			if (m_scanTimeoutCount == 5) {
				return;
			}
			m_scanTimerId = startTimer(CWPA_SCAN_RETRY_TIMER_TIME);
			return;
		}
		
		qDebug() << "Got range stuff";
	
		if (has_range) {
			singleres.maxquality.level = (quint8) range.max_qual.level;
			singleres.maxquality.qual = (quint8) range.max_qual.qual;
			singleres.maxquality.noise = (quint8) range.max_qual.noise;
			singleres.maxquality.updated = (quint8) range.max_qual.updated;
			qDebug() << "RANGE (scanresults): " << singleres.maxquality.level  << singleres.maxquality.qual << singleres.maxquality.noise << singleres.maxquality.updated;
		}
		else {
			singleres.maxquality.level = 0;
			singleres.maxquality.qual = 0;
			singleres.maxquality.noise = 0;
			singleres.maxquality.updated = 0;
			qWarning() << tr("Range information are not available");
		}
	
		unsigned char * newbuf;
		for (int i=0; i <= 16; i++) { //realloc (don't do this forever)
			//Allocate newbuffer 
			newbuf = (uchar*) realloc(buffer, buflen);
			if(newbuf == NULL) {
				if (buffer) {
					free(buffer);
					buffer = NULL;
				}
				qDebug() << "Allocating buffer for Wext failed";
				return;
			}
			buffer = newbuf;
			
			//Set Request variables:
			wrq.u.data.pointer = buffer;
			wrq.u.data.flags = 0;
			wrq.u.data.length = buflen;
			
			//Get the data:
			if (iw_get_ext(m_wextFd, m_ifname.toAscii().data(), SIOCGIWSCAN, &wrq) < 0) {
				//Buffer is too small
				if((errno == E2BIG) && (range.we_version_compiled > 16)) {
					
					//check if driver gives any hints about scan length:
					if (wrq.u.data.length > buflen) {
						buflen = wrq.u.data.length;
					}
					else { //If not, double the length
						buflen *= 2;
					}
					//Start from the beginning:
					qDebug() << "Buffer was too small";
					continue;
				}
				//No range error occured or kernel has wireless extension < 16
				if (errno == EAGAIN) {
					qDebug() << "Scan results not available yet";
					m_scanTimerId = startTimer(CWPA_SCAN_RETRY_TIMER_TIME);
					return;
				}
				
				//Bad error occured
				if (buffer) {
					free(buffer);
					buffer = NULL;
				}
				qWarning() << tr("(%1) Failed to read scan data : %2").arg(m_ifname, QString(strerror(errno)));
			}
			else { //Results are there
				break;
			}
		}
	
		//Now read the data:
		if (wrq.u.data.length) {
	
			struct iw_event iwe;
			memset(&iwe,0,sizeof(struct iw_event));
			struct stream_descr stream;
			memset(&stream,0,sizeof(struct stream_descr));
			int ret;
			
			//Init event stream
			QByteArray test;
			char buffer2[128];
			libnutcommon::MacAddress tmpMac;
			iw_init_event_stream(&stream, (char *) buffer, wrq.u.data.length);
			do {
				/* Extract an event and parse it*/
				memset(&iwe,0,sizeof(struct iw_event)); //valgrind complains about it?
				ret = iw_extract_event_stream(&stream, &iwe, range.we_version_compiled);
				if(ret > 0) {
					//Now parse our scan event:
					switch(iwe.cmd) {
						case SIOCGIWAP:
							//ap_addr has type socketaddr
							//Workaround for macaddress
							iw_saether_ntop(&(iwe.u.ap_addr), buffer2);
							tmpMac = libnutcommon::MacAddress(QString::fromAscii(buffer2,128));

							if (singleres.bssid.zero()) { //First bssid
								singleres.bssid = tmpMac;
								qDebug() << "Start parsing one network" << singleres.bssid.toString();
							}
							// if our last bssid is different from the actual one, then we have to append it to our scanlist
							if ( (singleres.bssid != tmpMac) )  {
								res.append(singleres);
								qDebug() << "End parsing one network" << singleres.bssid.toString();
								//reset singleres
								singleres.ssid = QString();
								singleres.bssid.clear();
								singleres.quality = WextRawSignal();
								singleres.freq = -1;
								singleres.group = GCI_UNDEFINED;
								singleres.pairwise = PCI_UNDEFINED;
								singleres.keyManagement = KM_UNDEFINED;
								singleres.protocols = PROTO_UNDEFINED;
								singleres.opmode = OPM_AUTO;
								singleres.bssid = tmpMac;
								qDebug() << "Start parsing one network" << singleres.bssid.toString();
							}
							qDebug() << "BSSID" << singleres.bssid.toString();
							break;
						case IWEVQUAL: //Quality event:
							qDebug() << "Quality" << singleres.bssid.toString();
							singleres.quality.qual = (quint8) iwe.u.qual.qual;
							singleres.quality.level = (quint8) iwe.u.qual.level;
							singleres.quality.noise = (quint8) iwe.u.qual.noise;
							singleres.quality.updated = (quint8) iwe.u.qual.updated;
							qDebug() << "STATS (scanresults): " << singleres.quality.level << singleres.quality.qual << singleres.quality.noise << singleres.quality.updated;
							break;
						case SIOCGIWFREQ:
							qDebug() << "Frequency" << singleres.bssid.toString();
							{
								double freq;
								freq = iw_freq2float(&(iwe.u.freq)); //Hopefully in hz
								if ( ( (freq/1e9) < 10.0 ) && ( (freq/1e9) > 0.0 ) ) {
									singleres.freq = (int) (freq/1e6);
									qDebug() << "Channel for" << singleres.ssid << "is " << freq << singleres.freq;
								}
								else {
									singleres.freq = -1;
								}
							}
							break;
						
						case SIOCGIWMODE:
							qDebug() << "Mode:" << singleres.bssid.toString();
							if(iwe.u.mode >= IW_NUM_OPER_MODE) {
								iwe.u.mode = IW_NUM_OPER_MODE;
							}
							singleres.opmode = (OPMODE) iwe.u.mode;
							break;
						case SIOCGIWESSID:
							qDebug() << "ESSID:" << singleres.bssid.toString();
							if (iwe.u.essid.flags) {
								/* Does it have an ESSID index ? */
								if ( iwe.u.essid.pointer && iwe.u.essid.length ) {
									if ( (iwe.u.essid.flags & IW_ENCODE_INDEX) > 1) {
										singleres.ssid = QString("%1 [%2]").arg(QString::fromAscii((char*) iwe.u.essid.pointer,iwe.u.essid.length), QString::number(iwe.u.essid.flags & IW_ENCODE_INDEX));
									}
									else {
										singleres.ssid = QString("%1").arg(QString::fromAscii((char*) iwe.u.essid.pointer,iwe.u.essid.length));
									}
								}
								else {
									singleres.ssid = QString("N/A");
								}
							}
							else {
								singleres.ssid = QString("<hidden>");
							}
							break;

						case SIOCGIWENCODE: //Get encrytion stuff: (just the key)
							qDebug() << "Encode" << singleres.bssid.toString();
							if (! iwe.u.data.pointer) {
								iwe.u.data.flags |= IW_ENCODE_NOKEY;
							}
							if(iwe.u.data.flags & IW_ENCODE_DISABLED) { //Encryption is disabled
								singleres.keyManagement = KM_OFF;
								qDebug() << "PARING ENCODE-Information: NO KEY";
							}
							else {
								//Extract key information: (See iwlib.c line 1500)
								
								//keyflags: iwe.u.data.flags
								//keysize = iwe.u.data.length
								//
								//Do we have a key
								if (iwe.u.data.flags & IW_ENCODE_NOKEY) { //no key
									if (iwe.u.data.length <= 0) {
										//Encryption is on, but group is unknown
										singleres.keyManagement = KM_NONE;
										qDebug() << "PARSING ENCODE-INFORMATION: WEP KEY";
									}
								} //else: we have a, key but check type later
							}
							break;

						case SIOCGIWRATE:
							singleres.bitrates.append((qint32) iwe.u.bitrate.value);
							qDebug() << "Adding Bitrate: " << (qint32) iwe.u.bitrate.value;
							break;
						
						case IWEVGENIE: //group/pairwsie ciphers etc.
							//buffer = iwe.u.data.pointer
							//bufflen = iwe.u.data.length
							qDebug() << "IE_START" << singleres.bssid.toString();
							{
								int offset = 0;
								/* Loop on each IE, each IE is minimum 2 bytes */
								while(offset <= (iwe.u.data.length - 2)) {
									/* Check IE type */
									if (0xdd == ((uchar *) iwe.u.data.pointer)[offset] || (0x30 == ((uchar *) iwe.u.data.pointer)[offset]) ) { // WPA1/2
										parseWextIeWpa(((uchar *) iwe.u.data.pointer) + offset, iwe.u.data.length, &singleres);

									qDebug() << "Parsed IE-Information of " << singleres.ssid << singleres.bssid.toString();
									qDebug() << toString(singleres.group) << toString(singleres.pairwise) << toString(singleres.keyManagement);
									}
									/* Skip over this IE to the next one in the list. */
									offset += buffer[offset+1] + 2;
								}
							}
							qDebug() << "IE_END" << singleres.bssid.toString();
							break;

						default: //Ignore all other event types. Maybe we need them later?
							break;
					}
				}
				else { //Append last scan:
					if (!singleres.bssid.zero()) {
						res.append(singleres);
						qDebug() << "End parsing one network" << singleres.bssid.toString();
					}
				}
		
			} while(ret > 0);

			//Delete buffer
			if (buffer) {
				free(buffer);
				buffer = NULL;
			}
			//We have the data, now construct complete ScanResult
			setScanResults(res);
		}
		else {
			qWarning() << tr("No Scanresults available");
		}
		if (buffer) {
			free(buffer);
			buffer = NULL;
		}
	}
	
	void CWpaSupplicantBase::readWirelessInfo() {
		if ( (m_wextFd == -1) || !m_wpaConnected) {
			return;
		}
		qDebug() << "Executing readWirelessInfo() with TimerRate of" << m_wextTimerRate;
		struct iw_range range;
		memset(&range,0,sizeof(struct iw_range));
		int hasRange = 0;
		iwstats stats;
		memset(&stats,0,sizeof(iwstats));
		WextRawScan res;
		/* workaround */
		struct wireless_config wifiConfig;
		memset(&wifiConfig,0,sizeof(struct wireless_config));
		/* Get basic information */ 
		if(iw_get_basic_config(m_wextFd, m_ifname.toAscii().constData(), &wifiConfig) < 0) {
			/* If no wireless name : no wireless extensions */ 
			/* But let's check if the interface exists at all */ 
			struct ifreq ifr;
			memset(&ifr,0,sizeof(struct ifreq)); 
		
			strncpy(ifr.ifr_name, m_ifname.toAscii().data(), IFNAMSIZ); 
			if(ioctl(m_wextFd, SIOCGIFFLAGS, &ifr) < 0) 
				qWarning() << tr("(Wireless Extension) No device present");
			else
				qWarning() << tr("(Wireless Extension) device not supported");
			return;
		}
		qDebug() << "Fetched basic config.";

		//We encode frequency in mhz;
		//kernel encodes as double; (hopefully always in hz)
		//But better test this:
		res.freq = -1;
		if (wifiConfig.has_freq) {
			if ( ( (wifiConfig.freq/1e9) < 10.0 ) && ( (wifiConfig.freq/1e9) > 0.0 ) ) {
				res.freq = (int) (wifiConfig.freq/1e6);
			}
		}
		
		struct iwreq wrq;
		memset(&wrq,0,sizeof(struct iwreq));
		/* Get AP address */
		if(iw_get_ext(m_wextFd, m_ifname.toAscii().data(), SIOCGIWAP, &wrq) >= 0) {
			//Add mac address of current ap;
			char buffer[128];
			iw_saether_ntop(&(wrq.u.ap_addr), buffer);
			res.bssid = libnutcommon::MacAddress(QString::fromAscii(buffer,128));
			qDebug() << "Got AP: " << res.bssid.toString();
		}
		
		/* Get bit rate */
		if(iw_get_ext(m_wextFd, m_ifname.toAscii().data(), SIOCGIWRATE, &wrq) >= 0) {
			res.bitrates.append((qint32) wrq.u.bitrate.value);
			qDebug() << "Got bit rate: " << res.bitrates[0];
		}
		
		/* Get Power Management settings */
		wrq.u.power.flags = 0;
		if(iw_get_ext(m_wextFd, m_ifname.toAscii().data(), SIOCGIWPOWER, &wrq) >= 0) {
			qDebug() << "Got power";
		}
		/* workaround */
			
		/* Get range stuff */
		qDebug() << QString("Getting range stuff for %1").arg(m_ifname.toAscii().data());
		if (iw_get_range_info(m_wextFd, m_ifname.toAscii().data(), &range) >= 0) {
			hasRange = 1;
			qDebug() << "Success readWirelessInfo getrange" << strerror(errno);
		}
		else { //This is VERY strange: we always get the "operation not permitted" error, although iw_get_range() worked
			qDebug() << QString("Error \"hasRange == 0\" (%1)").arg(strerror(errno));
		}
		res.hasRange = hasRange;
		if (errno == EAGAIN) {
			m_wextPollTimeoutCount++;
			qDebug() << QString("Getting range stuff failed (%1)").arg(strerror(errno));
			if ( (m_wextPollTimeoutCount > 10) && m_wextTimerRate < 1000) {
				setSignalQualityPollRate(10000);
			}
			else if (m_wextPollTimeoutCount > 10) { //Fast polling disabled, but still EAGAIN errors
				//Seems the kernel does not care about our calls
				killTimer(m_wextTimerId);
				m_wextTimerId = -1;
			}
			return;
		}
		
		qDebug() << "Got range stuff";
		

		//Set supported Frequencies if the list is not empty;
		if (m_supportedFrequencies.isEmpty() && hasRange) {
			qDebug() << range.num_frequency;
			qDebug() << "Printing Frequency information";
			quint32 m;
			quint16 e;
			quint32 freqinmhz;
			for (int i=0; i < range.num_channels; i++) {
				m = (quint32) range.freq[i].m;
				e = (quint16) range.freq[i].e;
				freqinmhz = m;
				for (int j=0; j < 9-e-3; j++) {
					freqinmhz = freqinmhz/10;
				}
				if (!m_supportedFrequencies.contains(freqinmhz)) { //Only add frequency that are not in our list
					m_supportedFrequencies.append(freqinmhz);
				}
				qDebug() << m << e << freqinmhz << frequencyToChannel(freqinmhz);
			}
			qDebug() << "Done printing";
		}
		else {
			qDebug() << "m_supportedFrequencies not set";
		}
		
		if (hasRange) {
			res.maxquality.level = (quint8) range.max_qual.level;
			res.maxquality.qual = (quint8) range.max_qual.qual;
			res.maxquality.noise = (quint8) range.max_qual.noise;
			res.maxquality.updated = (quint8) range.max_qual.updated;
			qDebug() << "RANGE: " << res.maxquality.level  << res.maxquality.qual << res.maxquality.noise << res.maxquality.updated;
		}
		else {
			res.maxquality.level = 0;
			res.maxquality.qual = 0;
			res.maxquality.noise = 0;
			res.maxquality.updated = 0;
			qDebug() << "Range information are not available";
		}
		if ( (hasRange) && (range.we_version_compiled > 11) ) {
			struct iwreq wrq;
			memset(&wrq,0,sizeof(struct iwreq));
			wrq.u.data.pointer = (caddr_t) &stats;
			wrq.u.data.length = sizeof(struct iw_statistics);
			wrq.u.data.flags = 1; // Clear updated flag
			strncpy(wrq.ifr_name, m_ifname.toAscii().data(), IFNAMSIZ);
			
			qDebug() << "Getting wireless stats";
			if(iw_get_ext(m_wextFd, m_ifname.toAscii().data(), SIOCGIWSTATS, &wrq) < 0) {
				qWarning() << tr("Error occured while fetching wireless info: ") << strerror(errno);
			}
			else { //Stats fetched
				qDebug() << "Stats fetched";
				res.quality.level = (quint8) stats.qual.level;
				res.quality.qual = (quint8) stats.qual.qual;
				res.quality.noise = (quint8) stats.qual.noise;
				res.quality.updated = (quint8) stats.qual.updated;
				qDebug() << "STATS: " << res.quality.level << res.quality.qual << res.quality.noise << res.quality.updated;
				m_signalQuality = convertValues(res);
				qDebug() << "Emittig m_signalQualityUpdated()";
				if (m_wextTimerRate < 1000) {
					setSignalQualityPollRate(10000);
					printMessage(tr("Auto-resetting timer to 10 seconds"));
				}
				emit signalQualityUpdated(m_signalQuality);
			}
		}
		else if (range.we_version_compiled <= 11) {
			printMessage(tr("Cannot fetch wireless information as your wireless extension is too old"));
			printMessage(tr("Think about updating your kernel (it's way too old)"));
		}
		else {
			qDebug() << "Error while trying to fetch wireless information" << strerror(errno);
			qDebug() << "Wireless Extension socket file descriptor was: " << m_wextFd;
		}
	}
	
	
	QList<ScanResult> CWpaSupplicantBase::scanResults() {
		return m_wpaScanResults;
	}
}
