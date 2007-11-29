#ifndef LIBNUTWIRELESS_WPA_SUPPLICANT_H
#define LIBNUTWIRELESS_WPA_SUPPLICANT_H
#include "base.h"
#include <QDebug>



namespace libnutwireless {

	/** @brief CWpa_Supplicant is the main class for communicatin with wpa_supplicant

		It provides all necessary functions to communicate with wpa_supplicant.
		
		To open the wpa_supplicant interface, open() has to be called.
	*/
	class CWpa_Supplicant: public CWpa_SupplicantBase {
			Q_OBJECT
		private:
			QList<quint8> m_supportedChannels;

			//Edit/get network helper functions
			EapNetworkConfig getEapNetworkConfig(int id);
			EapNetconfigFailures editEapNetwork(int netid, EapNetworkConfig config);
			NetconfigStatus checkAdHocNetwork(NetworkConfig &config);
		public:
			//TODO: Check why constructor is not beeing called
			
			/** 
			 * 
			 * @param m_ifname interface name
			 * The interface's socket has to be at /var/run/wpa_supplicant/ifname_name
			 */
			CWpa_Supplicant(QObject * parent, QString m_ifname) : CWpa_SupplicantBase(parent, m_ifname) {
				m_apScanDefault = -1;
				qDebug() << (QString("Constructor set ap_scan=%1").arg(QString::number(m_apScanDefault)));
				m_lastWasAdHoc = false;
				qDebug() << (QString("Constructor set m_lastWasAdHoc=%1").arg((m_lastWasAdHoc) ? "true" : "false"));
			}
			~CWpa_Supplicant() {}
			QList<quint8>& getSupportedChannels();
			
		public slots:
			/**
				Function to react to request made from wpa_supplicant:
				@param request Type of request
				@param msg the value that will be passed to wpa_supplicant
			*/
			void response(Request request, QString msg);
			//
			bool selectNetwork(int id);
			bool enableNetwork(int id);
			bool disableNetwork(int id);
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
