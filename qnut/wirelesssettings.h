//
// C++ Interface: wirelesssettings
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_WIRELESSSETTINGS_H
#define QNUT_WIRELESSSETTINGS_H

#ifndef QNUT_NO_WIRELESS
#include <QDialog>
#include <libnutclient/client.h>
#include "ui/ui_airset.h"

namespace qnut {
	class CManagedAPModel;
	class CAvailableAPModel;
	class CManagedAPProxyModel;
	class CAvailableAPProxyModel;
	
	/**
	 * @brief CWirelessSettings provides an UI to configure the wireless Connection for a given CDevice.
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 * 
	 * On creation, the CWirelessSettings sets up the basic UI and waits for the CWpaSupplicant
	 * to retrieve the device properties. While waiting it disables the UI for user input.
	 * 
	 * The class provides public functions to set and get the state of a detailed view.
	 */
	class CWirelessSettings : public QWidget {
		Q_OBJECT
	private:
		Ui::airset ui;
		libnutclient::CDevice * m_Device;
		
		CManagedAPModel * m_ManagedAPModel;
		CManagedAPProxyModel * m_ManagedAPProxyModel;
		CAvailableAPModel * m_AvailableAPModel;
		CAvailableAPProxyModel * m_AvailableAPProxyModel;
		
		QAction * m_EnableNetworkAction;
		QAction * m_DisableNetworkAction;
		QAction * m_SwitchNetworkAction;
		QAction * m_ConfigureNetworkAction;
		QAction * m_RemoveNetworkAction;
		
		QAction * m_ToggleDetailsAction;
		QAction * m_SaveNetworksAction;
		QAction * m_RescanNetworksAction;
		
		inline void createActions();
		QModelIndex selectedIndex(QTreeView * view);
	public:
		/// @brief returnes the visibility state of the details
		inline bool detailsVisible() const { return m_ToggleDetailsAction->isChecked(); }
		
		/**
		 * @brief sets the visibility state of the details
		 * @param value visibilty state
		 */
		inline void setDetailsVisible(bool value) { m_ToggleDetailsAction->setChecked(value); }
		
		/**
		 * @brief Creates the object and initializes the basic user interface.
		 * @param wireless CDevice with wireless support
		 * @param parent parent widget
		 */
		CWirelessSettings(libnutclient::CDevice * wireless, QWidget * parent = 0);
	private slots:
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
	};
}
#endif

#endif
