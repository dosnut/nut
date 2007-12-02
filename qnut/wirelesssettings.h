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
	class CManagedAPModel;
	class CAvailableAPModel;
	
	class CWirelessSettings : public QWidget {
		Q_OBJECT
	protected:
		Ui::airset ui;
		libnutclient::CDevice * m_Device;
		
		CManagedAPModel * m_ManagedAPModel;
		CAvailableAPModel * m_AvailableAPModel;
		
		QAction * m_EnableNetworkAction;
		QAction * m_DisableNetworkAction;
		QAction * m_SwitchNetworkAction;
		QAction * m_ConfigureNetworkAction;
		QAction * m_RemoveNetworkAction;
		
		QAction * m_ToggleDetailsAction;
		QAction * m_SaveNetworksAction;
		QAction * m_RescanNetworksAction;
		
		inline void createActions();
	public:
		inline bool detailsVisible() const { return m_ToggleDetailsAction->isChecked(); }
		inline void setDetailsVisible(bool value) { m_ToggleDetailsAction->setChecked(value); }
		CWirelessSettings(libnutclient::CDevice * wireless, QWidget * parent = 0);
		~CWirelessSettings();
	protected slots:
		void handleManagedAPSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
		void updateUi(libnutcommon::DeviceState state);
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
