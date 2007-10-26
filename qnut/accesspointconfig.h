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
#include <libnut/libnut_cli.h>
#include "ui/ui_apconf.h"
#include "ui/ui_pskconf.h"
#include "ui/ui_eapconf.h"
#include "ui/ui_wepconf.h"

namespace qnut {
	using namespace libnut;
	
	class CAccessPointConfig : public QDialog {
		Q_OBJECT
	private:
		Ui::apconf ui;
		Ui::pskconf pskUi;
		Ui::eapconf eapUi;
		Ui::wepconf wepUi;
		
		QWidget * pskWidget;
		QWidget * eapWidget;
		QWidget * wepWidget;
		
		QVBoxLayout * confLayout;
		
		CWpa_Supplicant * supplicant;
	public:
		bool execute(wps_scan scanResult);
		bool execute(wps_network network);
		
		CAccessPointConfig(CWpa_Supplicant * wpa_supplicant, QWidget * parent = 0);
		~CAccessPointConfig();
	private slots:
		void uiHandleAuthChanged(int type);
		//void uiHandleEncChanged(int type);
	};
}

#endif
