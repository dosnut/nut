#ifndef LIBNUTWIRELESS_WPA_SUPPLICANT_H
#define LIBNUTWIRELESS_WPA_SUPPLICANT_H
#include "base.h"
#include <QDebug>


//TODO:Check if we can start multiple wpa_supplicants for multiple devices and test behavior


namespace libnutwireless {

	class CWpa_Supplicant: public CWpa_SupplicantBase {
			Q_OBJECT
		private:
			QList<quint8> supportedChannels;

			//Edit/get network helper functions
			EapNetworkConfig wps_getEapNetworkConfig(int id);
			EapNetconfigFailures wps_editEapNetwork(int netid, EapNetworkConfig config);
			NetconfigStatus checkAdHocNetwork(NetworkConfig &config);
		public:
			//TODO: Check why constructor is not beeing called
			CWpa_Supplicant(QObject * parent, QString ifname) : CWpa_SupplicantBase(parent, ifname) {
				apScanDefault = -1;
				qDebug() << (QString("Constructor set ap_scan=%1").arg(QString::number(apScanDefault)));
				lastWasAdHoc = false;
				qDebug() << (QString("Constructor set lastWasAdHoc=%1").arg((lastWasAdHoc) ? "true" : "false"));
			}
			~CWpa_Supplicant() {}
			QList<quint8>& getSupportedChannels();
			
		public slots:
			//Functions to react to request made from wpa_supplicant:
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
		signals:
			void networkListUpdated();
	};

}
#endif
