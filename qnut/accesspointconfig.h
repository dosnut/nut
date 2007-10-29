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

namespace qnut {
	using namespace libnut;
	
	class CAccessPointConfig : public QDialog {
		Q_OBJECT
	private:
		Ui::apconf ui;
		
		QRegExpValidator * hexValidator;
		
		CWpa_Supplicant * supplicant;
	public:
		bool execute(wps_scan scanResult);
		bool execute(wps_network network);
		
		CAccessPointConfig(CWpa_Supplicant * wpa_supplicant, QWidget * parent = 0);
		~CAccessPointConfig();
	private slots:
		void uiHandleAuthChanged(int type);
		void uiHandleEncChanged(QString text);
		void convertSSID(bool hex);
	};
}

#endif
