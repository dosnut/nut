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
#include <libnutclient/client.h>
#include "ui/ui_airset.h"

namespace qnut {
	class CWirelessSettings : public QWidget {
		Q_OBJECT
	private:
		Ui::airset ui;
		libnutclient::CDevice * device;
	public:
		CWirelessSettings(libnutclient::CDevice * wireless, QWidget * parent = 0);
		~CWirelessSettings();
	public slots:
		void handleManagedAPSelectionChanged(const QItemSelection & selected);
		void handleAvailableAPSelectionChanged(const QItemSelection & selected);
		void updateUi(libnutcommon::DeviceState state);
	private slots:
		void updateSignalInfo(libnutwireless::WextSignal signal);
		void switchToSelectedNetwork();
		void addSelectedScanResult();
		void removeSelectedNetwork();
		void configureSelectedNetwork();
	};
};

#endif
