#ifndef LIBNUT_LIBNUT_WPA_SUPPLICANT_H
#define LIBNUT_LIBNUT_WPA_SUPPLICANT_H
#include "libnutwireless_base.h"


//TODO:Check if we can start multiple wpa_supplicants for multiple devices and test behavior


namespace libnutwireless {

	class CWpa_Supplicant: public CWpa_SupplicantBase {
			Q_OBJECT
		private:

			//Edit/get network helper functions
			EapNetworkConfig wps_getEapNetworkConfig(int id);
			EapNetconfigFailures wps_editEapNetwork(int netid, EapNetworkConfig config);
		public:
			CWpa_Supplicant(QObject * parent, QString ifname) : CWpa_SupplicantBase(parent, ifname) {}
			~CWpa_Supplicant() {}
			
		public slots:
			//Functions to react to request made from wpa_supplicant:
			void response(Request request, QString msg);
			//
			void selectNetwork(int id);
			void enableNetwork(int id);
			void disableNetwork(int id);
			void ap_scan(int type=1);
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
			NetconfigStatus addNetwork(NetworkConfig config); //return -1 if failed, otherwise return network id
			NetconfigStatus editNetwork(int netid, NetworkConfig config);
			NetworkConfig getNetworkConfig(int id);
			
			void removeNetwork(int id);
			void setBssid(int id, libnutcommon::MacAddress bssid);

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
			QString wps_cmd_PMKSA();
			//Maybe variable/value as new MIBVariable / NetworkVariableiable class
			void setVariable(wps_var var);
			void setNetworkVariable(int id, NetworkVariable var);
			NetworkVariable getNetworkVariable(int id, NetworkVariable::Type);
*/
	};

}
#endif
