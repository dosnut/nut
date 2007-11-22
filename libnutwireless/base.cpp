#include "base.h"
#include <QDebug>

namespace libnutwireless {
	
	//CWpa_supplicant
	
	
	//Wpa_supplicant control commands:
	//WARNING!!!! THIS MAY HAVE SOME MEMORY LEAKS!!! Check if char * is deleted in wpa_ctrl_request
	QString CWpa_SupplicantBase::wps_ctrl_command(QString cmd = "PING") {
		//int wpa_ctrl_request(struct wpa_ctrl *ctrl, const char *cmd, size_t cmd_len,
		//	     char *reply, size_t *reply_len,
		//	     void (*msg_cb)(char *msg, size_t len));
		//	     
		//Check if we have a control interface:
		if (cmd_ctrl == NULL || event_ctrl == NULL) {
			return QString();
		}
		if (!wps_connected or inConnectionPhase) {
			return QString();
		}
		//First Check if wpa_supplicant is running:
		size_t command_len;
		const char * command;
		char reply[4096];
		size_t reply_len = sizeof(reply);
		
		command = "PING";
		command_len = strlen(command);
		int status = wpa_ctrl_request(cmd_ctrl, command, command_len, reply, &reply_len,NULL);
		if ( (status != 0) or (QString::fromUtf8(reply, reply_len) != "PONG\n") ) {
			qDebug() << QString("(status=%2)PING COMMAND RESPONSE: %1").arg(QString::fromUtf8(reply, reply_len),QString::number(status));
			//TODO: Maybe we need to detach and close wpa_supplicant:
			wps_close("wps_ctrl_command/nopong");
			return QString();
		}
		if (cmd != "PING") {
			size_t reply_len = sizeof(reply);
			command = cmd.toAscii().data();
			command_len = cmd.toAscii().size();
			
			status = wpa_ctrl_request(cmd_ctrl, command, command_len, reply, &reply_len,NULL);
			if (0 == status) {
				qDebug() << cmd << ":" << QString::fromUtf8(reply, reply_len) << "\nEOC";
				if (reply_len > 0) {
					//TODO:Check if reply is \0 terminated
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
	
	
	
	
	
	
// 	void CWpa_SupplicantBase::printMessage(QString msg) {
// 		if (log_enabled) {
// 			emit(message(msg));
// 		}
// 	}
	
	
	//Private slots:
	//Reads messages from wpa_supplicant
	////TODO:Check if we need to check if wpa_supplicant is still available
	void CWpa_SupplicantBase::wps_read(int socket) {
		if (socket == wps_fd) {
			if (wps_connected) {
				//check if wpa_supplicant is still running (i.e. file exists)
				if (! QFile::exists(wpa_supplicant_path)) {
					wps_close("wps_read/no wps interface on event");
					return;
				}
				//status: 1 = msg available; 0 = no messages, -1 = error
				int status = wpa_ctrl_pending(event_ctrl);
				if (1 == status) {
					//Receive Messages
					char reply[512];
					size_t reply_len = sizeof(reply);
					wpa_ctrl_recv(event_ctrl, reply, &reply_len);
					Event_dispatcher(QString::fromUtf8(reply, reply_len));
				}
				else if (-1 == status) {
					qDebug() << "Error while trying to receive messages from wpa_supplicant";
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
	void CWpa_SupplicantBase::Event_dispatcher(Request req) {
		if (req.type != REQ_FAIL) {
			emit(request(req));
		}
	}
	void CWpa_SupplicantBase::Event_dispatcher(EventType event) {
		if (event == EVENT_CONNECTED) {
			emit(stateChanged(true));
		}
		else if (event == EVENT_DISCONNECTED) {
			emit(stateChanged(false));
		}
		else if (event == EVENT_TERMINATING) {
			wps_close("event-dispatcher/wpa-TERMINATING");
		}
		else {
			emit(eventMessage(event));
		}
	}
	
	void CWpa_SupplicantBase::Event_dispatcher(QString event) {
		QStringList str_list = event.split('\n',QString::KeepEmptyParts);
		InteractiveType type;
		foreach(QString str, str_list) {
			type = parseInteract(str);
			switch (type) {
				case (INTERACT_REQ):
					Event_dispatcher(parseReq(str));
					break;
				case (INTERACT_EVENT):
					Event_dispatcher(parseEvent(str));
					break;
				default:
					printMessage(str);
					break;
			}
		}
	}
	
	//Public functions:
	CWpa_SupplicantBase::CWpa_SupplicantBase(QObject * parent, QString ifname) : QObject(parent), cmd_ctrl(0), event_ctrl(0), wpa_supplicant_path("/var/run/wpa_supplicant/"+ifname), wps_fd(-1), wext_fd(-1), event_sn(0), timerId(-1), wextTimerId(-1), ScanTimerId(-1), wextTimerRate(10000), ifname(ifname), ScanTimeoutCount(0),wextPollTimeoutCount(0) {
		wps_connected = false;
		timerCount = 0;
		log_enabled = true;

		//Workaround as constructor of subclass is not beeing called
		apScanDefault = -1;
		qDebug() << (QString("Constructor set ap_scan=%1").arg(QString::number(apScanDefault)));
		lastWasAdHoc = false;
		qDebug() << (QString("Constructor set lastWasAdHoc=%1").arg((lastWasAdHoc) ? "true" : "false"));
		//

		connect(QCoreApplication::instance(),SIGNAL(aboutToQuit ()),this,SLOT(wps_detach()));
	}
	CWpa_SupplicantBase::~CWpa_SupplicantBase() {
		wps_close("destructor");
	}
	void CWpa_SupplicantBase::wps_open(bool) {
		if (timerId != -1) {
			killTimer(timerId);
			timerId = -1;
		}
		if (wextTimerId != -1) {
			killTimer(wextTimerId);
			wextTimerId = -1;
		}
		if (wps_connected) return;
		int status;
		//Open wpa_supplicant control interface
		if (!QFile::exists(wpa_supplicant_path)) {
			qDebug() << tr("Could not open wpa_supplicant socket: %1").arg(QString::number(timerCount));
			inConnectionPhase = true;
			timerId = startTimer(wps_TimerTime(timerCount));
			return;
		}
		cmd_ctrl = wpa_ctrl_open(wpa_supplicant_path.toAscii().constData());
		event_ctrl = wpa_ctrl_open(wpa_supplicant_path.toAscii().constData());
		if (cmd_ctrl == NULL and event_ctrl == NULL) {
			qDebug() << tr("Could not open wpa_supplicant control interface");
			inConnectionPhase = true;
			timerId = startTimer(wps_TimerTime(timerCount));
			return;
		}
		if (cmd_ctrl == NULL) {
			wpa_ctrl_close(event_ctrl);
			event_ctrl = NULL;
			qDebug() << tr("Could not open wpa_supplicant control interface");
			inConnectionPhase = true;
			timerId = startTimer(wps_TimerTime(timerCount));
			return;
		}
		if (event_ctrl == NULL) {
			wpa_ctrl_close(cmd_ctrl);
			cmd_ctrl = NULL;
			qDebug() << tr("Could not open wpa_supplicant control interface");
			inConnectionPhase = true;
			timerId = startTimer(wps_TimerTime(timerCount));
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
			qDebug() << tr("Could not attach to wpa_supplicant");
			inConnectionPhase = true;
			timerId = startTimer(wps_TimerTime(timerCount));
			return;
		}
		inConnectionPhase = false;
		timerCount = 0;
		//Set socket notifier
		wps_fd = wpa_ctrl_get_fd(event_ctrl);
		event_sn  = new QSocketNotifier(wps_fd, QSocketNotifier::Read,NULL);
		connect(event_sn,SIGNAL(activated(int)),this,SLOT(wps_read(int)));
		event_sn->setEnabled(true);
		wps_connected = true;
	
		//NEW FEATURE: Signal level retrieving:
		
		//Get file Descriptor to NET kernel
		if ( (wext_fd = iw_sockets_open()) < 0) {
			wext_fd = -1;
			qDebug() << "ERROR: Could not open socket to net kernel";
		}
		else { //Socket is set up, now set SocketNotifier
			qDebug() << QString("File Descriptor for Wext is: %1").arg(QString::number(wext_fd));
			
	
			//Start timer for reading wireless info (like in /proc/net/wireless)
			wextTimerId = startTimer(wextTimerRate);
		}
		
	
		//Continue of ol features:
		emit(opened());
		printMessage(tr("wpa_supplicant connection established"));
		return;
	}
	
	bool CWpa_SupplicantBase::wps_close(QString call_func, bool internal) {
		if (timerId != -1) {
			killTimer(timerId);
			timerId = -1;
			inConnectionPhase = false;
			timerCount = 0;
		}
		if (wextTimerId != -1) {
			killTimer(wextTimerId);
			wextTimerId = -1;
		}
		if (wps_connected) {
			if (event_sn != NULL) {
				disconnect(event_sn,SIGNAL(activated(int)),this,SLOT(wps_read(int)));
				delete event_sn;
				event_sn = NULL;
			}
			if (-1 != wext_fd) {
				iw_sockets_close(wext_fd);
				wext_fd = -1;
			}
			//Detaching takes place if program is about to quit
			//Close control connections
			wpa_ctrl_close(event_ctrl);
			wpa_ctrl_close(cmd_ctrl);
			event_ctrl = NULL;
			cmd_ctrl = NULL;
			wps_connected = false;
			emit(closed());
		}
		printMessage(tr("(%1)[%2] wpa_supplicant disconnected").arg(((internal) ? "internal" : "external"),call_func));
		return true;
	}
	int CWpa_SupplicantBase::wps_TimerTime(int timerCount) {
		if (timerCount > 0) { 
			if (timerCount <= 5) {
				return 1000;
			}
			if (timerCount <= 10) {
				return 3000;
			}
			if (timerCount <= 15) {
				return 10000;
			}
			return 30000;
		}
		else {
			return 0;
		}
	}
	
	
	//Slot is executed when aplication is about to quit;
	void CWpa_SupplicantBase::wps_detach() {
		if (event_ctrl != NULL) {
			wpa_ctrl_detach(event_ctrl);
		}
	}
	
	void CWpa_SupplicantBase::wps_setScanResults(QList<WextRawScan> wextScanResults) {
		QString response = wps_cmd_SCAN_RESULTS();
		if (response.isEmpty()) {
			wpsScanResults = QList<ScanResult>();
			return;
		}
		else {
			//Set scan results from wpa_supplicant:
			wpsScanResults = parseScanResult(sliceMessage(response));
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
				wextScanHash.insert(i.bssid.toString(), wextScan);
			}
			WextScan dummy;
			int count = 0;
			QHash<QString,WextScan>::iterator wextScanHashIter;
			//Set the signal quality and so on
			for (QList<ScanResult>::Iterator i = wpsScanResults.begin(); i != wpsScanResults.end(); ++i ) {
				if (wextScanHash.contains(i->bssid.toString())) {
					wextScanHashIter = wextScanHash.find(i->bssid.toString());
					i->bssid = wextScanHashIter.value().bssid;
					i->ssid = wextScanHashIter.value().ssid;
					if (-1 != frequencyToChannel(wextScanHashIter.value().freq)) {
						i->freq = wextScanHashIter.value().freq;
					}
					i->signal = wextScanHashIter.value().signal;
					i->group = wextScanHashIter.value().group;
					i->pairwise = wextScanHashIter.value().pairwise;
					i->keyManagement = wextScanHashIter.value().keyManagement;
					i->protocols = wextScanHashIter.value().protocols;
					i->opmode = wextScanHashIter.value().opmode;
					wextScanHash.erase(wextScanHashIter);
					count++;
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
				wpsScanResults.append(scanresult);
			}

			qDebug() << "Found " << count << "BSSIDs; " << wextScanHash.size() << " in Hash; " << wpsScanResults.size() << " in wpa_supplicant list";
			//The complete list is done;
			emit(scanCompleted());
		}
	}
	
	
	
	void CWpa_SupplicantBase::timerEvent(QTimerEvent *event) {
		if (event->timerId() == wextTimerId) {
			if ( (wext_fd != -1) && wps_connected) {
				readWirelessInfo();
			}
		}
		else if (event->timerId() == timerId) {
			if (!wps_connected) {
				timerCount++;
				wps_open(true);
			} else {
				killTimer(timerId);
				timerId = -1;
				timerCount = 0;
			}
		}
		else if (event->timerId() == ScanTimerId) {
			if ( (wext_fd != -1) && wps_connected) {
				wps_tryScanResults();
			}
		}
	}
	
	bool CWpa_SupplicantBase::connected() {
		if (wps_cmd_PING() == "PONG\n") {
			return true;
		}
		else {
			return false;
		}
	}
	
	//Public slots
	
	
	void CWpa_SupplicantBase::setSignalQualityPollRate(int msec) {
		wextPollTimeoutCount = 0;
		if (wextTimerId != -1) {
			qDebug() << "Setting signal quality polling (restarting timer)";
			killTimer(wextTimerId);
			wextTimerRate = msec;
			wextTimerId = startTimer(wextTimerRate);
		}
		else {
			qDebug() << "Setting signal quality polling (starting timer)";
			wextTimerRate = msec;
		}
	}
	int CWpa_SupplicantBase::getSignalQualityPollRate() {
		return wextTimerRate;
	}
	WextSignal CWpa_SupplicantBase::getSignalQuality() {
		return signalQuality;
	}
	
	void CWpa_SupplicantBase::setLog(bool enabled) {
		log_enabled = enabled;
	}

	
	void CWpa_SupplicantBase::scan() {
		if (0 == wps_cmd_SCAN().indexOf("OK")) {
			//Check if we're already scanning:
			if (ScanTimerId != -1) {
				killTimer(ScanTimerId);
				ScanTimerId = -1;
				printMessage("We're already scanning!");
			}
			//Return scanresults from wpa_supplicant immediately
			wps_setScanResults(QList<WextRawScan>());
			ScanTimeoutCount = 0;
			ScanTimerId = startTimer(CWPA_SCAN_TIMER_TIME);
		}
	}
	
	
	//noise levels, returned by this functions, are invalid, don't use them
	void CWpa_SupplicantBase::wps_tryScanResults() {
		if (wext_fd == -1 || !wps_connected) {
			return;
		}
		//Kill Timer:
		if (ScanTimerId != -1) {
			killTimer(ScanTimerId);
			ScanTimerId = -1;
		}
		else { //This is not a timer call; this should not happen
			qDebug() << "Warning, no timer present while trying to get scan results";
			return;
		}
		struct iwreq wrq;
		unsigned char * buffer = NULL;		/* Results */
		int buflen = IW_SCAN_MAX_DATA; /* Min for compat WE<17 */
		struct iw_range range;
		int has_range;
		QList<WextRawScan> res;
		WextRawScan singleres;
		singleres.bssid = libnutcommon::MacAddress();
		singleres.group = GCI_UNDEFINED;
		singleres.pairwise = PCI_UNDEFINED;
		singleres.protocols = PROTO_UNDEFINED;
		singleres.keyManagement = KM_UNDEFINED;
		
		/* workaround */
		struct wireless_config b;
		/* Get basic information */ 
		if(iw_get_basic_config(wext_fd, ifname.toAscii().data(), &b) < 0) {
			/* If no wireless name : no wireless extensions */ 
			/* But let's check if the interface exists at all */ 
			struct ifreq ifr; 
		
			strncpy(ifr.ifr_name, ifname.toAscii().data(), IFNAMSIZ); 
			if(ioctl(wext_fd, SIOCGIFFLAGS, &ifr) < 0) 
				qDebug() << "no device";
			else
				qDebug() << "not supported";
			return;
		}
		qDebug() << "Fetched basic config.";
		
		/* Get AP address */
		if(iw_get_ext(wext_fd, ifname.toAscii().data(), SIOCGIWAP, &wrq) >= 0) {
			qDebug() << "Got AP";
		}
		
		/* Get bit rate */
		if(iw_get_ext(wext_fd, ifname.toAscii().data(), SIOCGIWRATE, &wrq) >= 0) {
			qDebug() << "Got bit rate";
		}
		
		/* Get Power Management settings */
		wrq.u.power.flags = 0;
		if(iw_get_ext(wext_fd, ifname.toAscii().data(), SIOCGIWPOWER, &wrq) >= 0) {
			qDebug() << "Got power";
		}
		/* workaround */
	
		/* Get range stuff */
		/* Get range stuff */
		qDebug() << QString("Getting range stuff for %1").arg(ifname.toAscii().data());
		if (iw_get_range_info(wext_fd, ifname.toAscii().data(), &range) >= 0) {
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
			ScanTimeoutCount++;
			if (ScanTimeoutCount == 5) {
				return;
			}
			ScanTimerId = startTimer(CWPA_SCAN_RETRY_TIMER_TIME);
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
			qDebug() << "Range information are not available";
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
			if (iw_get_ext(wext_fd, ifname.toAscii().data(), SIOCGIWSCAN, &wrq) < 0) {
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
					ScanTimerId = startTimer(CWPA_SCAN_RETRY_TIMER_TIME);
					return;
				}
				
				//Bad error occured
				if (buffer) {
					free(buffer);
					buffer = NULL;
				}
				qDebug() << QString("(%1) Failed to read scan data : %2").arg(ifname, QString(strerror(errno)));
			}
			else { //Results are there
				break;
			}
		}
	
		//Now read the data:
		if (wrq.u.data.length) {
	
			struct iw_event iwe;
			struct stream_descr stream;
			int ret;
			
			//Init event stream
			QByteArray test;
			char buffer2[128];
			libnutcommon::MacAddress tmpMac;
			iw_init_event_stream(&stream, (char *) buffer, wrq.u.data.length);
			do {
				/* Extract an event and parse it*/
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
							qDebug() << "unuse event type";
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
			wps_setScanResults(res);
		}
		else {
			qDebug() << "No Scanresults available";
		}
		if (buffer) {
			free(buffer);
			buffer = NULL;
		}
	}
	
	void CWpa_SupplicantBase::readWirelessInfo() {
		if ( (wext_fd == -1) || !wps_connected) {
			return;
		}
		qDebug() << "Executing readWirelessInfo() with TimerRate of" << wextTimerRate;
		struct iw_range range;
		int hasRange = 0;
		iwstats stats;
		WextRawScan res;
		/* workaround */
		struct wireless_config b;
		/* Get basic information */ 
		if(iw_get_basic_config(wext_fd, ifname.toAscii().data(), &b) < 0) {
			/* If no wireless name : no wireless extensions */ 
			/* But let's check if the interface exists at all */ 
			struct ifreq ifr; 
		
			strncpy(ifr.ifr_name, ifname.toAscii().data(), IFNAMSIZ); 
			if(ioctl(wext_fd, SIOCGIFFLAGS, &ifr) < 0) 
				qDebug() << "no device";
			else
				qDebug() << "not supported";
			return;
		}
		qDebug() << "Fetched basic config.";

		//We encode frequency in mhz;
		//kernel encodes as double; (hopefully always in hz)
		//But better test this:
		res.freq = -1;
		if (b.has_freq) {
			if ( ( (b.freq/1e9) < 10.0 ) && ( (b.freq/1e9) > 0.0 ) ) {
				res.freq = (int) (b.freq/1e6);
			}
		}
		
		struct iwreq wrq;
		/* Get AP address */
		if(iw_get_ext(wext_fd, ifname.toAscii().data(), SIOCGIWAP, &wrq) >= 0) {
			qDebug() << "Got AP";
		}
		
		/* Get bit rate */
		if(iw_get_ext(wext_fd, ifname.toAscii().data(), SIOCGIWRATE, &wrq) >= 0) {
			qDebug() << "Got bit rate";
		}
		
		/* Get Power Management settings */
		wrq.u.power.flags = 0;
		if(iw_get_ext(wext_fd, ifname.toAscii().data(), SIOCGIWPOWER, &wrq) >= 0) {
			qDebug() << "Got power";
		}
		/* workaround */
			
		/* Get range stuff */
		qDebug() << QString("Getting range stuff for %1").arg(ifname.toAscii().data());
		if (iw_get_range_info(wext_fd, ifname.toAscii().data(), &range) >= 0) {
			hasRange = 1;
			qDebug() << "Success readWirelessInfo getrange" << strerror(errno);
		}
		else { //This is VERY strange: we always get the "operation not permitted" error, although iw_get_range() worked
			qDebug() << QString("Error \"hasRange == 0\" (%1)").arg(strerror(errno));
		}
		res.hasRange = hasRange;
		if (errno == EAGAIN) {
			wextPollTimeoutCount++;
			qDebug() << QString("Getting range stuff failed (%1)").arg(strerror(errno));
			if ( (wextPollTimeoutCount > 10) && wextTimerRate < 1000) {
				setSignalQualityPollRate(10000);
			}
			else if (wextPollTimeoutCount > 10) { //Fast polling disabled, but still EAGAIN errors
				//Seems the kernel does not care about our calls
				killTimer(wextTimerId);
				wextTimerId = -1;
			}
			return;
		}
		
		qDebug() << "Got range stuff";
		

		//Set supported Frequencies if the list is not empty;
		if (supportedFrequencies.isEmpty() && hasRange) {
			qDebug() << range.num_frequency;
			qDebug() << "Printing Frequency information";
			quint32 m;
			quint16 e;
			quint32 freqinmhz;
	// 		QList<struct iw_freq> freqlist;
			for (int i=0; i < range.num_channels; i++) {
				m = (quint32) range.freq[i].m;
				e = (quint16) range.freq[i].e;
				freqinmhz = m;
				for (int j=0; j < 9-e-3; j++) {
					freqinmhz = freqinmhz/10;
				}
				supportedFrequencies.append(freqinmhz);
				qDebug() << m << e << freqinmhz << frequencyToChannel(freqinmhz);
			}
			qDebug() << "Done printing";
		}
		else {
			qDebug() << "supportedFrequencies not set";
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
			wrq.u.data.pointer = (caddr_t) &stats;
			wrq.u.data.length = sizeof(struct iw_statistics);
			wrq.u.data.flags = 1; // Clear updated flag
			strncpy(wrq.ifr_name, ifname.toAscii().data(), IFNAMSIZ);
			
			qDebug() << "Getting wireless stats";
			if(iw_get_ext(wext_fd, ifname.toAscii().data(), SIOCGIWSTATS, &wrq) < 0) {
				qDebug() << "Error occured while fetching wireless info" << strerror(errno);
			}
			else { //Stats fetched
				qDebug() << "Stats fetched";
				res.quality.level = (quint8) stats.qual.level;
				res.quality.qual = (quint8) stats.qual.qual;
				res.quality.noise = (quint8) stats.qual.noise;
				res.quality.updated = (quint8) stats.qual.updated;
				qDebug() << "STATS: " << res.quality.level << res.quality.qual << res.quality.noise << res.quality.updated;
				signalQuality = convertValues(res);
				qDebug() << "Emittig signalQualityUpdated()";
				if (wextTimerRate < 1000) {
					setSignalQualityPollRate(10000);
					qDebug() << "Auto-resetting timer to 10 seconds";
				}
				emit signalQualityUpdated(signalQuality);
			}
		}
		else if (range.we_version_compiled <= 11) {
			printMessage(tr("Cannot fetch wireless information as your wireless extension is too old"));
			printMessage(tr("Think about updating your kernel (it's way too old)"));
		}
		else {
			qDebug() << "Error while trying to fetch wireless information" << strerror(errno);
			qDebug() << "Wireless Extension socket file descriptor was: " << wext_fd;
		}
	}
	
	
	QList<ScanResult> CWpa_SupplicantBase::scanResults() {
		return wpsScanResults;
	}
}
