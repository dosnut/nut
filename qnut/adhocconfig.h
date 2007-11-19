//
// C++ Interface: adhocconfig
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QNUT_ADHOCCONFIG_H
#define QNUT_ADHOCCONFIG_H

#include <QDialog>
#include <libnutclient/client.h>
#include "ui/ui_adhoc.h"

namespace qnut {
	class CAdhocConfig : public QDialog {
		Q_OBJECT
	private:
		Ui::adhoc ui;
		QRegExpValidator * hexValidator;
		int currentID;
		libnutwireless::CWpa_Supplicant * supplicant;
	public:
		bool execute(int id);
		bool execute();
		
		CAdhocConfig(libnutwireless::CWpa_Supplicant * wpa_supplicant, QWidget * parent = 0);
		~CAdhocConfig();
	private slots:
		void verifyConfiguration();
		void convertSSID(bool hex);
	};
};

#endif
