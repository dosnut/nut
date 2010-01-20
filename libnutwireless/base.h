#ifndef LIBNUTWIRELESS_BASE
#define LIBNUTWIRELESS_BASE
#include <QHostAddress>
#include <QFile>
#include <QSocketNotifier>
#include <QTimerEvent>
#include <QCoreApplication>
#include "libnutcommon/common.h"

#include "parsers.h"

#define CWPA_SCAN_TIMER_TIME 1000
#define CWPA_SCAN_RETRY_TIMER_TIME 1000

struct wpa_ctrl;

namespace libnutwireless {

	/** @brief The base class contains all timer/lowlevel io dependant functions
	
		This class provides all functions that need timers, socket notifiers etc.
		It is not used directly.
	*/
	class CWpaSupplicantBase: public QObject, public CWpaSupplicantParsers {
			Q_OBJECT
		protected:
			struct wpa_ctrl *cmd_ctrl, *event_ctrl;
			QString m_wpaSupplicantPath;
			int m_wpaFd;
			QSocketNotifier *m_eventSn;
			bool m_logEnabled;
			bool m_wpaConnected;
			int m_connectTimerId; ///Timer id for connection timer
			int m_timerCount;
			bool m_inConnectionPhase;
			QString m_ifname;

			
			//Workaround as Constructor of subclass is not beeing called
			int m_apScanDefault;
			bool m_lastWasAdHoc;
			//

			QString wpaCtrlCommand(QString cmd);
			//Abstracted Commands:
			inline QString wpaCtrlCmd_PING() { return wpaCtrlCommand("PING"); }
			inline QString wpaCtrlCmd_MIB() { return wpaCtrlCommand("MIB"); }
			inline QString wpaCtrlCmd_STATUS(bool verbose=false) { return (verbose) ? wpaCtrlCommand("STATUS-VERBOSE") : wpaCtrlCommand("STATUS"); }
			inline QString wpaCtrlCmd_PMKSA() { return wpaCtrlCommand("PMKSA"); }
			inline void wpaCtrlCmd_SET(QString var, QString val) { wpaCtrlCommand(QString("SET %1 %2").arg(var,val)); }
			inline void wpaCtrlCmd_LOGON() { wpaCtrlCommand("LOGON"); }
			inline void wpaCtrlCmd_LOGOFF() { wpaCtrlCommand("LOGOFF"); }
			inline void wpaCtrlCmd_REASSOCIATE() { wpaCtrlCommand("REASSOCIATE"); }
			//Start pre-authentication with the given BSSID.
			inline void wpaCtrlCmd_PREAUTH(QString bssid) { wpaCtrlCommand(QString("PREAUTH %1").arg(bssid)); }
			inline void wpaCtrlCmd_LEVEL(int level) { wpaCtrlCommand(QString("LEVEL %1").arg(QString::number(level))); }
			inline void wpaCtrlCmd_RECONFIGURE() { wpaCtrlCommand("RECONFIGURE"); };
			inline void wpaCtrlCmd_TERMINATE() { wpaCtrlCommand("TERMINATE"); }
			//Set preferred BSSID for a network. Network id can be received from the LIST_NETWORKS command output.
			inline QString wpaCtrlCmd_BSSID(int id, QString bssid) { return wpaCtrlCommand(QString("BSSID %1 %2").arg(QString::number(id),bssid));}
			inline QString wpaCtrlCmd_LIST_NETWORKS() { return wpaCtrlCommand("LIST_NETWORKS"); }
			inline void wpaCtrlCmd_DISCONNECT() { wpaCtrlCommand("DISCONNECT"); }
			inline QString wpaCtrlCmd_SCAN() { return wpaCtrlCommand("SCAN"); }
			inline QString wpaCtrlCmd_SCAN_RESULTS() { return wpaCtrlCommand("SCAN_RESULTS"); }
			inline QString wpaCtrlCmd_SELECT_NETWORK(int id) { return wpaCtrlCommand(QString("SELECT_NETWORK %1").arg(QString::number(id))); }
			inline QString wpaCtrlCmd_ENABLE_NETWORK(int id) { return wpaCtrlCommand(QString("ENABLE_NETWORK %1").arg(QString::number(id))); }
			inline QString wpaCtrlCmd_DISABLE_NETWORK(int id) { return wpaCtrlCommand(QString("DISABLE_NETWORK %1").arg(QString::number(id))); }
			//creates new empty network, return id on success and FAIL on failure
			inline QString wpaCtrlCmd_ADD_NETWORK() { return wpaCtrlCommand("ADD_NETWORK"); }
			inline void wpaCtrlCmd_REMOVE_NETWORK(int id) { wpaCtrlCommand(QString("REMOVE_NETWORK %1").arg(QString::number(id))); }
			inline QString wpaCtrlCmd_SET_NETWORK(int id, QString var, QString val) { return wpaCtrlCommand(QString("SET_NETWORK %1 %2 %3").arg(QString::number(id),var,val));}
			//get network variable
			inline QString wpaCtrlCmd_GET_NETWORK(int id, QString var) { return wpaCtrlCommand(QString("GET_NETWORK %1 %2").arg(QString::number(id), var)); }
			inline QString wpaCtrlCmd_SAVE_CONFIG() { return wpaCtrlCommand("SAVE_CONFIG"); }
			inline void wpaCtrlCmd_CTRL_RSP(QString field_name, int id, QString val) { wpaCtrlCommand(QString("CTRL-RSP-%1-%2-%3").arg(field_name,QString::number(id), val)); }
			inline QString wpaCtrlCmd_GET_CAPABILITY(QString option, bool strict) { return (strict) ? wpaCtrlCommand(QString("GET_CAPABILITY %1 strict").arg(option)) : wpaCtrlCommand(QString("GET_CAPABILITY %1").arg(option));}
			//Change ap_scan value: 0 = no scanning,
			//1 = wpa_supplicant requests scans and uses scan results to select the AP
			//2 = wpa_supplicant does not use scanning and just requests driver to associate and take care of AP selection
			inline QString wpaCtrlCmd_AP_SCAN(int val) { return wpaCtrlCommand(QString("AP_SCAN %1").arg(QString::number(val))); }
			inline QString wpaCtrlCmd_INTERFACES() { return wpaCtrlCommand("INTERFACES"); }
			
			//Event helper functions:
			void eventDispatcher(Request req);
			void eventDispatcher(EventType event, QString str);
			/** This function is called by readFromWpa
				when new information is available from wpa_supplicant
			*/
			void eventDispatcher(QString event);


			//Need to do it that way, as inline fails otherwise
			inline void printMessage(QString msg) { if (m_logEnabled) emit(message(msg));}

			/** Open connection to wpa_supplicant */
			void openWpa(bool time_call);
			/** close the connection to the wpa_supplicant */
			bool closeWpa(QString call_func, bool internal=true);
			/**Dynamically adjust the time between two timer events for polling the wpa_supplicant interface on connection try*/
			int dynamicTimerTime(int m_timerCount);

		protected slots:
			void readFromWpa(int socket);
			void detachWpa();
		protected:
			/**Function to dispatch the timer events */
			void timerEvent(QTimerEvent *event);
			
			//Set ap_scan
			virtual void setApScanDefault()=0;
			
		public:
			CWpaSupplicantBase(QObject * parent, QString m_ifname);
			~CWpaSupplicantBase();
			/** open connection to wpa_supplicant */
			inline void open() { openWpa(false); }
			/** close connection to wpa_supplicant */
			inline bool close() {return closeWpa("libnutclient",false); }
			bool connected();
			
			void scan();
			
		public slots:
			
			
			void setLog(bool enabled);
			
		signals:
			/** Signal which is emitted if wpa_supplicant connects or disconnects
				from a network
				@param state connection state of network
				@param id network id
			*/
			void connectionStateChanged(bool state, int id);
			/** Signal which is emitted if wpa_supplicant requests a action.
				Respond with CWpaSupplicant::response();
			*/
			void request(libnutwireless::Request req);
			/** This signal is emitted if the connection to wpa_supplicant is
				established: state=true or
				closed: state=false
			*/
			void stateChanged(bool state);
			/** This signal is emitted whenever a scan is ready.
				So far this happens twice:
				First the scanresults from wpa_supplicant are set imediately.
				Then we're trying to retrieve further information via wirelessExtension.
			*/

			/** Emits messages from wpa_supplicant like rekying */
			void message(QString msg);
			void eventMessage(libnutwireless::EventType type);

			/** This signal is emitted whenever the network list from listNetworks() has changed. */
			void networkListUpdated();
	};

}
#endif
