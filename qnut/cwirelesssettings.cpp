//
// C++ Implementation: wirelesssettings
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_NO_WIRELESS
#include "cwirelesssettings.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QMenu>
#include <QListView>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QSignalMapper>

#include <libnutclient/client.h>

#include <libnutwireless/cwireless.h>
#include <libnutwireless/conversion.h>

#include "common.h"
#include "constants.h"

#include "caccesspointconfig.h"
#include "cadhocconfig.h"

#include "modelview/cmanagedapmodel.h"
#include "modelview/cavailableapmodel.h"

namespace qnut {
	//TODO make only theese functions available that are supported by the wpa_supplicant
	using namespace libnutcommon;
	using namespace libnutclient;
	using namespace libnutwireless;
	
	CWirelessSettings::CWirelessSettings(CDevice * wireless, QWidget * parent) : QWidget(parent), m_Device(wireless) {
		ui.setupUi(this);
		
		QListView * hiddenListView = new QListView(this);
		hiddenListView->hide();
		hiddenListView->setViewMode(QListView::IconMode);
		hiddenListView->setIconSize(QSize(24, 24));
		hiddenListView->setGridSize(QSize(96, 64));
		
		setWindowTitle(tr("QNUT - Wireless Settings for \"%1\"").arg(m_Device->getName()));
		setWindowIcon(QIcon(UI_ICON_AP));
		
		ui.nameLabel->setText(m_Device->getName());
		
		m_ManagedAPProxyModel = new CManagedAPProxyModel(this);
		m_ManagedAPModel = new CManagedAPModel(m_Device->getWireless()->getWpaSupplicant(), this);
		m_ManagedAPProxyModel->setSourceModel(m_ManagedAPModel);
		
		m_AvailableAPProxyModel = new CAvailableAPProxyModel(this);
		m_AvailableAPModel = new CAvailableAPModel(m_Device->getWireless()->getHardware(), this);
		m_AvailableAPProxyModel->setSourceModel(m_AvailableAPModel);
		
		createActions();
		
		ui.managedView->setModel(m_ManagedAPProxyModel);
		ui.availableView->setModel(m_AvailableAPProxyModel);
		
		ui.managedView->header()->setResizeMode(QHeaderView::ResizeToContents);
		ui.availableView->header()->setResizeMode(QHeaderView::ResizeToContents);
		
		updateUi(m_Device->getState());
		
		ui.managedView->header()->setMinimumSectionSize(-1);
		ui.availableView->header()->setMinimumSectionSize(-1);
		
		connect(m_Device, SIGNAL(stateChanged(libnutcommon::DeviceState)), this, SLOT(updateUi(libnutcommon::DeviceState)));
		
		connect(ui.managedView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(handleManagedAPSelectionChanged(const QItemSelection &, const QItemSelection &)));
		
		connect(ui.availableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(addNetwork()));
		connect(ui.managedView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(switchToSelectedNetwork()));
		
		connect(ui.availableAPFilterEdit, SIGNAL(textChanged(const QString &)), m_AvailableAPProxyModel, SLOT(setFilterWildcard(const QString &)));
		
		connect(m_Device->getWireless()->getHardware(), SIGNAL(signalQualityUpdated(libnutwireless::SignalQuality)),
			this, SLOT(updateSignalInfo(libnutwireless::SignalQuality)));
		
		connect(m_AvailableAPModel, SIGNAL(cachedScansUpdated()), this, SLOT(updateBSSIDMenu()));
	}
	
	inline void CWirelessSettings::createActions() {
		QAction * enableNetworksAction;
		
		QAction * addNetworkAction;
		QAction * addAdhocAction;
		
		QAction * reloadNetworksAction;
		
		QAction * importNetworksAction;
		QAction * exportNetworkAction;
		QAction * exportMultipleNetworksAction;
		
		QAction * reassociateAction;
		
		QMenu * advancedFuntionsMenu;
		QMenu * manageNetworksMenu;
		
		m_RescanNetworksAction    = new QAction(QIcon(UI_ICON_SEARCH), tr("Scan for ne&tworks"), this);
		m_ToggleScanResultsAction = new QAction(QIcon(UI_ICON_DETAILED), tr("Show scan results"), this);
		
		m_SwitchNetworkAction    = new QAction(QIcon(UI_ICON_FORCE), tr("S&witch"), this);
		m_ConfigureNetworkAction = new QAction(QIcon(UI_ICON_CONFIGURE), tr("&Configure..."), this);
		m_EnableNetworkAction    = new QAction(QIcon(UI_ICON_AP), tr("&Enable"), this);
		m_DisableNetworkAction   = new QAction(QIcon(UI_ICON_AP_DOWN), tr("&Disable"), this);
		
		enableNetworksAction         = new QAction(/*QIcon(UI_ICON_ENABLE_ALL), */tr("Enable &all"), this);
		importNetworksAction         = new QAction(/*QIcon(UI_ICON_IMPORT), */tr("&Import networks..."), this);
		exportNetworkAction          = new QAction(/*QIcon(UI_ICON_EXPORT), */tr("E&xport selected network..."), this);
		exportMultipleNetworksAction = new QAction(/*QIcon(UI_ICON_MULTIEXPORT), */tr("Export all &networks..."), this);
		
		addNetworkAction      = new QAction(QIcon(UI_ICON_ADD), tr("Add &network"), this);
		addAdhocAction        = new QAction(QIcon(UI_ICON_ADD_ADHOC), tr("Add ad-&hoc"), this);
		m_RemoveNetworkAction = new QAction(QIcon(UI_ICON_REMOVE), tr("&Remove"), this);
		
		m_SaveNetworksAction     = new QAction(QIcon(UI_ICON_SAVE), tr("&Write managed to global config"), this);
		reloadNetworksAction     = new QAction(/*QIcon(UI_ICON_RELOAD), */tr("Re&load global config"), this);
		m_AutoSaveNetworksAction = new QAction(/*QIcon(UI_ICON_AUTOSAVE), */tr("&Autowrite global config"), this);
		m_KeepScanResultsAction  = new QAction(/*QIcon(UI_ICON_DETAILED), */tr("&Keep scan results visible"), this);
		reassociateAction        = new QAction(QIcon(UI_ICON_RELOAD), tr("&Reassociate"), this);
		
		manageNetworksMenu = new QMenu(tr("More..."));
		manageNetworksMenu->setIcon(QIcon(UI_ICON_EDIT));
		
		advancedFuntionsMenu = new QMenu(tr("Advanced..."));
// 		advancedFuntionsMenu->setIcon(QIcon(UI_ICON_ADVANCED));
		
		m_SetBSSIDMenu = new QMenu(tr("Switch to alternate BSSID"));
		m_SetBSSIDMenu->setIcon(QIcon(UI_ICON_AP));
		updateBSSIDMenu();
		
		m_AutoSaveNetworksAction->setCheckable(true);
		
		m_ToggleScanResultsAction->setCheckable(true);
		m_ToggleScanResultsAction->setChecked(false);
		ui.availableAPGroupBox->setVisible(false);
		
		m_KeepScanResultsAction->setCheckable(true);
		
		m_EnableNetworkAction->setEnabled(false);
		m_DisableNetworkAction->setEnabled(false);
		m_SwitchNetworkAction->setEnabled(false);
		m_ConfigureNetworkAction->setEnabled(false);
		m_RemoveNetworkAction->setEnabled(false);
		
		connect(m_RescanNetworksAction, SIGNAL(triggered()), this, SLOT(handleRescanRequest()));
		connect(m_ToggleScanResultsAction, SIGNAL(toggled(bool)), ui.availableAPGroupBox, SLOT(setVisible(bool)));
		
		connect(m_SwitchNetworkAction, SIGNAL(triggered()), this, SLOT(switchToSelectedNetwork()));
		connect(m_ConfigureNetworkAction, SIGNAL(triggered()), this, SLOT(configureSelectedNetwork()));
		connect(m_EnableNetworkAction, SIGNAL(triggered()), this, SLOT(enableSelectedNetwork()));
		connect(m_DisableNetworkAction, SIGNAL(triggered()), this, SLOT(disableSelectedNetwork()));
		
		connect(enableNetworksAction, SIGNAL(triggered()), this, SLOT(enableNetworks()));
		connect(importNetworksAction, SIGNAL(triggered()), this, SLOT(importNetworks()));
		connect(exportNetworkAction, SIGNAL(triggered()), this, SLOT(exportSelectedNetwork()));
		connect(exportMultipleNetworksAction, SIGNAL(triggered()), this, SLOT(exportMultipleNetworks()));
		
		connect(addNetworkAction, SIGNAL(triggered()), this, SLOT(addNetwork()));
		connect(addAdhocAction, SIGNAL(triggered()), this, SLOT(addAdhoc()));
		connect(m_RemoveNetworkAction, SIGNAL(triggered()), this, SLOT(removeSelectedNetwork()));
		
		connect(m_SaveNetworksAction, SIGNAL(triggered()), m_Device->getWireless()->getWpaSupplicant(), SLOT(save_config()));
		connect(reloadNetworksAction, SIGNAL(triggered()), m_Device->getWireless()->getWpaSupplicant(), SLOT(reconfigure()));
		connect(m_KeepScanResultsAction, SIGNAL(toggled(bool)), this, SLOT(keepScanResultsVisible(bool)));
		connect(reassociateAction, SIGNAL(triggered()), m_Device->getWireless()->getWpaSupplicant(), SLOT(reassociate()));
		
		manageNetworksMenu->addAction(enableNetworksAction);
		manageNetworksMenu->addSeparator();
		manageNetworksMenu->addAction(importNetworksAction);
		manageNetworksMenu->addAction(exportNetworkAction);
		manageNetworksMenu->addAction(exportMultipleNetworksAction);
		
		advancedFuntionsMenu->addAction(m_SaveNetworksAction);
		advancedFuntionsMenu->addAction(reloadNetworksAction);
		advancedFuntionsMenu->addSeparator();
		advancedFuntionsMenu->addAction(m_AutoSaveNetworksAction);
		advancedFuntionsMenu->addAction(m_KeepScanResultsAction);
		advancedFuntionsMenu->addSeparator();
		advancedFuntionsMenu->addMenu(m_SetBSSIDMenu);
		
		ui.managedView->addAction(m_EnableNetworkAction);
		ui.managedView->addAction(m_DisableNetworkAction);
		ui.managedView->addAction(getSeparator(this));
		ui.managedView->addAction(m_SwitchNetworkAction);
		ui.managedView->addAction(m_ConfigureNetworkAction);
		ui.managedView->addAction(getSeparator(this));
		ui.managedView->addAction(addNetworkAction);
		ui.managedView->addAction(addAdhocAction);
		ui.managedView->addAction(m_RemoveNetworkAction);
		ui.managedView->addAction(getSeparator(this));
		ui.managedView->addAction(manageNetworksMenu->menuAction());
		ui.managedView->setContextMenuPolicy(Qt::ActionsContextMenu);
		
		ui.availableView->addAction(addNetworkAction);
		ui.availableView->addAction(getSeparator(this));
		ui.availableView->addAction(m_RescanNetworksAction);
		ui.availableView->setContextMenuPolicy(Qt::ActionsContextMenu);
		
		ui.enableNetworkButton->setDefaultAction(m_EnableNetworkAction);
		ui.disableNetworkButton->setDefaultAction(m_DisableNetworkAction);
		ui.switchNetworkButton->setDefaultAction(m_SwitchNetworkAction);
		ui.configureNetworkButton->setDefaultAction(m_ConfigureNetworkAction);
		ui.removeNetworkButton->setDefaultAction(m_RemoveNetworkAction);
		ui.toggleScanResultsButton->setDefaultAction(m_ToggleScanResultsAction);
		
		ui.manageNetworksButton->setMenu(manageNetworksMenu);
		ui.manageNetworksButton->setText(manageNetworksMenu->title());
		ui.manageNetworksButton->setIcon(QIcon(UI_ICON_EDIT));
		ui.manageNetworksButton->setPopupMode(QToolButton::InstantPopup);
		
		ui.addNetworkButton->setDefaultAction(addNetworkAction);
		ui.addAdhocButton->setDefaultAction(addAdhocAction);
		ui.rescanNetworksButton->setDefaultAction(m_RescanNetworksAction);
		
		ui.advancedFunctionsButton->setMenu(advancedFuntionsMenu);
		ui.advancedFunctionsButton->setText(advancedFuntionsMenu->title());
		ui.advancedFunctionsButton->setIcon(QIcon(UI_ICON_SELECTED));
		ui.advancedFunctionsButton->setPopupMode(QToolButton::InstantPopup);
		
		ui.reassociateButton->setDefaultAction(reassociateAction);
		
		m_SetBSSIDMapper = new QSignalMapper(this);
		connect(m_SetBSSIDMapper, SIGNAL(mapped(const QString &)), this, SLOT(handleBSSIDSwitchRequest(const QString &)));
	}
	
	void CWirelessSettings::updateSignalInfo(SignalQuality signal) {
		QString quality = QString::number(signal.quality.value) + '/' + QString::number(signal.quality.maximum);
		QString level;
		QString noise;
		bool showLevelNoise = true;
		
		switch (signal.type) {
		case WSR_RCPI:
			level = QString::number(signal.level.rcpi);
			noise = QString::number(signal.noise.rcpi);
			break;
		case WSR_ABSOLUTE:
			level = QString::number(signal.level.nonrcpi.value);
			noise = QString::number(signal.noise.nonrcpi.value);
			break;
		case WSR_RELATIVE:
			level = QString::number(signal.level.nonrcpi.value) + '/' + QString::number(signal.level.nonrcpi.maximum);
			noise = QString::number(signal.noise.nonrcpi.value) + '/' + QString::number(signal.noise.nonrcpi.maximum);
			break;
		default:
			showLevelNoise = false;
			break;
		}
		
		ui.signalLabel->setText(signal.ssid + " (" +
			tr("Channel") + ' ' + QString::number(frequencyToChannel(signal.frequency)) + ')');
		ui.bssidLabel->setText(signal.bssid.toString());
		ui.qualityLabel->setText(tr("Quality") + ": " + quality);
		ui.rateLabel->setText(tr("Bitrate") + ": " + QString::number(signal.bitrates[0] / 1000000) + "Mb/s");
		
		ui.qualityLevelLine->setVisible(showLevelNoise);
		ui.levelLabel->setVisible(showLevelNoise);
		ui.noiseLabel->setVisible(showLevelNoise);
		if (showLevelNoise) {
			ui.levelLabel->setText(tr("Level") + ": " + level + "dBm");
			ui.noiseLabel->setText(tr("Noise") + ": " + noise + "dBm");
		}
	}
	
	void CWirelessSettings::handleManagedAPSelectionChanged(const QItemSelection & selected, const QItemSelection &) {
		m_EnableNetworkAction->setDisabled(selected.isEmpty());
		m_DisableNetworkAction->setDisabled(selected.isEmpty());
		m_SwitchNetworkAction->setDisabled(selected.isEmpty());
		m_ConfigureNetworkAction->setDisabled(selected.isEmpty());
		m_RemoveNetworkAction->setDisabled(selected.isEmpty());
	}
	
	void CWirelessSettings::updateUi(DeviceState state) {
		ui.managedAvailableAPSplitter->setEnabled(state >= DS_ACTIVATED && m_Device->getWireless());
		ui.actionsWidget->setEnabled(state >= DS_ACTIVATED && m_Device->getWireless());
		ui.signalFrame->setEnabled(state > DS_ACTIVATED && m_Device->getWireless());
		
		ui.iconLabel->setPixmap(QPixmap(iconFile(m_Device)));
		ui.stateLabel->setText(toStringTr(state));
		
		if (state <= DS_ACTIVATED) {
			ui.signalLabel->setText(tr("no signal info"));
			ui.bssidLabel->setText("00:00:00:00:00:00");
			ui.qualityLabel->setText(tr("Quality") + ": 0/0");
			ui.rateLabel->setText(tr("Bitrate") + ": 0Mb/s");
			ui.levelLabel->setText(tr("Level") + ": 0");
			ui.noiseLabel->setText(tr("Noise") + ": 0");
		}
		
		updateBSSIDMenu();
	}
	
	QModelIndex CWirelessSettings::selectedIndex(QAbstractItemView * view) {
		QModelIndexList selectedIndexes = view->selectionModel()->selectedIndexes();
		
		if (selectedIndexes.isEmpty())
			return QModelIndex();
		
		QSortFilterProxyModel * proxyModel = qobject_cast<QSortFilterProxyModel *>(view->model());
		if (proxyModel)
			return proxyModel->mapToSource(selectedIndexes[0]);
		else
			return selectedIndexes[0];
	}
	
	void CWirelessSettings::addNetwork() {
		QModelIndex index = selectedIndex(ui.availableView);
		
		bool accepted = false;
		if (index.isValid()) {
			ScanResult scan = m_AvailableAPModel->cachedScans()[m_AvailableAPModel->scanResultIdByModelIndex(index)];
			if (scan.opmode == OPM_ADHOC) {
				CAdhocConfig dialog(m_Device->getWireless(), this);
				accepted = dialog.execute(scan);
			}
			else {
				CAccessPointConfig dialog(m_Device->getWireless(), this);
				accepted = dialog.execute(scan);
			}
		}
		else {
			CAccessPointConfig dialog(m_Device->getWireless(), this);
			accepted = dialog.execute();
		}
		if (accepted && m_AutoSaveNetworksAction->isChecked())
			m_SaveNetworksAction->trigger();
	}
	
	void CWirelessSettings::addAdhoc() {
		CAdhocConfig dialog(m_Device->getWireless(), this);
		if (dialog.execute() && m_AutoSaveNetworksAction->isChecked())
			m_SaveNetworksAction->trigger();
	}
	
	void CWirelessSettings::switchToSelectedNetwork() {
		int id = m_ManagedAPModel->cachedNetworks()[selectedIndex(ui.managedView).internalId()].id;
		
		m_Device->getWireless()->getWpaSupplicant()->selectNetwork(id);
		if (m_AutoSaveNetworksAction->isChecked())
			m_SaveNetworksAction->trigger();
	}
	
	void CWirelessSettings::removeSelectedNetwork() {
		ShortNetworkInfo network = m_ManagedAPModel->cachedNetworks()[selectedIndex(ui.managedView).internalId()];
		
		QString message = tr("Are you sure to remove \"%1\"?").arg(network.ssid);
		if (m_AutoSaveNetworksAction->isChecked())
			message += '\n' + tr("This action is not reversible.");
		
		if (QMessageBox::question(this, tr("Removing a managed network"), message,
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
			m_Device->getWireless()->getWpaSupplicant()->removeNetwork(network.id);
			if (m_AutoSaveNetworksAction->isChecked())
				m_SaveNetworksAction->trigger();
	}
	
	void CWirelessSettings::configureSelectedNetwork() {
		ShortNetworkInfo network = m_ManagedAPModel->cachedNetworks()[selectedIndex(ui.managedView).internalId()];
		
		CAbstractWifiNetConfigDialog * dialog;
		
		if (network.adhoc)
			dialog = new CAdhocConfig(m_Device->getWireless(), this);
		else
			dialog = new CAccessPointConfig(m_Device->getWireless(), this);
		
		if (dialog->execute(network.id) && m_AutoSaveNetworksAction->isChecked())
			m_SaveNetworksAction->trigger();
		
		delete dialog;
	}
	
	void CWirelessSettings::enableSelectedNetwork() {
		int id = m_ManagedAPModel->cachedNetworks()[selectedIndex(ui.managedView).internalId()].id;
		
		m_Device->getWireless()->getWpaSupplicant()->enableNetwork(id);
		
		if (m_AutoSaveNetworksAction->isChecked())
			m_SaveNetworksAction->trigger();
	}
	
	void CWirelessSettings::disableSelectedNetwork() {
		int id = m_ManagedAPModel->cachedNetworks()[selectedIndex(ui.managedView).internalId()].id;
		
		m_Device->getWireless()->getWpaSupplicant()->disableNetwork(id);
		
		if (m_AutoSaveNetworksAction->isChecked())
			m_SaveNetworksAction->trigger();
	}
	
	void CWirelessSettings::enableNetworks() {
		QList<ShortNetworkInfo> networks = m_ManagedAPModel->cachedNetworks();
		foreach (ShortNetworkInfo i, networks) {
			m_Device->getWireless()->getWpaSupplicant()->enableNetwork(i.id);
		}
		
		if (m_AutoSaveNetworksAction->isChecked())
			m_SaveNetworksAction->trigger();
	}
	
	void CWirelessSettings::handleRescanRequest() {
		m_Device->getWireless()->scan();
		m_ToggleScanResultsAction->setChecked(true);
	}
	
	void CWirelessSettings::importNetworks() {
		QString fileName = QFileDialog::getOpenFileName(this, tr("Import network configuration"), QDir::currentPath(), tr("Configuration files (*.apconf *.conf)"));
		if (!fileName.isEmpty()) {
			QFile file(fileName);
			file.open(QFile::ReadOnly);
			QTextStream inStream(&file);
			
			m_Device->getWireless()->getWpaSupplicant()->addNetworks(&inStream);
			inStream.seek(0);
			qDebug("[qnut] imported networks:\n%s", qPrintable(inStream.readAll()));
		}
	}
	
	void CWirelessSettings::exportSelectedNetwork() {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Export network configuration"), QDir::currentPath(), tr("Configuration files (*.apconf *.conf)"));
		if (!fileName.isEmpty()) {
			QFile file(fileName);
			file.open(QFile::WriteOnly);
			QTextStream outStream(&file);
			
			int id = m_ManagedAPModel->cachedNetworks()[selectedIndex(ui.managedView).internalId()].id;
			m_Device->getWireless()->getWpaSupplicant()->getNetworkConfig(id).writeTo(outStream);
		}
	}
	
	void CWirelessSettings::exportMultipleNetworks() {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Export network configuration"), QDir::currentPath(), tr("Configuration files (*.apconf *.conf)"));
		if (!fileName.isEmpty()) {
			QFile file(fileName);
			file.open(QFile::WriteOnly);
			QTextStream outStream(&file);
			
			foreach (libnutwireless::ShortNetworkInfo i, m_ManagedAPModel->cachedNetworks())
				m_Device->getWireless()->getWpaSupplicant()->getNetworkConfig(i.id).writeTo(outStream);
		}
	}
	
	void CWirelessSettings::handleBSSIDSwitchRequest(const QString & data) {
		libnutcommon::MacAddress bssid = data;
		if (bssid.valid()) {
			m_Device->getWireless()->getWpaSupplicant()->setBssid(m_ManagedAPModel->currentID(), bssid);
			m_ManagedAPModel->updateNetworks();
			
			m_Device->getWireless()->getWpaSupplicant()->selectNetwork(m_ManagedAPModel->currentID());
			
// 			m_Device->getWireless()->getWpaSupplicant()->reassociate();
		}
	}
	
	inline QString signalQualityToString(const libnutwireless::SignalQuality & signal) {
		return QString::number(signal.quality.value) + '/'+
			QString::number(signal.quality.maximum);
	}
	
	void CWirelessSettings::updateBSSIDMenu() {
		foreach (QAction * i, m_SetBSSIDMenu->actions())
			m_SetBSSIDMapper->removeMappings(i);
		
		m_SetBSSIDMenu->clear();
		
		QAction * currentAction;
		
		if (m_Device->getState() > DS_ACTIVATED) {
			CAvailableAPModel::IndexList * scanList = m_AvailableAPModel->scanResultIdListBySSID(m_Device->getEssid());
			if (scanList) {
				QString bssidString;
				for (int i = 0; i < scanList->count(); i++) {
					bssidString = m_AvailableAPModel->cachedScans().at(scanList->at(i)).bssid.toString();
					currentAction = m_SetBSSIDMenu->addAction(tr("%1 (Quality: %2)")
						.arg(bssidString, signalQualityToString(m_AvailableAPModel->cachedScans().at(scanList->at(i)).signal)));
					m_SetBSSIDMapper->setMapping(currentAction, bssidString);
					connect(currentAction, SIGNAL(triggered()), m_SetBSSIDMapper, SLOT(map()));
				}
			}
		}
		
		if (m_SetBSSIDMenu->actions().isEmpty()) {
			currentAction = m_SetBSSIDMenu->addAction(tr("< no matching scan results >"));
			currentAction->setEnabled(false);
		}
		
	}
	
	void CWirelessSettings::keepScanResultsVisible(bool value) {
		ui.toggleScanResultsButton->setVisible(!value);
		ui.availableAPGroupBox->setVisible(value || ui.toggleScanResultsButton->isChecked());
	}
	
	void CWirelessSettings::readSettings(QSettings * settings) {
		settings->beginGroup(UI_SETTINGS_WIRELESSSETTINGS);
		restoreGeometry(settings->value(UI_SETTINGS_GEOMETRY).toByteArray());
		m_KeepScanResultsAction->setChecked(settings->value(UI_SETTINGS_SHOWSCANRESULTS, false).toBool());
		m_AutoSaveNetworksAction->setChecked(settings->value(UI_SETTINGS_AUTOWRITECONFIG, false).toBool());
		CAccessPointConfig::setLastFileOpenDir(settings->value(UI_SETTINGS_LASTFILEOPENDIR, "/").toString());
		
		QByteArray buffer = settings->value(UI_SETTINGS_NETWORKS).toByteArray();
		if (!buffer.isEmpty()) {
			QTextStream inStream(buffer, QIODevice::ReadOnly);
			m_Device->getWireless()->getWpaSupplicant()->addOnlyNewNetworks(&inStream);
		}
		settings->endGroup();
	}
	
	void CWirelessSettings::writeSettings(QSettings * settings) {
		settings->beginGroup(UI_SETTINGS_WIRELESSSETTINGS);
		settings->setValue(UI_SETTINGS_GEOMETRY, saveGeometry());
		settings->setValue(UI_SETTINGS_SHOWSCANRESULTS, m_KeepScanResultsAction->isChecked());
		settings->setValue(UI_SETTINGS_AUTOWRITECONFIG, m_AutoSaveNetworksAction->isChecked());
		settings->setValue(UI_SETTINGS_LASTFILEOPENDIR, CAccessPointConfig::lastFileOpenDir());
		
		QByteArray buffer;
		QTextStream outStream(buffer, QIODevice::WriteOnly);
		
		foreach (libnutwireless::CNetworkConfig i, m_Device->getWireless()->getWpaSupplicant()->getManagedConfigs())
			i.writeTo(outStream);
		
		if (buffer.isEmpty())
			settings->remove(UI_SETTINGS_NETWORKS);
		else
			settings->setValue(UI_SETTINGS_NETWORKS, buffer);
		
		settings->endGroup();
	}
}
#endif
