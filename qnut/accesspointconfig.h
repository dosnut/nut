//
// C++ Interface: accesspointconfig
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_ACCESSPOINTCONFIG_H
#define QNUT_ACCESSPOINTCONFIG_H

#ifndef QNUT_NO_WIRELESS
#include <QDialog>
#include <libnutclient/client.h>
#include "ui/ui_apconf.h"

namespace qnut {
	/**
	 * @brief CAccessPointConfig provides a dialog to configure a managed nework or add a new network for the given wpa_supplicant.
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 * 
	 * On creation, the CAccessPointConfig sets up the user interface according to the given instance of a wpa_supplicant.
	 * The class provides function to execute the dialog for adding (w/o scan result) and configuring a network (by network id).
	 *
	 * By accepting the settings made in the UI they are verified (on error the dialog stays open).
	 */
	class CAccessPointConfig : public QDialog {
		Q_OBJECT
	private:
		Ui::apconf ui;
		QRegExpValidator * m_HexValidator;
		
		libnutwireless::CWpaSupplicant * m_Supplicant;
		
		int m_CurrentID;
		bool m_WEPEnabled;
		
		struct {
			libnutwireless::GroupCiphers group;
			libnutwireless::PairwiseCiphers pairwise;
			libnutwireless::Protocols protocols;
		} m_OldConfig;
		
		inline void convertLineEditText(QLineEdit * lineEdit, bool hex);
		inline QString convertQuoted(QString text);
		inline void writeEAPConfig(libnutwireless::EapNetworkConfig &eap_config);
		inline void readEAPConfig(libnutwireless::EapNetworkConfig &eap_config);
	public:
		/**
		 * @brief Opens the dialog for adding the given scanned network.
		 * @param scanResult scan result with network configuration to use
		 */
		bool execute(libnutwireless::ScanResult scanResult);
		/**
		 * @brief Opens the dialog for configuring the given managed network
		 * @param id managed network id
		 */
		bool execute(int id);
		/// @brief Opens the dialog for adding a new annonymous network
		bool execute();
		
		/**
		 * @brief Creates the object and initializes the basic user interface.
		 * @param parent parent widget
		 */
		CAccessPointConfig(libnutwireless::CWpaSupplicant * wpa_supplicant, QWidget * parent = 0);
		/// @brief Destroyes the object.
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

#endif
