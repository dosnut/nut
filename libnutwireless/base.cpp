#include "wpa_supplicant.h"
#include <QDebug>

extern "C" {
#include <linux/wireless.h>
#include "wpa_ctrl/wpa_ctrl.h"
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
}

namespace libnutwireless {

	//CWpa_supplicant


	//Wpa_supplicant control commands:
	QString CWpaSupplicant::wpaCtrlCommand(QString cmd = "PING") {
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






// 	void CWpaSupplicant::printMessage(QString msg) {
// 		if (m_logEnabled) {
// 			emit(message(msg));
// 		}
// 	}


	//Private slots:
	//Reads messages from wpa_supplicant
	void CWpaSupplicant::readFromWpa(int socket) {
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
	void CWpaSupplicant::eventDispatcher(Request req) {
		if (req.type != REQ_FAIL) {
			emit(request(req));
		}
	}
	void CWpaSupplicant::eventDispatcher(EventType event, QString str) {
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

	void CWpaSupplicant::eventDispatcher(QString event) {
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

	void CWpaSupplicant::openWpa(bool) {
		if (m_connectTimerId != -1) {
			killTimer(m_connectTimerId);
			m_connectTimerId = -1;
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

		//Set ap_scan default
		setApScanDefault();

		//Continue of old features:
		emit(stateChanged(true));
		printMessage(tr("wpa_supplicant connection established"));
		if (m_managedNetworks.size() > 0) {
			addOnlyNewNetworks(m_managedNetworks.values());

		}
		return;
	}

	bool CWpaSupplicant::closeWpa(QString call_func, bool internal) {
		if (m_connectTimerId != -1) {
			killTimer(m_connectTimerId);
			m_connectTimerId = -1;
			m_inConnectionPhase = false;
			m_timerCount = 0;
		}
		if (m_wpaConnected) {
			if (m_eventSn != NULL) {
				disconnect(m_eventSn,SIGNAL(activated(int)),this,SLOT(readFromWpa(int)));
				delete m_eventSn;
				m_eventSn = NULL;
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
	int CWpaSupplicant::dynamicTimerTime(int m_timerCount) {
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
	void CWpaSupplicant::detachWpa() {
		if (event_ctrl != NULL) {
			wpa_ctrl_detach(event_ctrl);
		}
	}

	void CWpaSupplicant::timerEvent(QTimerEvent *event) {
		if (event->timerId() == m_connectTimerId) {
			if (!m_wpaConnected) {
				m_timerCount++;
				openWpa(true);
			} else {
				killTimer(m_connectTimerId);
				m_connectTimerId = -1;
				m_timerCount = 0;
			}
		}
	}

	bool CWpaSupplicant::connected() {
		if (wpaCtrlCmd_PING() == "PONG\n") {
			return true;
		}
		else {
			return false;
		}
	}

	void CWpaSupplicant::setLog(bool enabled) {
		m_logEnabled = enabled;
	}


	void CWpaSupplicant::scan() {
		if (0 != wpaCtrlCmd_SCAN().indexOf("OK")) {
			printMessage("Error while scanning");
		}
	}



}
