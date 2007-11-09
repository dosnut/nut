//
// C++ Interface: deviceconfiguration
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QNUT_SCRIPTSETTINGS_H
#define QNUT_SCRIPTSETTINGS_H

#include <QDialog>
#include "ui/ui_scrset.h"

namespace qnut {
	class CDeviceOptions;
	
	class CScriptSettings : public QDialog {
		Q_OBJECT
	private:
		Ui::scrset ui;
	public:
		bool execute(CDeviceOptions * deviceoptions);
		
		CScriptSettings(QWidget *parent = 0);
		~CScriptSettings();
	};
};

#endif
