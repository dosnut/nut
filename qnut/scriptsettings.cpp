//
// C++ Implementation: deviceconfiguration
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include <QDir>
#include <libnutclient/client.h>
#include "scriptsettings.h"
#include "devicedetails.h"
#include "constants.h"

namespace qnut {
	using namespace libnutclient;
	
	bool CScriptSettings::execute(CDeviceDetails * deviceoptions) {
		quint8 flags = deviceoptions->scriptFlags();
		ui.upCheck->setChecked(flags & UI_FLAG_SCRIPT_UP);
		ui.unconfiguredCheck->setChecked(flags & UI_FLAG_SCRIPT_UNCONFIGURED);
		ui.carrierCheck->setChecked(flags & UI_FLAG_SCRIPT_CARRIER);
		ui.activatedCheck->setChecked(flags & UI_FLAG_SCRIPT_ACTIVATED);
		ui.deactivatedCheck->setChecked(flags & UI_FLAG_SCRIPT_DEACTIVATED);
		if (exec() == QDialog::Accepted) {
			QDir workdir(UI_PATH_DEV(deviceoptions->device()->getName()));
			flags = UI_FLAG_SCRIPT_NONE;
			
			if (ui.upCheck->isChecked()) {
				flags = flags | UI_FLAG_SCRIPT_UP;
				if (!workdir.exists(UI_DIR_SCRIPT_UP))
					workdir.mkdir(UI_DIR_SCRIPT_UP);
			}
			if (ui.unconfiguredCheck->isChecked()) {
				flags = flags | UI_FLAG_SCRIPT_UNCONFIGURED;
				if (!workdir.exists(UI_DIR_SCRIPT_UNCONFIGURED))
					workdir.mkdir(UI_DIR_SCRIPT_UNCONFIGURED);
			}
			if (ui.carrierCheck->isChecked()) {
				flags = flags | UI_FLAG_SCRIPT_CARRIER;
				if (!workdir.exists(UI_DIR_SCRIPT_CARRIER))
					workdir.mkdir(UI_DIR_SCRIPT_CARRIER);
			}
			if (ui.activatedCheck->isChecked()) {
				flags = flags | UI_FLAG_SCRIPT_ACTIVATED;
				if (!workdir.exists(UI_DIR_SCRIPT_ACTIVATED))
					workdir.mkdir(UI_DIR_SCRIPT_ACTIVATED);
			}
			if (ui.deactivatedCheck->isChecked()) {
				flags = flags | UI_FLAG_SCRIPT_DEACTIVATED;
				if (!workdir.exists(UI_DIR_SCRIPT_DEACTIVATED))
					workdir.mkdir(UI_DIR_SCRIPT_DEACTIVATED);
			}
			
			deviceoptions->setScriptFlags(flags);
			
			return true;
		}
		else
			return false;
	}
	
	CScriptSettings::CScriptSettings(QWidget * parent) : QDialog(parent) {
		ui.setupUi(this);
	}
}
