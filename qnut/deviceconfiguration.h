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
#ifndef QNUT_DEVICECONFIGURATION_H
#define QNUT_DEVICECONFIGURATION_H

#include <QDialog>
#include "ui/ui_devconf.h"
#include "deviceoptions.h"

namespace qnut {
	using namespace libnut;
	
	class CDeviceConfiguration : public QDialog {
		Q_OBJECT
	private:
		Ui::devconf ui;
	public:
		bool execute(CDeviceOptions * deviceoptions);
	
		CDeviceConfiguration(QWidget *parent = 0);
	
		~CDeviceConfiguration();
	
	};
};

#endif
