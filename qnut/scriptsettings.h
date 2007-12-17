//
// C++ Interface: deviceconfiguration
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_SCRIPTSETTINGS_H
#define QNUT_SCRIPTSETTINGS_H

#include <QDialog>
#include "ui/ui_scrset.h"

namespace qnut {
	class CDeviceDetails;
	
	/**
	 * @brief CScriptSettings provides a dialog to configure scripting flags.
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 * 
	 * On creation, the CScriptSettings sets up the user interface to configure scripting flags.
	 * It provides a public function to open the dialog for an existing instance of CDeviceDetails.
	 */
	class CScriptSettings : public QDialog {
		Q_OBJECT
	private:
		Ui::scrset ui;
	public:
		/**
		 * @brief Opens the dialog and returns true if changes are made.
		 * @param deviceoptions pointer to an existing instance of CDeviceDetails
		 */
		bool execute(CDeviceDetails * deviceoptions);
		/**
		 * @brief Creates the object and initializes its user interface.
		 * @param parent parent widget
		 */
		CScriptSettings(QWidget * parent = 0);
	};
};

#endif
