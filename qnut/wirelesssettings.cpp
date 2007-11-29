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
		
		managedAPModel = new CManagedAPModel();
		availableAPModel = new CAvailableAPModel();
		
		ui.managedView->setModel(managedAPModel);
		ui.availableView->setModel(availableAPModel);
		
		createActions();
		
		ui.managedView->header()->setResizeMode(QHeaderView::ResizeToContents);
		ui.availableView->header()->setResizeMode(QHeaderView::ResizeToContents);
		
		updateUi(device->state);
		
		ui.managedView->header()->setMinimumSectionSize(-1);
		ui.availableView->header()->setMinimumSectionSize(-1);
		
		connect(device, SIGNAL(stateChanged(libnutcommon::DeviceState)), this, SLOT(updateUi(libnutcommon::DeviceState)));
		
		connect(ui.managedView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(handleManagedAPSelectionChanged(const QItemSelection &, const QItemSelection &)));
		connect(ui.availableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(handleAvailableAPSelectionChanged(const QItemSelection &, const QItemSelection &)));
		
		connect(ui.availableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(addNetwork()));
		connect(ui.managedView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(switchToSelectedNetwork()));
		
		//connect(ui.managedView->model(), SIGNAL(modelAboutToBeReset()), ui.managedView, SLOT(clearSelection()));
	}
	
	CWirelessSettings::~CWirelessSettings() {
	}
	
	inline void CWirelessSettings::createActions() {
		enableNetworkAction    = new QAction(QIcon(UI_ICON_ENABLE), tr("&Enable"), this);
		enableNetworksAction   = new QAction(QIcon(UI_ICON_ENABLE_ALL), tr("Enable &all"), this);
		disableNetworkAction   = new QAction(QIcon(UI_ICON_DISABLE), tr("&Disable"), this);
		switchNetworkAction    = new QAction(QIcon(UI_ICON_FORCE), tr("S&witch"), this);
		configureNetworkAction = new QAction(QIcon(UI_ICON_CONFIGURE), tr("&Configure..."), this);
		removeNetworkAction    = new QAction(QIcon(UI_ICON_REMOVE), tr("&Remove"), this);
		
		addNetworkAction       = new QAction(QIcon(UI_ICON_ADD), tr("Add &network"), this);
		addAdhocAction         = new QAction(QIcon(UI_ICON_ADD_ADHOC), tr("Add ad-&hoc"), this);
		
		rescanNetworksAction   = new QAction(QIcon(UI_ICON_SEARCH), tr("Scan ne&tworks"), this);
		
		saveNetworksAction     = new QAction(QIcon(UI_ICON_SAVE), tr("&Save configuration"), this);
		reloadNetworksAction   = new QAction(QIcon(UI_ICON_RELOAD), tr("Re&load configuration"), this);
		
		toggleDetailsAction    = new QAction(QIcon(UI_ICON_DETAILED), tr("Detailed &view"), this);
		
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
		connect(reloadNetworksAction, SIGNAL(triggered()), ui.managedView->model(), SLOT(updateNetworks()));
		
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
	
	void CWirelessSettings::handleManagedAPSelectionChanged(const QItemSelection & selected, const QItemSelection &) {
		qDebug("selection changed");
		QModelIndexList selectedIndexes = selected.indexes();
		if (selected.isEmpty())
			qDebug("empty selection");
		if (ui.managedView->selectionModel()->selectedIndexes().isEmpty()) qDebug("ZAPP!");
		enableNetworkAction->setDisabled(selected.isEmpty());
		disableNetworkAction->setDisabled(selectedIndexes.isEmpty());
		
		switchNetworkAction->setDisabled(selectedIndexes.isEmpty());
		configureNetworkAction->setDisabled(selectedIndexes.isEmpty());
		removeNetworkAction->setDisabled(selectedIndexes.isEmpty());
	}
	
	void CWirelessSettings::handleAvailableAPSelectionChanged(const QItemSelection & selected, const QItemSelection &) {
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
		
		managedAPModel->setWpaSupplicant(device->wpa_supplicant);
		availableAPModel->setWpaSupplicant(device->wpa_supplicant);
	}
	
	void CWirelessSettings::switchToSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		
		if (!selectedIndexes.isEmpty())
			device->wpa_supplicant->selectNetwork(selectedIndexes[0].internalId());
	}
	
	void CWirelessSettings::addNetwork() {
		QModelIndexList selectedIndexes = ui.availableView->selectionModel()->selectedIndexes();
		
		if (selectedIndexes.isEmpty()) {
			CAccessPointConfig dialog(device->wpa_supplicant, this);
			dialog.execute();
		}
		else {
			ScanResult scan = availableAPModel->cachedScans()[selectedIndexes[0].row()];
			if (scan.opmode == OPM_ADHOC) {
				CAdhocConfig dialog(device->wpa_supplicant, this);
				dialog.execute(scan);
			}
			else {
				CAccessPointConfig dialog(device->wpa_supplicant, this);
				dialog.execute(scan);
			}
		}
	}
	
	void CWirelessSettings::addAdhoc() {
		CAdhocConfig dialog(device->wpa_supplicant, this);
		dialog.execute();
	}
	
	void CWirelessSettings::removeSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		ShortNetworkInfo network = managedAPModel->cachedNetworks()[selectedIndexes[0].row()];
		
		if (QMessageBox::question(this, tr("Removing a managed network"), tr("Are you sure to remove \"%1\"?").arg(network.ssid),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
			device->wpa_supplicant->removeNetwork(network.id);
		}
	}
	
	void CWirelessSettings::configureSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		ShortNetworkInfo network = managedAPModel->cachedNetworks()[selectedIndexes[0].row()];
		
		bool accepted = false;
		if (network.adhoc) {
			CAdhocConfig dialog(device->wpa_supplicant, this);
			accepted = dialog.execute(selectedIndexes[0].internalId());
		}
		else {
			CAccessPointConfig dialog(device->wpa_supplicant, this);
			accepted = dialog.execute(selectedIndexes[0].internalId());
		}
	}
	
	void CWirelessSettings::enableSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		device->wpa_supplicant->enableNetwork(selectedIndexes[0].internalId());
	}
	
	void CWirelessSettings::enableNetworks() {
		QList<ShortNetworkInfo> networks = static_cast<CManagedAPModel *>(ui.managedView->model())->cachedNetworks();
		foreach (ShortNetworkInfo i, networks) {
			device->wpa_supplicant->enableNetwork(i.id);
		}
	}
	
	void CWirelessSettings::disableSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		device->wpa_supplicant->disableNetwork(selectedIndexes[0].internalId());
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
