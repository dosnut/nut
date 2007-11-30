//
// C++ Implementation: wirelesssettings
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <QHeaderView>
#include <QMessageBox>
#include "wirelesssettings.h"
#include "managedapmodel.h"
#include "common.h"
#include "availableapmodel.h"
#include "accesspointconfig.h"
#include "adhocconfig.h"

namespace qnut {
	using namespace libnutcommon;
	using namespace libnutclient;
	using namespace libnutwireless;
	
	CWirelessSettings::CWirelessSettings(CDevice * wireless, QWidget * parent) : QWidget(parent), m_Device(wireless) {
		ui.setupUi(this);
		
		setWindowTitle(tr("Wireless Settings for \"%1\"").arg(m_Device->name));
		setWindowIcon(QIcon(UI_ICON_AIR));
		
		ui.nameLabel->setText(m_Device->name);
		
		m_ManagedAPModel = new CManagedAPModel();
		m_AvailableAPModel = new CAvailableAPModel();
		
		ui.managedView->setModel(m_ManagedAPModel);
		ui.availableView->setModel(m_AvailableAPModel);
		
		createActions();
		
		ui.managedView->header()->setResizeMode(QHeaderView::ResizeToContents);
		ui.availableView->header()->setResizeMode(QHeaderView::ResizeToContents);
		
		updateUi(m_Device->state);
		
		ui.managedView->header()->setMinimumSectionSize(-1);
		ui.availableView->header()->setMinimumSectionSize(-1);
		
		connect(m_Device, SIGNAL(stateChanged(libnutcommon::DeviceState)), this, SLOT(updateUi(libnutcommon::DeviceState)));
		
		connect(ui.managedView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(handleManagedAPSelectionChanged(const QItemSelection &, const QItemSelection &)));
		
		connect(ui.availableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(addNetwork()));
		connect(ui.managedView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(switchToSelectedNetwork()));
	}
	
	CWirelessSettings::~CWirelessSettings() {
	}
	
	inline void CWirelessSettings::createActions() {
		QAction * enableNetworksAction;
		
		QAction * addNetworkAction;
		QAction * addAdhocAction;
		
		QAction * reloadNetworksAction;
		
		m_EnableNetworkAction    = new QAction(QIcon(UI_ICON_ENABLE), tr("&Enable"), this);
		enableNetworksAction   = new QAction(QIcon(UI_ICON_ENABLE_ALL), tr("Enable &all"), this);
		m_DisableNetworkAction   = new QAction(QIcon(UI_ICON_DISABLE), tr("&Disable"), this);
		m_SwitchNetworkAction    = new QAction(QIcon(UI_ICON_FORCE), tr("S&witch"), this);
		m_ConfigureNetworkAction = new QAction(QIcon(UI_ICON_CONFIGURE), tr("&Configure..."), this);
		m_RemoveNetworkAction    = new QAction(QIcon(UI_ICON_REMOVE), tr("&Remove"), this);
		
		addNetworkAction       = new QAction(QIcon(UI_ICON_ADD), tr("Add &network"), this);
		addAdhocAction         = new QAction(QIcon(UI_ICON_ADD_ADHOC), tr("Add ad-&hoc"), this);
		
		m_RescanNetworksAction   = new QAction(QIcon(UI_ICON_SEARCH), tr("Scan ne&tworks"), this);
		
		m_SaveNetworksAction     = new QAction(QIcon(UI_ICON_SAVE), tr("&Save configuration"), this);
		reloadNetworksAction   = new QAction(QIcon(UI_ICON_RELOAD), tr("Re&load configuration"), this);
		
		m_ToggleDetailsAction    = new QAction(QIcon(UI_ICON_DETAILED), tr("Detailed &view"), this);
		
		m_ToggleDetailsAction->setCheckable(true);
		m_ToggleDetailsAction->setChecked(true);
		
		m_EnableNetworkAction->setEnabled(false);
		m_DisableNetworkAction->setEnabled(false);
		m_SwitchNetworkAction->setEnabled(false);
		m_ConfigureNetworkAction->setEnabled(false);
		m_RemoveNetworkAction->setEnabled(false);
		
		connect(m_EnableNetworkAction, SIGNAL(triggered()), this, SLOT(enableSelectedNetwork()));
		connect(enableNetworksAction, SIGNAL(triggered()), this, SLOT(enableNetworks()));
		connect(m_DisableNetworkAction, SIGNAL(triggered()), this, SLOT(disableSelectedNetwork()));
		connect(m_SwitchNetworkAction, SIGNAL(triggered()), this, SLOT(switchToSelectedNetwork()));
		connect(m_ConfigureNetworkAction, SIGNAL(triggered()), this, SLOT(configureSelectedNetwork()));
		connect(m_RemoveNetworkAction, SIGNAL(triggered()), this, SLOT(removeSelectedNetwork()));
		
		connect(addNetworkAction, SIGNAL(triggered()), this, SLOT(addNetwork()));
		connect(addAdhocAction, SIGNAL(triggered()), this, SLOT(addAdhoc()));
		
		connect(m_ToggleDetailsAction, SIGNAL(toggled(bool)), this, SLOT(toggleDetails(bool)));
		connect(reloadNetworksAction, SIGNAL(triggered()), ui.managedView->model(), SLOT(updateNetworks()));
		
		ui.managedView->addAction(m_EnableNetworkAction);
		ui.managedView->addAction(enableNetworksAction);
		ui.managedView->addAction(m_DisableNetworkAction);
		ui.managedView->addAction(getSeparator(this));
		ui.managedView->addAction(m_SwitchNetworkAction);
		ui.managedView->addAction(m_ConfigureNetworkAction);
		ui.managedView->addAction(getSeparator(this));
		ui.managedView->addAction(addAdhocAction);
		ui.managedView->addAction(m_RemoveNetworkAction);
		ui.managedView->setContextMenuPolicy(Qt::ActionsContextMenu);
		
		ui.availableView->addAction(addNetworkAction);
		ui.availableView->addAction(getSeparator(this));
		ui.availableView->addAction(m_RescanNetworksAction);
		ui.availableView->setContextMenuPolicy(Qt::ActionsContextMenu);
		
		ui.enableNetworkButton->setDefaultAction(m_EnableNetworkAction);
		ui.enableNetworksButton->setDefaultAction(enableNetworksAction);
		ui.disableNetworkButton->setDefaultAction(m_DisableNetworkAction);
		ui.switchNetworkButton->setDefaultAction(m_SwitchNetworkAction);
		ui.configureNetworkButton->setDefaultAction(m_ConfigureNetworkAction);
		ui.removeNetworkButton->setDefaultAction(m_RemoveNetworkAction);
		ui.saveNetworksButton->setDefaultAction(m_SaveNetworksAction);
		ui.reloadNetworksButton->setDefaultAction(reloadNetworksAction);
		ui.toggleDetailsButton->setDefaultAction(m_ToggleDetailsAction);
		
		ui.addNetworkButton->setDefaultAction(addNetworkAction);
		ui.addAdhocButton->setDefaultAction(addAdhocAction);
		ui.rescanNetworksButton->setDefaultAction(m_RescanNetworksAction);
	}
	
	void CWirelessSettings::updateSignalInfo(WextSignal signal) {
		ui.signalLabel->setText(tr("Signal (Quality, Level, Noise): %1").arg(signalSummary(signal)));
	}
	
	void CWirelessSettings::enableInterface() {
		setEnabled(true);
		disconnect(m_Device->wpa_supplicant, SIGNAL(signalQualityUpdated(libnutwireless::WextSignal)),
			this, SLOT(enableInterface()));
	}
	
	void CWirelessSettings::handleManagedAPSelectionChanged(const QItemSelection & selected, const QItemSelection &) {
		m_EnableNetworkAction->setDisabled(selected.isEmpty());
		m_DisableNetworkAction->setDisabled(selected.isEmpty());
		m_SwitchNetworkAction->setDisabled(selected.isEmpty());
		m_ConfigureNetworkAction->setDisabled(selected.isEmpty());
		m_RemoveNetworkAction->setDisabled(selected.isEmpty());
	}
	
	void CWirelessSettings::updateUi(DeviceState state) {
		ui.iconLabel->setPixmap(QPixmap(iconFile(m_Device)));
		ui.stateLabel->setText(toStringTr(m_Device->state));
		
		if (state <= DS_ACTIVATED)
			ui.signalLabel->setText("not assigned to accesspoint");
		
		if (state != DS_DEACTIVATED) {
			m_Device->wpa_supplicant->ap_scan(1);
			ui.signalLabel->setText(tr("waiting for device properties..."));
			connect(m_Device->wpa_supplicant, SIGNAL(signalQualityUpdated(libnutwireless::WextSignal)),
				this, SLOT(enableInterface()));
			connect(m_Device->wpa_supplicant, SIGNAL(closed()), ui.managedView, SLOT(clearSelection()));
			connect(m_SaveNetworksAction, SIGNAL(triggered()), m_Device->wpa_supplicant, SLOT(save_config()));
			connect(m_RescanNetworksAction, SIGNAL(triggered()), m_Device->wpa_supplicant, SLOT(scan()));
			connect(m_Device->wpa_supplicant, SIGNAL(signalQualityUpdated(libnutwireless::WextSignal)),
				this, SLOT(updateSignalInfo(libnutwireless::WextSignal)));
		}
		else {
			setEnabled(false);
		}
		
		m_ManagedAPModel->setWpaSupplicant(m_Device->wpa_supplicant);
		m_AvailableAPModel->setWpaSupplicant(m_Device->wpa_supplicant);
	}
	
	void CWirelessSettings::switchToSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		
		if (!selectedIndexes.isEmpty())
			m_Device->wpa_supplicant->selectNetwork(selectedIndexes[0].internalId());
	}
	
	void CWirelessSettings::addNetwork() {
		QModelIndexList selectedIndexes = ui.availableView->selectionModel()->selectedIndexes();
		
		if (selectedIndexes.isEmpty()) {
			CAccessPointConfig dialog(m_Device->wpa_supplicant, this);
			dialog.execute();
		}
		else {
			ScanResult scan = m_AvailableAPModel->cachedScans()[selectedIndexes[0].row()];
			if (scan.opmode == OPM_ADHOC) {
				CAdhocConfig dialog(m_Device->wpa_supplicant, this);
				dialog.execute(scan);
			}
			else {
				CAccessPointConfig dialog(m_Device->wpa_supplicant, this);
				dialog.execute(scan);
			}
		}
	}
	
	void CWirelessSettings::addAdhoc() {
		CAdhocConfig dialog(m_Device->wpa_supplicant, this);
		dialog.execute();
	}
	
	void CWirelessSettings::removeSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		ShortNetworkInfo network = m_ManagedAPModel->cachedNetworks()[selectedIndexes[0].row()];
		
		if (QMessageBox::question(this, tr("Removing a managed network"), tr("Are you sure to remove \"%1\"?").arg(network.ssid),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
			m_Device->wpa_supplicant->removeNetwork(network.id);
		}
	}
	
	void CWirelessSettings::configureSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		ShortNetworkInfo network = m_ManagedAPModel->cachedNetworks()[selectedIndexes[0].row()];
		
		bool accepted = false;
		if (network.adhoc) {
			CAdhocConfig dialog(m_Device->wpa_supplicant, this);
			accepted = dialog.execute(selectedIndexes[0].internalId());
		}
		else {
			CAccessPointConfig dialog(m_Device->wpa_supplicant, this);
			accepted = dialog.execute(selectedIndexes[0].internalId());
		}
	}
	
	void CWirelessSettings::enableSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		m_Device->wpa_supplicant->enableNetwork(selectedIndexes[0].internalId());
	}
	
	void CWirelessSettings::enableNetworks() {
		QList<ShortNetworkInfo> networks = static_cast<CManagedAPModel *>(ui.managedView->model())->cachedNetworks();
		foreach (ShortNetworkInfo i, networks) {
			m_Device->wpa_supplicant->enableNetwork(i.id);
		}
	}
	
	void CWirelessSettings::disableSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		m_Device->wpa_supplicant->disableNetwork(selectedIndexes[0].internalId());
	}
	
	void CWirelessSettings::toggleDetails(bool value) {
		if (value) {
			ui.managedView->showColumn(UI_MANAP_ID);
			ui.managedView->showColumn(UI_MANAP_BSSID);
			ui.availableView->showColumn(UI_AVLAP_BSSID);
			ui.availableView->showColumn(UI_AVLAP_CHANNEL);
			ui.availableView->showColumn(UI_AVLAP_LEVEL);
		}
		else {
			ui.managedView->hideColumn(UI_MANAP_ID);
			ui.managedView->hideColumn(UI_MANAP_BSSID);
			ui.availableView->hideColumn(UI_AVLAP_BSSID);
			ui.availableView->hideColumn(UI_AVLAP_CHANNEL);
			ui.availableView->hideColumn(UI_AVLAP_LEVEL);
		}
/*		ui.managedView->header()->resizeSections(QHeaderView::ResizeToContents);
		ui.availableView->header()->resizeSections(QHeaderView::ResizeToContents);*/
	}
};
