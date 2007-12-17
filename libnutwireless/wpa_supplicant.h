//Features not implemented yet:
//Retrieving of bit-rate
//Signal from server when a new network was found
//Fallback config in server


#ifndef LIBNUTWIRELESS_WPA_SUPPLICANT_H
#define LIBNUTWIRELESS_WPA_SUPPLICANT_H

#ifndef LIBNUT_NO_WIRELESS
#include "base.h"
#include <QDebug>



namespace libnutwireless {

	/** @brief CWpaSupplicant is the main class for communicatin with wpa_supplicant

		It provides all necessary functions to communicate with wpa_supplicant.
		
		To open the wpa_supplicant interface, open() has to be called.
		To close it, close() has to be called.
		Wpa_supplicant connection changes are emitted by stateChanged(bool state).
		Network connection changes are emitted by connectionStateChanged

		We try to do all tasks automatically as far as possible.
		On instantiation we set apScanDefault to 1. If you want to change this, call ap_scan with your desired value.
		If the current network is an adhoc network, we assume the actual ap_scan value to be 2.
		Otherwise we assume ap_scan=1.
		When opening wpa_supplicant we check if the current network is an adhoc network.
		If you later change into a normal network ap_scan will be set to apScanDefault.
	*/
	class CWpaSupplicant: public CWpaSupplicantBase {
			Q_OBJECT
		private:
			QList<quint8> m_supportedChannels;

			//Edit/get network helper functions
			EapNetworkConfig getEapNetworkConfig(int id);
			EapNetconfigFailures editEapNetwork(int netid, EapNetworkConfig config);
			/** This function is used to check if an adhoc network is configured properly.
				It checks for plaintext,wep and wpa networks.
			**/
			NetconfigStatus checkAdHocNetwork(NetworkConfig &config);
			
			//Set ap_scan defaults
			void setApScanDefault();
			
		public:
			//TODO: Check why constructor is not beeing called
			
			/** 
			 * 
			 * @param m_ifname interface name
			 * The interface's socket has to be at /var/run/wpa_supplicant/ifname_name
			 */
			CWpaSupplicant(QObject * parent, QString m_ifname) : CWpaSupplicantBase(parent, m_ifname) {
				m_apScanDefault = -1;
				qDebug() << (QString("Constructor set ap_scan=%1").arg(QString::number(m_apScanDefault)));
				m_lastWasAdHoc = false;
				qDebug() << (QString("Constructor set m_lastWasAdHoc=%1").arg((m_lastWasAdHoc) ? "true" : "false"));
				
			}
			~CWpaSupplicant() {}
			QList<quint8>& getSupportedChannels();
			
		public slots:
			/**
				Function to react to request made from wpa_supplicant:
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
			NetconfigStatus addNetwork(NetworkConfig config);
			/**
				Function to edit a Network. Errors will be written in NetconfigStatus
				EAP-Networks are automatically detected.
				Undefined values in Networkconfig will not be set.
			*/
			NetconfigStatus editNetwork(int netid, NetworkConfig config);
			NetworkConfig getNetworkConfig(int id);
			
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
			//Future functions: (these may never be implemented as noone realy needs them
			/*
			QString wpaCtrlCmd_PMKSA();
			//Maybe variable/value as new MIBVariable / NetworkVariableiable class
			void setVariable(NetworkVariable var);
			void setNetworkVariable(int id, NetworkVariable var);
			NetworkVariable getNetworkVariable(int id, NetworkVariable::Type);
*/
	};

}
#endif
#endif
