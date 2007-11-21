#ifndef LIBNUTWIRELESS_BASE
#define LIBNUTWIRELESS_BASE
#include <QHostAddress>
#include <QFile>
#include <QSocketNotifier>
#include <QTimerEvent>
#include <QCoreApplication>
#include "libnutcommon/common.h"
#include "wpa_ctrl/wpa_ctrl.h"
#include "parsers.h"

#include <iwlib.h>
extern "C" {
// #include <linux/wireless.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
}

#define CWPA_SCAN_TIMER_TIME 1000
#define CWPA_SCAN_RETRY_TIMER_TIME 1000

namespace libnutwireless {

	//This class provides all functions that need very "low-level" (like timers) functions to
	//communicate with the wpa_supplicant
	class CWpa_SupplicantBase: public QObject, public CWpa_SupplicantParsers {
			Q_OBJECT
		protected:
			struct wpa_ctrl *cmd_ctrl, *event_ctrl;
			QString wpa_supplicant_path;
			int wps_fd, wext_fd;
			QSocketNotifier *event_sn;
			bool log_enabled;
			bool wps_connected;
			int timerId;
			int wextTimerId;
			int ScanTimerId;
			int wextTimerRate;
			int timerCount;
			bool inConnectionPhase;
			QString ifname;
			QList<ScanResult> wpsScanResults;
			WextSignal signalQuality; //bssid is zero
			int ScanTimeoutCount;
			int wextPollTimeoutCount;
			QList<quint32> supportedFrequencies;
			
			//Workaround as Constructor of subclass is not beeing called
			int apScanDefault;
			bool lastWasAdHoc;
			//

			QString wps_ctrl_command(QString cmd);
		//Abstracted Commands:
			inline QString wps_cmd_PING() { return wps_ctrl_command("PING"); }
			inline QString wps_cmd_MIB() { return wps_ctrl_command("MIB"); }
			inline QString wps_cmd_STATUS(bool verbose=false) { return (verbose) ? wps_ctrl_command("STATUS-VERBOSE") : wps_ctrl_command("STATUS"); }
			inline QString wps_cmd_PMKSA() { return wps_ctrl_command("PMKSA"); }
			inline void wps_cmd_SET(QString var, QString val) { wps_ctrl_command(QString("SET %1 %2").arg(var,val)); }
			inline void wps_cmd_LOGON() { wps_ctrl_command("LOGON"); }
			inline void wps_cmd_LOGOFF() { wps_ctrl_command("LOGOFF"); }
			inline void wps_cmd_REASSOCIATE() { wps_ctrl_command("REASSOCIATE"); }
			//Start pre-authentication with the given BSSID.
			inline void wps_cmd_PREAUTH(QString bssid) { wps_ctrl_command(QString("PREAUTH %1").arg(bssid)); }
			inline void wps_cmd_LEVEL(int level) { wps_ctrl_command(QString("LEVEL %1").arg(QString::number(level))); }
			inline void wps_cmd_RECONFIGURE() { wps_ctrl_command("RECONFIGURE"); };
			inline void wps_cmd_TERMINATE() { wps_ctrl_command("TERMINATE"); }
			//Set preferred BSSID for a network. Network id can be received from the LIST_NETWORKS command output.
			inline void wps_cmd_BSSID(int id, QString bssid) { wps_ctrl_command(QString("BSSID %1 %2").arg(QString::number(id),bssid));}
			inline QString wps_cmd_LIST_NETWORKS() { return wps_ctrl_command("LIST_NETWORKS"); }
			inline void wps_cmd_DISCONNECT() { wps_ctrl_command("DISCONNECT"); }
			inline QString wps_cmd_SCAN() { return wps_ctrl_command("SCAN"); }
			inline QString wps_cmd_SCAN_RESULTS() { return wps_ctrl_command("SCAN_RESULTS"); }
			inline QString wps_cmd_SELECT_NETWORK(int id) { return wps_ctrl_command(QString("SELECT_NETWORK %1").arg(QString::number(id))); }
			inline QString wps_cmd_ENABLE_NETWORK(int id) { return wps_ctrl_command(QString("ENABLE_NETWORK %1").arg(QString::number(id))); }
			inline QString wps_cmd_DISABLE_NETWORK(int id) { return wps_ctrl_command(QString("DISABLE_NETWORK %1").arg(QString::number(id))); }
			//creates new empty network, return id on success and FAIL on failure
			inline QString wps_cmd_ADD_NETWORK() { return wps_ctrl_command("ADD_NETWORK"); }
			inline void wps_cmd_REMOVE_NETWORK(int id) { wps_ctrl_command(QString("REMOVE_NETWORK %1").arg(QString::number(id))); }
			inline QString wps_cmd_SET_NETWORK(int id, QString var, QString val) { return wps_ctrl_command(QString("SET_NETWORK %1 %2 %3").arg(QString::number(id),var,val));}
			//get network variable
			inline QString wps_cmd_GET_NETWORK(int id, QString var) { return wps_ctrl_command(QString("GET_NETWORK %1 %2").arg(QString::number(id), var)); }
			inline QString wps_cmd_SAVE_CONFIG() { return wps_ctrl_command("SAVE_CONFIG"); }
			inline void wps_cmd_CTRL_RSP(QString field_name, int id, QString val) { wps_ctrl_command(QString("CTRL-RSP-%1-%2-%3").arg(field_name,QString::number(id), val)); }
			inline QString wps_cmd_GET_CAPABILITY(QString option, bool strict) { return (strict) ? wps_ctrl_command(QString("GET_CAPABILITY %1 strict").arg(option)) : wps_ctrl_command(QString("GET_CAPABILITY %1").arg(option));}
			//Change ap_scan value: 0 = no scanning,
			//1 = wpa_supplicant requests scans and uses scan results to select the AP
			//2 = wpa_supplicant does not use scanning and just requests driver to associate and take care of AP selection
			inline QString wps_cmd_AP_SCAN(int val) { return wps_ctrl_command(QString("AP_SCAN %1").arg(QString::number(val))); }
			inline QString wps_cmd_INTERFACES() { return wps_ctrl_command("INTERFACES"); }
			
			//Parser Functions
			//Event helper functions:
			void Event_dispatcher(Request req);
			void Event_dispatcher(EventType event);
			void Event_dispatcher(QString event);

			//Functions to get actual signal strength and/or signal strength for scan results:
			//And set scanresults
			void wps_setScanResults(QList<WextRawScan> wextScanResults);
			void wps_tryScanResults();

			//Need to do it that way, as inline fails otherwise
			inline void printMessage(QString msg) { if (log_enabled) emit(message(msg));}

			void wps_open(bool time_call);
			bool wps_close(QString call_func, bool internal=true);
			int wps_TimerTime(int timerCount);

		protected slots:
			void wps_read(int socket);
			void wps_detach();
		protected:
			//proposed time polling:
			void timerEvent(QTimerEvent *event);
			
		public:
			CWpa_SupplicantBase(QObject * parent, QString ifname);
			~CWpa_SupplicantBase();
			inline void open() { wps_open(false); }
			inline bool close() {return wps_close("libnutclient",false); }
			bool connected();
			void readWirelessInfo();
			
		public slots:
			void setLog(bool enabled);
			//Functions to react to request made from wpa_supplicant:
			void scan();
			
			void setSignalQualityPollRate(int msec);
			int getSignalQualityPollRate();
			WextSignal getSignalQuality();

			QList<ScanResult> scanResults();
			
		signals:
			void stateChanged(bool state);
			void request(libnutwireless::Request req);
			void closed();
			void opened();
			void scanCompleted();
			void message(QString msg);
			void eventMessage(libnutwireless::EventType type);
			void signalQualityUpdated(libnutwireless::WextSignal signal);
	};

}
#endif
