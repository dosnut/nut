#ifndef LIBNUTWIRELESS_WPA_SUPPLICANT_H
#define LIBNUTWIRELESS_WPA_SUPPLICANT_H

#pragma once

//Features not implemented yet:
//Signal from server when a new network was found

#ifndef NUT_NO_WIRELESS
#include <libnutcommon/common.h>

#include "cnetworkconfig.h"
#include "wstypes.h"

#include <QHostAddress>
#include <QSocketNotifier>
#include <QTimerEvent>
#include <QCoreApplication>

extern "C" {
struct wpa_ctrl;
}

namespace libnutwireless {
	/** @brief CWpaSupplicant is the main class for communicatin with wpa_supplicant

		It provides all necessary functions to communicate with wpa_supplicant.

		To open the wpa_supplicant interface, open() has to be called.
		To close it, close() has to be called.
		We try to do all tasks automatically as far as possible.
		On instantiation we set apScanDefault to 1. If you want to change this, call ap_scan with your desired value.
		If the current network is an adhoc network, we assume the actual ap_scan value to be 2.
		Otherwise we assume ap_scan=1.
		When opening wpa_supplicant we check if the current network is an adhoc network.
		If you later change into a normal network ap_scan will be set to apScanDefault.
	*/
	class CWpaSupplicant final: public QObject {
		Q_OBJECT
	public:
		/**
		 *
		 * @param m_ifname interface name
		 * The interface's socket has to be at /var/run/wpa_supplicant/ifname_name
		 */
		explicit CWpaSupplicant(QObject* parent, QString const& ifname);
		~CWpaSupplicant();

		static QByteArray parseConfigString(QString const& str);

		/** Get the Hash of the managed network configs */
		QHash<CNetworkConfig::NetworkId, CNetworkConfig> const& getManagedConfigs() const { return m_managedNetworks; }

		/** open connection to wpa_supplicant */
		void open() { openWpa(false); }
		/** close connection to wpa_supplicant */
		bool close() { return closeWpa("libnutclient", false); }
		bool connected();

		void scan();

	public slots:
		/** set whether log should be enabled */
		void setLog(bool enabled);

		/**
			Function to react to a request made from wpa_supplicant:
			@param request Type of request
			@param msg the value that will be passed to wpa_supplicant
		*/
		void response(Request request, QString msg);

		/** Select a configured network
			ap_scan will be set automatically:
			ap_scan=1 (or your default) for non-adhoc
			ap_scan=2 for adhoc
			It'll be set, if we select a network that needs a different ap_scan value.
		*/
		bool selectNetwork(int id);
		bool enableNetwork(int id);
		bool disableNetwork(int id);

		/** Set the ap_scan value.
			If it's 0 or 1 the value will be used as default ap_scan value.
			Normally you do not have to call the ap_scan function.
			See selectNetwork(int id) and CWpaSupplicant.
		*/
		bool ap_scan(int type=1);

		bool save_config();
		void disconnect_device();
		void logon();
		void logoff();
		void reassociate();
		void debug_level(int level);
		void reconfigure();
		void terminate();
		void preauth(libnutcommon::MacAddress bssid);
		int addNetwork(); //return -1 if failed, otherwise return network id

		/**
			Function to add a Network. Errors will be written in NetconfigStatus
			EAP-Networks are automatically detected.
			Undefined values in Networkconfig will not be set.
		*/
		NetconfigStatus addNetwork(CNetworkConfig config);

		/** Same as above, but check if network is already managed (checks id_str) **/
		NetconfigStatus addOnlyNewNetwork(CNetworkConfig config);

		/** Function to add multiple networks. Same as above
		*/
		QList<NetconfigStatus> addNetworks(QList<CNetworkConfig> configs);

		/** Same as above, but check if network is already managed (checks id_str) **/
		QList<NetconfigStatus> addOnlyNewNetworks(QList<CNetworkConfig> configs);


		/** Function to add networks read from a stream */
		QList<NetconfigStatus> addNetworks(QTextStream * stream);
		/**
			Function to edit a Network. Errors will be written in NetconfigStatus
			EAP-Networks are automatically detected.
			Undefined values in Networkconfig will not be set.
			If you edit a managed network, the managed copy will be updated automatically.
			Just don't touch the id_str
		*/

		/** Same as above, but check if network is already managed (checks id_str) **/
		QList<NetconfigStatus> addOnlyNewNetworks(QTextStream * stream);

		NetconfigStatus editNetwork(int netid, CNetworkConfig config);
		CNetworkConfig getNetworkConfig(int id);

		void removeNetwork(int id);
		bool setBssid(int id, libnutcommon::MacAddress bssid);

		void setVariable(QString var, QString val);
		bool setNetworkVariable(int id, QString var, QString val);
		QString getNetworkVariable(int id, QString val);

		QList<ShortNetworkInfo> listNetworks();
		Status status();

		//Seldomly used functions
		MIBVariables getMIBVariables();
		Capabilities getCapabilities();

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

	private: //Private Functions:
		QString wpaCtrlCommand(QString const& cmd);

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

		/** This function is used to check if an adhoc network is configured properly.
			It checks for plaintext,wep and wpa networks.
		**/
		NetconfigStatus checkAdHocNetwork(CNetworkConfig &config);

		//Set ap_scan defaults
		void setApScanDefault();

		/**Function to dispatch the timer events */
		void timerEvent(QTimerEvent *event) override;

	private: //abstracted low-level wpa_supplicant functions
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

	private: //Parser functions

		///Splits a message at newlines
		static QStringList sliceMessage(QString str);

		//Parse MIB Variables
		static MIBVariables parseMIB(QStringList list);
		static MIBVariable::MIBVariable_type parseMIBType(QString str);

		//parse list network
		static QList<ShortNetworkInfo> parseListNetwork(QStringList list);
		static NetworkFlags parseNetworkFlags(QString str);

		//parse Status with helper functionss
		static Status parseStatus(QStringList list);
		static Status::WPA_STATE parseWpaState(QString str);
		static Status::PAE_STATE parsePaeState(QString str);
		static Status::PORT_STATUS parsePortStatus(QString str);
		static Status::PORT_CONTROL parsePortControl(QString str);
		static Status::BACKEND_STATE parseBackendState(QString str);
		static Status::EAP_STATE parseEapState(QString str);
		static Status::METHOD_STATE parseMethodState(QString str);
		static Status::DECISION parseDecision(QString str);


		//parse Event
		static EventType parseEvent(QString str);
		static Request parseReq(QString str);
		static RequestType parseReqType(QString str);
		static InteractiveType parseInteract(QString str);
		static int parseEventNetworkId(QString str);

	private slots:
		void readFromWpa(int socket);
		void detachWpa();

	private: //Variables:
		QHash<CNetworkConfig::NetworkId,CNetworkConfig> m_managedNetworks;
		::wpa_ctrl* cmd_ctrl{nullptr};
		::wpa_ctrl* event_ctrl{nullptr};
		QString m_wpaSupplicantPath;
		int m_wpaFd{-1};
		QSocketNotifier* m_eventSn{nullptr};
		bool m_logEnabled{true};
		bool m_wpaConnected{false};
		QBasicTimer m_connectTimer;
		int m_timerCount{0};
		bool m_inConnectionPhase;
		QString m_ifname;

		int m_apScanDefault{-1};
		bool m_lastWasAdHoc{false};
	};
}
#endif
#endif /* LIBNUTWIRELESS_WPA_SUPPLICANT_H */
