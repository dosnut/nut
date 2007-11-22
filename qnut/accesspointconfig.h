//
// C++ Interface: accesspointconfig
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QNUT_ACCESSPOINTCONFIG_H
#define QNUT_ACCESSPOINTCONFIG_H

#include <QDialog>
#include <libnutclient/client.h>
#include "ui/ui_apconf.h"

namespace qnut {
	class CAccessPointConfig : public QDialog {
		Q_OBJECT
	private:
		Ui::apconf ui;
		
		QRegExpValidator * hexValidator;
		
		libnutwireless::CWpa_Supplicant * supplicant;
		
		int currentID;
		
		bool wepEnabled;
		struct {
			libnutwireless::GroupCiphers group;
			libnutwireless::PairwiseCiphers pairwise;
			libnutwireless::Protocols protocols;
		} oldConfig;
		
		inline void convertLineEditText(QLineEdit * lineEdit, bool hex);
		inline QString convertQuoted(QString text);
		inline void writeEAPConfig(libnutwireless::EapNetworkConfig &eap_config);
		inline void readEAPConfig(libnutwireless::EapNetworkConfig &eap_config);
	public:
		bool execute(libnutwireless::ScanResult scanResult);
		bool execute(int id);
		bool execute();
		
		CAccessPointConfig(libnutwireless::CWpa_Supplicant * wpa_supplicant, QWidget * parent = 0);
		~CAccessPointConfig();
	private slots:
		void setAuthConfig(int type);
		void setEncConfig(QString value);
		void setWEPDisabled(bool value);
		void verifyConfiguration();
		void countPskChars(QString psk);
		void togglePlainPSK(bool show);
		void convertSSID(bool hex);
		void convertWEPKey0(bool hex);
		void convertWEPKey1(bool hex);
		void convertWEPKey2(bool hex);
		void convertWEPKey3(bool hex);
		
		void selectCAFile();
		void selectClientFile();
		void selectKeyFile();
	};
}

#endif
