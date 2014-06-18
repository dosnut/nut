//
// C++ Interface: wirelesssettings
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_CWIRELESSSETTINGS_H
#define QNUT_CWIRELESSSETTINGS_H

#ifndef QNUT_NO_WIRELESS
#include <QDialog>
#include <libnutwireless/hwtypes.h>
#include <libnutcommon/device.h>

#include "ui_wirelesssettings.h"

namespace libnutclient {
	class CDevice;
}

class QSettings;
class QSignalMapper;

namespace qnut {
	class CManagedAPModel;
	class CAvailableAPModel;
	class CManagedAPProxyModel;
	class CAvailableAPProxyModel;
	class CAccessPointModel;

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
		libnutclient::CDevice* const m_Device;

		CManagedAPModel * m_ManagedAPModel = nullptr;
		CManagedAPProxyModel * m_ManagedAPProxyModel = nullptr;
		CAvailableAPModel * m_AvailableAPModel = nullptr;
		CAvailableAPProxyModel * m_AvailableAPProxyModel = nullptr;

		CAccessPointModel * m_AccessPointModel = nullptr;

		QAction * m_EnableNetworkAction = nullptr;
		QAction * m_DisableNetworkAction = nullptr;
		QAction * m_SwitchNetworkAction = nullptr;
		QAction * m_ConfigureNetworkAction = nullptr;
		QAction * m_RemoveNetworkAction = nullptr;

		QAction * m_ToggleScanResultsAction = nullptr;

		QAction * m_SaveNetworksAction = nullptr;
		QAction * m_RescanNetworksAction = nullptr;
		QAction * m_AutoSaveNetworksAction = nullptr;
		QAction * m_KeepScanResultsAction = nullptr;

		QSignalMapper * m_SetBSSIDMapper = nullptr;

		QMenu * m_SetBSSIDMenu = nullptr;

		inline void createActions();
		QModelIndex selectedIndex(QAbstractItemView * view);
	public:
		/// @brief returnes the visibility state of the scan results panel
// 		inline bool scansVisible() const { return m_ToggleScanResultsAction->isChecked(); }

		/**
		 * @brief sets the visibility state of the scan results panel
		 * @param value visibilty state
		 */
// 		inline void setScansVisible(bool value) { m_ToggleScanResultsAction->setChecked(value); }

		/**
		 * @brief Creates the object and initializes the basic user interface.
		 * @param wireless CDevice with wireless support
		 * @param parent parent widget
		 */
		CWirelessSettings(libnutclient::CDevice * wireless, QWidget * parent = 0);

		void readSettings(QSettings * settings);
		void writeSettings(QSettings * settings);
	private slots:
		void handleManagedAPSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
		void updateUi(libnutcommon::DeviceState state);
		void updateSignalInfo(libnutwireless::SignalQuality signal);
		void switchToSelectedNetwork();
		void addNetwork();
		void addAdhoc();
		void removeSelectedNetwork();
		void configureSelectedNetwork();
		void enableSelectedNetwork();
		void enableNetworks();
		void disableSelectedNetwork();
		void handleRescanRequest();
		void importNetworks();
		void exportSelectedNetwork();
		void exportMultipleNetworks();
		void handleBSSIDSwitchRequest(const QString & data);
		void updateBSSIDMenu();
		void keepScanResultsVisible(bool value);
	};
}
#endif

#endif
