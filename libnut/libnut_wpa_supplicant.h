#ifndef LIBNUT_LIBNUT_WPA_SUPPLICANT_H
#define LIBNUT_LIBNUT_WPA_SUPPLICANT_H
#include <QHostAddress>
#include <common/types.h>
#include <common/config.h>
#include "libnut_server_proxy.h"
#include "libnut_exceptions.h"
#include "libnut_wpa_supplicant_types.h"
#include "wpa_ctrl.h"
#include <QFile>
#include <QSocketNotifier>
#include <QStringList>
#include <QTimerEvent>
#include <common/macaddress.h>
#include <QCoreApplication>
#define CWPA_SCAN_TIMER_TIME 1000
#define CWPA_SCAN_RETRY_TIMER_TIME 1000

#include <iwlib.h>
extern "C" {
// #include <linux/wireless.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
}

//TODO:Check if we can start multiple wpa_supplicants for multiple devices and test behavior


namespace libnutws {

	class CWpa_Supplicant: public QObject {
			Q_OBJECT
		private:
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
			QList<wps_scan> wpsScanResults;
			wps_wext_signal_readable signalQuality; //bssid is zero
			int ScanTimeoutCount;
			int wextPollTimeoutCount;

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
			inline void wps_cmd_SELECT_NETWORK(int id) { wps_ctrl_command(QString("SELECT_NETWORK %1").arg(QString::number(id))); }
			inline void wps_cmd_ENABLE_NETWORK(int id) { wps_ctrl_command(QString("ENABLE_NETWORK %1").arg(QString::number(id))); }
			inline void wps_cmd_DISABLE_NETWORK(int id) { wps_ctrl_command(QString("DISABLE_NETWORK %1").arg(QString::number(id))); }
			//creates new empty network, return id on success and FAIL on failure
			inline QString wps_cmd_ADD_NETWORK() { return wps_ctrl_command("ADD_NETWORK"); }
			inline void wps_cmd_REMOVE_NETWORK(int id) { wps_ctrl_command(QString("REMOVE_NETWORK %1").arg(QString::number(id))); }
			inline QString wps_cmd_SET_NETWORK(int id, QString var, QString val) { return wps_ctrl_command(QString("SET_NETWORK %1 %2 %3").arg(QString::number(id),var,val));}
			//get network variable
			inline QString wps_cmd_GET_NETWORK(int id, QString var) { return wps_ctrl_command(QString("GET_NETWORK %1 %2").arg(QString::number(id), var)); }
			inline void wps_cmd_SAVE_CONFIG() { wps_ctrl_command("SAVE_CONFIG"); }
			inline void wps_cmd_CTRL_RSP(QString field_name, int id, QString val) { wps_ctrl_command(QString("CTRL-RSP-%1-%2-%3").arg(field_name,QString::number(id), val)); }
			inline QString wps_cmd_GET_CAPABILITY(QString option, bool strict) { return (strict) ? wps_ctrl_command(QString("GET_CAPABILITY %1 strict").arg(option)) : wps_ctrl_command(QString("GET_CAPABILITY %1").arg(option));}
			//Change ap_scan value: 0 = no scanning,
			//1 = wpa_supplicant requests scans and uses scan results to select the AP
			//2 = wpa_supplicant does not use scanning and just requests driver to associate and take care of AP selection
			inline void wps_cmd_AP_SCAN(int val) { wps_ctrl_command(QString("AP_SCAN %1").arg(QString::number(val))); }
			inline QString wps_cmd_INTERFACES() { return wps_ctrl_command("INTERFACES"); }
			
			//Parser Functions

			
			QStringList sliceMessage(QString str);
			
			//Parse MIB Variables
			wps_MIB parseMIB(QStringList list);
			wps_variable::wps_variable_type parseMIBType(QString str);
			
			//parse list network
			QList<wps_network> parseListNetwork(QStringList list);
			NetworkFlags parseNetworkFlags(QString str);


			//parse scan results
			wps_ciphers parseScanCiphers(QString str);
			wps_authentication parseScanAuth(QString str);
			QList<wps_scan> parseScanResult(QStringList list);

			//parse config
			wps_protocols parseProtocols(QString str);
			wps_key_management parseKeyMgmt(QString str);
			wps_auth_algs parseAuthAlg(QString str);
			wps_pairwise_ciphers parsePairwiseCiphers(QString str);
			wps_group_ciphers parseGroupCiphers(QString str);
			wps_eapol_flags parseEapolFlags(QString str);
			wps_eap_method parseEapMethod(QString str);
			
			

			//parse Status with helper functionss
			wps_status parseStatus(QStringList list);
			wps_status::WPA_STATE parseWpaState(QString str);
			wps_status::PAE_STATE parsePaeState(QString str);
			wps_status::PORT_STATUS parsePortStatus(QString str);
			wps_status::PORT_CONTROL parsePortControl(QString str);
			wps_status::BACKEND_STATE parseBackendState(QString str);
			wps_status::EAP_STATE parseEapState(QString str);
			wps_status::METHOD_STATE parseMethodState(QString str);
			wps_status::DECISION parseDecision(QString str);

			
			//parse Event
			wps_event_type parseEvent(QString str);
			wps_req parseReq(QString str);
			wps_req_type parseReqType(QString str);
			wps_interact_type parseInteract(QString str);

			//Event helper functions:
			void Event_dispatcher(wps_req req);
			void Event_dispatcher(wps_event_type event);
			void Event_dispatcher(QString event);


			//Edit/get network helper functions
			wps_eap_network_config wps_getEapNetworkConfig(int id);
			wps_eap_netconfig_failures wps_editEapNetwork(int netid, wps_eap_network_config config);

			//Functions to get actual signal strength and/or signal strength for scan results:
			//And set scanresults
			void wps_setScanResults(QList<wps_wext_raw_scan> &wextScanResults);
			void wps_tryScanResults();


			inline void printMessage(QString msg);

			void wps_open(bool time_call);
			bool wps_close(QString call_func, bool internal=true);
			int wps_TimerTime(int timerCount);


		private slots:
			void wps_read(int socket);
			void wps_detach();
		protected:
			//proposed time polling:
			void timerEvent(QTimerEvent *event);
			
		public:
			CWpa_Supplicant(QObject * parent, QString ifname);
			~CWpa_Supplicant();
			inline void open() { wps_open(false); }
			inline bool close() {return wps_close("libnut",false); }
			bool connected();
			void readWirelessInfo();
			
		public slots:
			void setLog(bool enabled);
			//Functions to react to request made from wpa_supplicant:
			void response(wps_req request, QString msg);
			//
			void selectNetwork(int id);
			void enableNetwork(int id);
			void disableNetwork(int id);
			void scan();
			void ap_scan(int type=1);
			void save_config();
			void disconnect_device();
			void logon();
			void logoff();
			void reassociate();
			void debug_level(int level);
			void reconfigure();
			void terminate();
			void preauth(nut::MacAddress bssid);
			int addNetwork(); //return -1 if failed, otherwise return network id
			wps_netconfig_status addNetwork(wps_network_config config); //return -1 if failed, otherwise return network id
			wps_netconfig_status editNetwork(int netid, wps_network_config config);
			wps_network_config getNetworkConfig(int id);
			
			void removeNetwork(int id);
			void setBssid(int id, nut::MacAddress bssid);

			void setVariable(QString var, QString val);
			bool setNetworkVariable(int id, QString var, QString val);
			QString getNetworkVariable(int id, QString val);

			void setSignalQualityPollRate(int msec);
			int getSignalQualityPollRate();
			wps_wext_signal_readable getSignalQuality();

			QList<wps_network> listNetworks();
			QList<wps_scan> scanResults();
			wps_status status();
			
			//Seldomly used functions
			wps_MIB getMIBVariables();
			wps_capabilities getCapabilities();
			//Future functions: (these may never be implemented as noone realy needs them
			/*
			QString wps_cmd_PMKSA();
			//Maybe variable/value as new wps_variable / wps_net_variable class
			void setVariable(wps_var var);
			void setNetworkVariable(int id, wps_net_var var);
			wps_net_var getNetworkVariable(int id, wps_net_var::Type);
*/


			
		signals:
			void stateChanged(bool state);
			void request(libnutws::wps_req req);
			void closed();
			void opened();
			void scanCompleted();
			void message(QString msg);
			void eventMessage(libnutws::wps_event_type type);
			void signalQualityUpdated(libnutws::wps_wext_signal_readable signal);
	};

}
#endif
