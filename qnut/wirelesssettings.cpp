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

namespace qnut {
	using namespace libnut;
	using namespace libnutws;
	
	CWirelessSettings::CWirelessSettings(CDevice * wireless, QWidget * parent) : QWidget(parent), device(wireless) {
		ui.setupUi(this);
		
		setWindowTitle(tr("Wireless Settings for \"%1\"").arg(device->name));
		setWindowIcon(QIcon(UI_ICON_AIR_SETTINGS));
		
		ui.nameLabel->setText(device->name);
		
		ui.managedView->setModel(new CManagedAPModel());
		ui.availableView->setModel(new CAvailableAPModel());
		ui.managedView->header()->setResizeMode(QHeaderView::ResizeToContents);
		//ui.availableView->header()->setResizeMode(QHeaderView::ResizeToContents);
		
		updateUi(device->state);
		
		connect(device, SIGNAL(stateChanged(libnut::DeviceState)), this, SLOT(updateUi(libnut::DeviceState)));
		
		connect(ui.managedView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(handleManagedAPSelectionChanged(const QItemSelection &)));
		connect(ui.availableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(handleAvailableAPSelectionChanged(const QItemSelection &)));
		
		connect(ui.availableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(addSelectedScanResult()));
		connect(ui.managedView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(configureSelectedNetwork()));
		
		connect(ui.switchButton, SIGNAL(clicked()), this, SLOT(switchToSelectedNetwork()));
		connect(ui.rescanButton, SIGNAL(clicked()), device->wpa_supplicant, SLOT(scan()));
		connect(ui.addButton, SIGNAL(clicked()), this, SLOT(addSelectedScanResult()));
		
		connect(ui.removeButton, SIGNAL(clicked()), this, SLOT(removeSelectedNetwork()));
		connect(ui.configureButton, SIGNAL(clicked()), this, SLOT(configureSelectedNetwork()));
	}
	
	CWirelessSettings::~CWirelessSettings() {
	}
	
	void CWirelessSettings::updateSignalInfo(WextSignal signal) {
		ui.signalLabel->setText(tr("Signal (Quality, Level, Noise): %1").arg(signalSummary(signal)));
	}
	
	void CWirelessSettings::handleManagedAPSelectionChanged(const QItemSelection & selected) {
		QModelIndexList selectedIndexes = selected.indexes();
		
		ui.switchButton->setDisabled(selectedIndexes.isEmpty());
		ui.configureButton->setDisabled(selectedIndexes.isEmpty());
		ui.removeButton->setDisabled(selectedIndexes.isEmpty());
	}
	
	void CWirelessSettings::handleAvailableAPSelectionChanged(const QItemSelection & selected) {
		QModelIndexList selectedIndexes = selected.indexes();
		
		ui.addButton->setDisabled(selectedIndexes.isEmpty());
	}
	
	void CWirelessSettings::updateUi(DeviceState state) {
		ui.managedGroup->setEnabled(state != DS_DEACTIVATED);
		ui.availableGroup->setEnabled(state != DS_DEACTIVATED);
		
		ui.iconLabel->setPixmap(QPixmap(iconFile(device)));
		ui.stateLabel->setText(toString(device->state));
		
		if (state <= DS_ACTIVATED)
			ui.signalLabel->setText("not assigned to accesspoint");
		
		if (state != DS_DEACTIVATED)
			connect(device->wpa_supplicant, SIGNAL(signalQualityUpdated(libnutws::WextSignal)),
				this, SLOT(updateSignalInfo(libnutws::WextSignal)));
		
		dynamic_cast<CAvailableAPModel *>(ui.availableView->model())->setWpaSupplicant(device->wpa_supplicant);
		dynamic_cast<CManagedAPModel *>(ui.managedView->model())->setWpaSupplicant(device->wpa_supplicant);
	}
	
	void CWirelessSettings::switchToSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		wps_network * network = static_cast<wps_network *>(selectedIndexes[0].internalPointer());
		if (network) {
			device->wpa_supplicant->selectNetwork(network->id);
		}
	}
	
	void CWirelessSettings::addSelectedScanResult() {
		QModelIndexList selectedIndexes = ui.availableView->selectionModel()->selectedIndexes();
		CAccessPointConfig dialog(device->wpa_supplicant, this);
		wps_scan * scan = static_cast<wps_scan *>(selectedIndexes[0].internalPointer());
		
		if (dialog.execute(*scan))
			static_cast<CManagedAPModel *>(ui.managedView->model())->reloadNetworks();
	}
	
	void CWirelessSettings::removeSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		CAccessPointConfig dialog(device->wpa_supplicant, this);
		wps_network * network = static_cast<wps_network *>(selectedIndexes[0].internalPointer());
		
		if (QMessageBox::question(this, tr("Removing a managed network"), tr("Are you sure to remove \"%1\"?").arg(network->ssid),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
			device->wpa_supplicant->removeNetwork(network->id);
			static_cast<CManagedAPModel *>(ui.managedView->model())->reloadNetworks();
		}
	}
	
	void CWirelessSettings::configureSelectedNetwork() {
		QModelIndexList selectedIndexes = ui.managedView->selectionModel()->selectedIndexes();
		CAccessPointConfig dialog(device->wpa_supplicant, this);
		wps_network * network = static_cast<wps_network *>(selectedIndexes[0].internalPointer());
		
		if (dialog.execute(network->id))
			static_cast<CManagedAPModel *>(ui.managedView->model())->reloadNetworks();
	}
};
