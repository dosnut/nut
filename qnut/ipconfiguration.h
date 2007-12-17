//
// C++ Interface: ipconfiguration
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_IPCONFIGURATION_H
#define QNUT_IPCONFIGURATION_H

#include <QDialog>
#include <libnutclient/client.h>
#include "ui/ui_ipconf.h"

namespace qnut {
	/**
	 * @brief CIPConfiguration provides a dialog to configure an user definable interface.
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 * 
	 * On creation, the CScriptSettings sets up the user interface to configure an user definable interface.
	 * It provides a public function to open the dialog for an existing user configuration.
	 */
	class CIPConfiguration : public QDialog {
		Q_OBJECT
	private:
		Ui::ipconf ui;
		QList<QHostAddress> m_DNSList;
	public:
		/**
		 * @brief Opens the dialog and returns true if changes are made.
		 * @param config existing user configuration
		 */
		bool execute(libnutcommon::IPv4UserConfig & config);
		/**
		 * @brief Creates the object and initializes its user interface.
		 * @param parent parent widget
		 */
		CIPConfiguration(QWidget * parent = 0);
	private slots:
		void addDNS();
		void removeDNS();
		void handleSelectionChanged(const QItemSelection & selected);
	};

};

#endif
