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
	
	CWirelessSettings::CWirelessSettings(CDevice * wireless, QWidget * parent) : QWidget(parent), device(wireless) {
		ui.setupUi(this);
		
		setWindowTitle(tr("Wireless Settings for \"%1\"").arg(device->name));
		setWindowIcon(QIcon(UI_ICON_AIR));
		
		ui.nameLabel->setText(device->name);
		
		ui.managedView->setModel(new CManagedAPModel());
		ui.availableView->setModel(new CAvailableAPModel());
		
		createActions();
		
		ui.managedView->header()->setResizeMode(QHeaderView::ResizeToContents);
		ui.availableView->header()->setResizeMode(QHeaderView::ResizeToContents);
		
		updateUi(device->state);
		
		ui.managedView->header()->setMinimumSectionSize(-1);
		ui.availableView->header()->setMinimumSectionSize(-1);
		
		connect(device, SIGNAL(stateChanged(libnutcommon::DeviceState)), this, SLOT(updateUi(libnutcommon::DeviceState)));
		
		connect(ui.managedView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(handleManagedAPSelectionChanged(const QItemSelection &)));
		connect(ui.availableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(handleAvailableAPSelectionChanged(const QItemSelection &)));
		
		connect(ui.availableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(addNetwork()));
		connect(ui.managedView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(switchToSelectedNetwork()));
	}
	
	CWirelessSettings::~CWirelessSettings() {
	}
	
	inline void CWirelessSettings::createActions() {
		enableNetworkAction    = new QAction(QIcon(UI_ICON_ENABLE), tr("Enable"), this);
		enableNetworksAction   = new QAction(QIcon(UI_ICON_ENABLE_ALL), tr("Enable all"), this);
		disableNetworkAction   = new QAction(QIcon(UI_ICON_DISABLE), tr("Disable"), this);
		switchNetworkAction    = new QAction(QIcon(UI_ICON_FORCE), tr("Switch"), this);
		configureNetworkAction = new QAction(QIcon(UI_ICON_CONFIGURE), tr("Configure..."), this);
		removeNetworkAction    = new QAction(QIcon(UI_ICON_REMOVE), tr("Remove"), this);
		
		addNetworkAction       = new QAction(QIcon(UI_ICON_ADD), tr("Add network"), this);
		addAdhocAction         = new QAction(QIcon(UI_ICON_ADD_ADHOC), tr("Add ad-hoc"), this);
		
		rescanNetworksAction   = new QAction(QIcon(UI_ICON_SEARCH), tr("Rescan"), this);
		
		saveNetworksAction     = new QAction(QIcon(UI_ICON_SAVE), tr("Save configuration"), this);
		reloadNetworksAction   = new QAction(QIcon(UI_ICON_RELOAD), tr("Reload configuration"), this);
		
		toggleDetailsAction    = new QAction(QIcon(UI_ICON_DETAILED), tr("Detailed view"), this);
		
		toggleDetailsAction->setCheckable(true);
		toggleDetailsAction->setChecked(true);
		
		enableNetworkAction->setEnabled(false);
		disableNetworkAction->setEnabled(false);
		switchNetworkAction->setEnabled(false);
		configureNetworkAction->setEnabled(false);
		removeNetworkAction->setEnabled(false);
		
		connect(enableNetworkAction, SIGNAL(triggered()), this, SLOT(enableSelectedNetwork()));
		connect(enableNetworksAction, SIGNAL(triggered()), this, SLOT(enableNetworks()));
		connect(disableNetworkAction, SIGNAL(triggered()), this, SLOT(disableSelectedNetwork()));
		connect(switchNetworkAction, SIGNAL(triggered()), this, SLOT(switchToSelectedNetwork()));
		connect(configureNetworkAction, SIGNAL(triggered()), this, SLOT(configureSelectedNetwork()));
		connect(removeNetworkAction, SIGNAL(triggered()), this, SLOT(removeSelectedNetwork()));
		
		connect(addNetworkAction, SIGNAL(triggered()), this, SLOT(addNetwork()));
		connect(addAdhocAction, SIGNAL(triggered()), this, SLOT(addAdhoc()));
		
		connect(toggleDetailsAction, SIGNAL(toggled(bool)), this, SLOT(toggleDetails(bool)));
		connect(reloadNetworksAction, SIGNAL(triggered()), ui.managedView->model(), SLOT(reloadNetworks()));
		
		ui.managedView->addAction(enableNetworkAction);
		ui.managedView->addAction(enableNetworksAction);
		ui.managedView->addAction(disableNetworkAction);
		ui.managedView->addAction(getSeparator(this));
		ui.managedView->addAction(switchNetworkAction);
		ui.managedView->addAction(configureNetworkAction);
		ui.managedView->addAction(getSeparator(this));
		ui.managedView->addAction(addAdhocAction);
		ui.managedView->addAction(removeNetworkAction);
		ui.managedView->setContextMenuPolicy(Qt::ActionsContextMenu);
		
		ui.availableView->addAction(addNetworkAction);
		ui.availableView->addAction(getSeparator(this));
		ui.availableView->addAction(rescanNetworksAction);
		ui.availableView->setContextMenuPolicy(Qt::ActionsContextMenu);
		
		ui.enableNetworkButton->setDefaultAction(enableNetworkAction);
		ui.enableNetworksButton->setDefaultAction(enableNetworksAction);
		ui.disableNetworkButton->setDefaultAction(disableNetworkAction);
		ui.switchNetworkButton->setDefaultAction(switchNetworkAction);
		ui.configureNetworkButton->setDefaultAction(configureNetworkAction);
		ui.removeNetworkButton->setDefaultAction(removeNetworkAction);
		ui.saveNetworksButton->setDefaultAction(saveNetworksAction);
		ui.reloadNetworksButton->setDefaultAction(reloadNetworksAction);
		ui.toggleDetailsButton->setDefaultAction(toggleDetailsAction);
		
		ui.addNetworkButton->setDefaultAction(addNetworkAction);
		ui.addAdhocButton->setDefaultAction(addAdhocAction);
		ui.rescanNetworksButton->setDefaultAction(rescanNetworksAction);
	}
	
	void CWirelessSettings::updateSignalInfo(WextSignal signal) {
		ui.signalLabel->setText(tr("Signal (Quality, Level, Noise): %1").arg(signalSummary(signal)));
	}
	
	void CWirelessSettings::enableInterface() {
		setEnabled(true);
		disconnect(device->wpa_supplicant, SIGNAL(signalQualityUpdated(libnutwireless::WextSignal)),
			this, SLOT(enableInterface()));
	}
	
	void CWirelessSettings::handleManagedAPSelectionChanged(const QItemSelection & selected) {
		QModelIndexList selectedIndexes = selected.indexes();
		
		enableNetworkAction->setDisabled(selectedIndexes.isEmpty());
		disableNetworkAction->setDisabled(selectedIndexes.isEmpty());
		
		switchNetworkAction->setDisabled(selectedIndexes.isEmpty());
		configureNetworkAction->setDisabled(selectedIndexes.isEmpty());
		removeNetworkAction->setDisabled(selectedIndexes.isEmpty());
	}
	
	void CWirelessSettings::handleAvailableAPSelectionChanged(const QItemSelection & selected) {
		QModelIndexList selectedIndexes = selected.indexes();
		
		addNetworkAction->setDisabled(selectedIndexes.isEmpty());
	}
	
	void CWirelessSettings::updateUi(DeviceState state) {
		ui.iconLabel->setPixmap(QPixmap(iconFile(device)));
		ui.stateLabel->setText(toStringTr(device->state));
		
		if (state <= DS_ACTIVATED)
			ui.signalLabel->setText("not assigned to accesspoint");
		
		if (state != DS_DEACTIVATED) {
			device->wpa_supplicant->ap_scan(1);
			ui.signalLabel->setText(tr("waiting for device properties..."));
			connect(device->wpa_supplicant, SIGNAL(signalQualityUpdated(libnutwireless::WextSignal)),
				this, SLOT(enableInterface()));
			connect(device->wpa_supplicant, SIGNAL(closed()), ui.managedView, SLOT(clearSelection()));
			connect(saveNetworksAction, SIGNAL(triggered()), device->wpa_supplicant, SLOT(save_config()));
			connect(rescanNetworksAction, SIGNAL(triggered()), device->wpa_supplicant, SLOT(scan()));
			connect(device->wpa_supplicant, SIGNAL(signalQualityUpdated(libnutwireless::WextSignal)),
				this, SLOT(updateSignalInfo(libnutwireless::WextSignal)));
		}
		else {
			setEnabled(false);
		}
		
		dynamic_cast<CAvailableAPModel *>(ui.availableView->model())->setWpaSupplicant(device->wpa_supplicant);
		dynamic_cast<CManagedAPModel *>(ui.managedView->model())->setWpaSupplicant(device->wpa_supplicant);
	}
	
	void CWirelessSettings::switchToSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		ShortNetworkInfo * network = static_cast<ShortNetworkInfo *>(selectedIndexes[0].internalPointer());
		if (network) {
			device->wpa_supplicant->selectNetwork(network->id);
		}
	}
	
	void CWirelessSettings::addNetwork() {
		QModelIndexList selectedIndexes = ui.availableView->selectionModel()->selectedIndexes();
		
		bool networkAdded;
		if (selectedIndexes.isEmpty()) {
			CAccessPointConfig dialog(device->wpa_supplicant, this);
			networkAdded = dialog.execute();
		}
		else {
			ScanResult * scan = static_cast<ScanResult *>(selectedIndexes[0].internalPointer());
			if (scan->opmode == OPM_ADHOC) {
				CAdhocConfig dialog(device->wpa_supplicant, this);
				networkAdded = dialog.execute(*scan);
			}
			else if (scan->opmode == OPM_MANAGED) {
				CAccessPointConfig dialog(device->wpa_supplicant, this);
				networkAdded = dialog.execute(*scan);
			}
		}
		
		if (networkAdded)
			static_cast<CManagedAPModel *>(ui.managedView->model())->reloadNetworks();
	}
	
	void CWirelessSettings::addAdhoc() {
		CAdhocConfig dialog(device->wpa_supplicant, this);
		if (dialog.execute())
			static_cast<CManagedAPModel *>(ui.managedView->model())->reloadNetworks();
	}
	
	void CWirelessSettings::removeSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		ShortNetworkInfo * network = static_cast<ShortNetworkInfo *>(selectedIndexes[0].internalPointer());
		
		if (QMessageBox::question(this, tr("Removing a managed network"), tr("Are you sure to remove \"%1\"?").arg(network->ssid),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
			device->wpa_supplicant->removeNetwork(network->id);
			static_cast<CManagedAPModel *>(ui.managedView->model())->reloadNetworks();
			ui.managedView->clearSelection();
		}
	}
	
	void CWirelessSettings::configureSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		ShortNetworkInfo * network = static_cast<ShortNetworkInfo *>(selectedIndexes[0].internalPointer());
		
		bool accepted = false;
		if (network->adhoc) {
			CAdhocConfig dialog(device->wpa_supplicant, this);
			accepted = dialog.execute(network->id);
		}
		else {
			CAccessPointConfig dialog(device->wpa_supplicant, this);
			accepted = dialog.execute(network->id);
		}
		
		if (accepted)
			static_cast<CManagedAPModel *>(ui.managedView->model())->reloadNetworks();
	}
	
	void CWirelessSettings::enableSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		ShortNetworkInfo * network = static_cast<ShortNetworkInfo *>(selectedIndexes[0].internalPointer());
		device->wpa_supplicant->enableNetwork(network->id);
		static_cast<CManagedAPModel *>(ui.managedView->model())->reloadNetworks();
	}
	
	void CWirelessSettings::enableNetworks() {
		QList<ShortNetworkInfo> networks = static_cast<CManagedAPModel *>(ui.managedView->model())->cachedNetworks();
		foreach (ShortNetworkInfo i, networks) {
			device->wpa_supplicant->enableNetwork(i.id);
		}
		static_cast<CManagedAPModel *>(ui.managedView->model())->reloadNetworks();
	}
	
	void CWirelessSettings::disableSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		ShortNetworkInfo * network = static_cast<ShortNetworkInfo *>(selectedIndexes[0].internalPointer());
		device->wpa_supplicant->disableNetwork(network->id);
		static_cast<CManagedAPModel *>(ui.managedView->model())->reloadNetworks();
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
