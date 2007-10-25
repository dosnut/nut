//
// C++ Interface: wirelesssettings
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QNUT_WIRELESSSETTINGS_H
#define QNUT_WIRELESSSETTINGS_H

#include <QDialog>
#include <libnut/libnut_cli.h>
#include "ui/ui_airset.h"

namespace qnut {
	using namespace libnut;
	
	class CWirelessSettings : public QWidget {
		Q_OBJECT
	private:
		Ui::airset ui;
		CDevice * device;
		
		inline void setHeadInfo();
	public:
		CWirelessSettings(CDevice * wireless, QWidget *parent = 0);
		~CWirelessSettings();
		
	public slots:
		void uiHandleStateChange(DeviceState state);
		void uiHandleSwitchNetwork();
	};
};

#endif
