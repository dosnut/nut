//
// C++ Interface: adhocconfig
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_ADHOCCONFIG_H
#define QNUT_ADHOCCONFIG_H

#ifndef QNUT_NO_WIRELESS
#include <QDialog>
#include <libnutclient/client.h>
#include "ui/ui_adhoc.h"

namespace qnut {
	/**
	 * @brief CAdhocConfig provides a dialog to configure a managed ad-hoc nework or add a new ad-hoc network for the given wpa_supplicant.
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 * 
	 * On creation, the CAdhocConfig sets up the user interface according to the given instance of a wpa_supplicant.
	 * The class provides function to execute the dialog for adding (w/o scan result) and configuring a network (by network id).
	 *
	 * By accepting the settings made in the UI they are verified (on error the dialog stays open).
	 */
	class CAdhocConfig : public QDialog {
		Q_OBJECT
	private:
		Ui::adhoc ui;
		QRegExpValidator * m_HexValidator;
		int m_CurrentID;
		libnutwireless::CWpaSupplicant * m_Supplicant;
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
		CAdhocConfig(libnutwireless::CWpaSupplicant * wpa_supplicant, QWidget * parent = 0);
		/// @brief Destroyes the object.
		~CAdhocConfig();
	private slots:
		void verifyConfiguration();
		void convertSSID(bool hex);
	};
};
#endif

#endif
