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
		
		QAction * enableNetworkAction;
		QAction * enableNetworksAction;
		QAction * disableNetworkAction;
		QAction * switchNetworkAction;
		QAction * configureNetworkAction;
		QAction * removeNetworkAction;
		
		QAction * saveNetworksAction;
		
		QAction * addNetworkAction;
		QAction * addAdhocAction;
		
		QAction * rescanNetworksAction;
		QAction * reloadNetworksAction;
		QAction * toggleDetailsAction;
		inline void createActions();
	public:
		inline bool detailsVisible() const { return toggleDetailsAction->isChecked(); }
		inline void setDetailsVisible(bool value) { toggleDetailsAction->setChecked(value); }
		CWirelessSettings(libnutclient::CDevice * wireless, QWidget * parent = 0);
		~CWirelessSettings();
	public slots:
		void handleManagedAPSelectionChanged(const QItemSelection & selected);
		void handleAvailableAPSelectionChanged(const QItemSelection & selected);
		void updateUi(libnutcommon::DeviceState state);
	private slots:
		void updateSignalInfo(libnutwireless::WextSignal signal);
		void switchToSelectedNetwork();
		void addNetwork();
		void addAdhoc();
		void removeSelectedNetwork();
		void configureSelectedNetwork();
		void enableSelectedNetwork();
		void enableNetworks();
		void disableSelectedNetwork();
		void toggleDetails(bool value);
		void enableInterface();
	};
};

#endif
