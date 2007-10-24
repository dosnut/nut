#ifndef LIBNUT_LIBNUT_WPA_SUPPLICANT_H
#define LIBNUT_LIBNUT_WPA_SUPPLICANT_H
// #define CONFIG_CTRL_IFACE
// #define CONFIG_CTRL_IFACE_UNIX
#include <iostream>
#include <QObject>
#include <QList>
#include <QHostAddress>
#include <common/types.h>
#include <common/config.h>
#include "libnut_server_proxy.h"
#include "libnut_exceptions.h"
#include "wpa_ctrl.h"
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QFile>
#include <QTextStream>
#include <QHash>
#include <QSocketNotifier>
#include <QString>
#include <QStringList>
#include <common/macaddress.h>

//TODO:Check if we can start multiple wpa_supplicants for multiple devices and test behavior



namespace libnut {
	typedef enum {WNF_NONE=0, WNF_CURRENT=1} wps_network_flags;
	typedef enum {CI_UNDEFINED=0, CI_NONE=1, CI_CCMP=2, CI_TKIP=4, CI_WEP104=8, CI_WEP40=16, CI_WEP=32} CIPHERS;
	typedef enum {KEYMGMT_PLAIN=0, KEYMGMT_WPA_PSK=2, KEYMGMT_WPA2_PSK=4, KEYMGMT_WPA_EAP=8, KEYMGMT_WPA2_EAP=16, KEYMGMT_IEEE8021X=32} KEYMGMT;
	struct wps_network {
		int id;
		QString ssid;
		nut::MacAddress bssid;
		wps_network_flags flags;
	};
	struct wps_network_config {
		
	};
	struct wps_scan {
		nut::MacAddress bssid;
		QString ssid;
		int freq;
		int level;
		CIPHERS ciphers;
		KEYMGMT key_mgmt;
	};
	struct wps_variable;
	typedef QList<wps_variable> wps_MIB;
	//enums are NOT complete, but maybe we schould change this to QString
	struct wps_status {
// 		typedef enum {COMPLETED} WPA_STATE;
// 		typedef enum {AUTHENTICATED} PAE_STATE;
// 		typedef enum {AUTHORIZED} PORT_STATUS;
// 		typedef enum {AUTO} PORT_CONTROL;
// 		typedef enum {IDLE} BACKEND_STATE;
// 		typedef enum {SUCCESS} EAP_STATE;
// 		typedef enum {NOSTATE} METHOD_STATE;
// 		typedef enum {COND_SUCC} DECISION;
		//These typedefs may change in the future to the ones above (more complete)
		typedef QString WPA_STATE;
		typedef QString PAE_STATE;
		typedef QString PORT_STATUS;
		typedef QString PORT_CONTROL;
		typedef QString BACKEND_STATE;
		typedef QString EAP_STATE;
		typedef QString METHOD_STATE;
		typedef QString DECISION;
		nut::MacAddress bssid;
		QString ssid;
		int id;
		CIPHERS pairwise_cipher;
		CIPHERS group_cipher;
		KEYMGMT key_mgmt;
		WPA_STATE wpa_state;
		QHostAddress ip_address;
		PAE_STATE pae_state;
		PORT_STATUS PortStatus;
		int heldPeriod;
		int authPeriod;
		int startPeriod;
		int maxStart;
		PORT_CONTROL portControl;
		BACKEND_STATE backend_state;
		EAP_STATE eap_state;
		int reqMethod;
		METHOD_STATE methodState;
		DECISION decision;
		int ClientTimeout;
	};

	struct wps_variable {
		typedef enum {PLAIN=1,STRING=2,NUMBER=4,LOGIC=8} wps_variable_type;
		wps_variable_type type;
		QString name;
		union {
			qint32 * num;
			QString * str;
			bool * logic;
		} value;
	};
	struct wps_net_var {
		typedef enum {PLAIN=1,STRING=2,NUMBER=4,LOGIC=8} Type;
		QString name;
		union {
			int * num;
			QString * str;
			bool * logic;
		} value;
	};
	typedef enum {WI_MSG=0, WI_REQ=1,WI_EVENT=2} wps_interact_type;
	typedef enum {WR_FAIL=0, WR_PASSWORD=2, WR_IDENTITY=4, WR_NEW_PASSWORD=8, WR_PIN=16, WR_OTP=32, WR_PASSPHRASE=64} wps_req_type;
	typedef enum {WE_OTHER=0, WE_DISCONNECTED=1, WE_CONNECTED=2, WE_TERMINATING=4} wps_event_type;
	struct wps_req {
		wps_req_type type;
		int id;
	};

	class CWpa_Supplicant: public QObject {
			Q_OBJECT
		private:
			struct wpa_ctrl *cmd_ctrl, *event_ctrl;
			QString wpa_supplicant_path;
			int wps_fd;
			QString wps_ctrl_command(QString cmd);
			QSocketNotifier *event_sn;
			bool log_enabled;
			bool wps_connected;
			
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
			inline void wps_cmd_SCAN() { wps_ctrl_command("SCAN"); }
			inline QString wps_cmd_SCAN_RESULTS() { return wps_ctrl_command("SCAN_RESULTS"); }
			inline void wps_cmd_SELECT_NETWORK(int id) { wps_ctrl_command(QString("SELECT_NETWORK %1").arg(QString::number(id))); }
			inline void wps_cmd_ENABLE_NETWORK(int id) { wps_ctrl_command(QString("ENABLE_NETWORK %1").arg(QString::number(id))); }
			inline void wps_cmd_DISABLE_NETWORK(int id) { wps_ctrl_command(QString("DISABLE_NETWORK %1").arg(QString::number(id))); }
			//creates new empty network, return id on success and FAIL on failure
			inline QString wps_cmd_ADD_NETWORK() { return wps_ctrl_command("ADD_NETWORK"); }
			inline void wps_cmd_SET_NETWORK(int id, QString var, QString val) { wps_ctrl_command(QString("SET_NETWORK %1 %2 %3").arg(QString::number(id),var,val));}
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
			wps_network_flags parseNetworkFlags(QString str);


			//parse scan results
			CIPHERS parseScanCiphers(QString str);
			KEYMGMT parseScanKeymgmt(QString str);
			QList<wps_scan> parseScanResult(QStringList list);

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
			void Event_dispatcher(wps_req request);
			void Event_dispatcher(wps_event_type event);
			void Event_dispatcher(QString event);

			inline void printMessage(QString msg);
		private slots:
			void wps_read(int socket);
			
		public:
			CWpa_Supplicant(QObject * parent, QString wpa_supplicant_path);
			~CWpa_Supplicant();
			bool wps_open();
			bool wps_close(bool available=true);
			bool connected();
	
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
			int addNetwork();
			void setBssid(int id, nut::MacAddress bssid);

			void setVariable(QString var, QString val);
			void setNetworkVariable(int id, QString var, QString val);
			QString getNetworkVariable(int id, QString val);

			QList<wps_network> listNetworks();
			QList<wps_scan> scanResults();
			wps_status status();

			//Future functions:
			/*
			QList<QStringList> getMIBVariables();
			getCapability(QString option, bool strict);
			QString wps_cmd_PMKSA();
			//Maybe variable/value as new wps_variable / wps_net_variable class
			void setVariable(wps_var var);
			void setNetworkVariable(int id, wps_net_var var);
			wps_net_var getNetworkVariable(int id, wps_net_var::Type);
*/


			
		signals:
			void opened();
			void closed();
			void wps_stateChange(bool state);
			void wps_request(wps_req request);
			void message(QString msg);
	};

}
#endif
