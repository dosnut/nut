//
// C++ Implementation: deviceconfiguration
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "scriptsettings.h"
#include "constants.h"

namespace qnut {
	bool CScriptSettings::execute(CDeviceOptions * deviceoptions) {
		quint8 flags = deviceoptions->scriptFlags;
		ui.upCheck->setChecked(flags && UI_FLAG_SCRIPT_UP);
		ui.unconfiguredCheck->setChecked(flags && UI_FLAG_SCRIPT_UNCONFIGURED);
		ui.carrierCheck->setChecked(flags && UI_FLAG_SCRIPT_CARRIER);
		ui.activatedCheck->setChecked(flags && UI_FLAG_SCRIPT_ACTIVATED);
		ui.deactivatedCheck->setChecked(flags && UI_FLAG_SCRIPT_DEACTIVATED);
		if (exec() == QDialog::Accepted) {
			flags = UI_FLAG_SCRIPT_NONE;
			
			if (ui.upCheck->isChecked())
				flags = flags | UI_FLAG_SCRIPT_UP;
			if (ui.unconfiguredCheck->isChecked())
				flags = flags | UI_FLAG_SCRIPT_UNCONFIGURED;
			if (ui.carrierCheck->isChecked())
				flags = flags | UI_FLAG_SCRIPT_CARRIER;
			if (ui.activatedCheck->isChecked())
				flags = flags | UI_FLAG_SCRIPT_ACTIVATED;
			if (ui.deactivatedCheck->isChecked())
				flags = flags | UI_FLAG_SCRIPT_DEACTIVATED;
			
			deviceoptions->scriptFlags = flags;
			
			return true;
		}
		else
			return false;
	}
	
	CScriptSettings::CScriptSettings(QWidget *parent) : QDialog(parent) {
		ui.setupUi(this);
	}
	
	CScriptSettings::~CScriptSettings() {
	}
};
