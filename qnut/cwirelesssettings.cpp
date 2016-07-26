//
// C++ Implementation: wirelesssettings
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef NUT_NO_WIRELESS
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

		ui.managedView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
		ui.availableView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

		updateUi(m_Device->getState());

		ui.managedView->header()->setMinimumSectionSize(-1);
		ui.availableView->header()->setMinimumSectionSize(-1);

		connect(m_Device, &CDevice::stateChanged, this, &CWirelessSettings::updateUi);

		connect(ui.managedView->selectionModel(), &QItemSelectionModel::selectionChanged,
			this, &CWirelessSettings::handleManagedAPSelectionChanged);

		connect(ui.availableView, &QAbstractItemView::doubleClicked, this, &CWirelessSettings::addNetwork);
		connect(ui.managedView, &QAbstractItemView::doubleClicked, this, &CWirelessSettings::switchToSelectedNetwork);

		connect(ui.availableAPFilterEdit, &QLineEdit::textChanged, m_AvailableAPProxyModel, &CAvailableAPProxyModel::setFilterWildcard);

		connect(m_Device->getWireless()->getHardware(), &CWirelessHW::signalQualityUpdated,
			this, &CWirelessSettings::updateSignalInfo);

		connect(m_AvailableAPModel, &CAvailableAPModel::cachedScansUpdated, this, &CWirelessSettings::updateBSSIDMenu);
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
		advancedFuntionsMenu->setIcon(QIcon(UI_ICON_ADVANCED));

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

		connect(m_RescanNetworksAction, &QAction::triggered, this, &CWirelessSettings::handleRescanRequest);
		connect(m_ToggleScanResultsAction, &QAction::toggled, ui.availableAPGroupBox, &QGroupBox::setVisible);

		connect(m_SwitchNetworkAction, &QAction::triggered, this, &CWirelessSettings::switchToSelectedNetwork);
		connect(m_ConfigureNetworkAction, &QAction::triggered, this, &CWirelessSettings::configureSelectedNetwork);
		connect(m_EnableNetworkAction, &QAction::triggered, this, &CWirelessSettings::enableSelectedNetwork);
		connect(m_DisableNetworkAction, &QAction::triggered, this, &CWirelessSettings::disableSelectedNetwork);

		connect(enableNetworksAction, &QAction::triggered, this, &CWirelessSettings::enableNetworks);
		connect(importNetworksAction, &QAction::triggered, this, &CWirelessSettings::importNetworks);
		connect(exportNetworkAction, &QAction::triggered, this, &CWirelessSettings::exportSelectedNetwork);
		connect(exportMultipleNetworksAction, &QAction::triggered, this, &CWirelessSettings::exportMultipleNetworks);

		connect(addNetworkAction, &QAction::triggered, this, &CWirelessSettings::addNetwork);
		connect(addAdhocAction, &QAction::triggered, this, &CWirelessSettings::addAdhoc);
		connect(m_RemoveNetworkAction, &QAction::triggered, this, &CWirelessSettings::removeSelectedNetwork);

		connect(m_SaveNetworksAction, &QAction::triggered, m_Device->getWireless()->getWpaSupplicant(), &CWpaSupplicant::save_config);
		connect(reloadNetworksAction, &QAction::triggered, m_Device->getWireless()->getWpaSupplicant(), &CWpaSupplicant::reconfigure);
		connect(m_KeepScanResultsAction, &QAction::toggled, this, &CWirelessSettings::keepScanResultsVisible);
		connect(reassociateAction, &QAction::triggered, m_Device->getWireless()->getWpaSupplicant(), &CWpaSupplicant::reassociate);

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
		ui.manageNetworksButton->setIcon(manageNetworksMenu->icon());
		ui.manageNetworksButton->setPopupMode(QToolButton::InstantPopup);

		ui.addNetworkButton->setDefaultAction(addNetworkAction);
		ui.addAdhocButton->setDefaultAction(addAdhocAction);
		ui.rescanNetworksButton->setDefaultAction(m_RescanNetworksAction);

		ui.advancedFunctionsButton->setMenu(advancedFuntionsMenu);
		ui.advancedFunctionsButton->setText(advancedFuntionsMenu->title());
		ui.advancedFunctionsButton->setIcon(advancedFuntionsMenu->icon());
		ui.advancedFunctionsButton->setPopupMode(QToolButton::InstantPopup);

		ui.reassociateButton->setDefaultAction(reassociateAction);

		m_SetBSSIDMapper = new QSignalMapper(this);
		connect(m_SetBSSIDMapper, static_cast<void(QSignalMapper::*)(const QString &)>(&QSignalMapper::mapped), this, &CWirelessSettings::handleBSSIDSwitchRequest);
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
		ui.managedAvailableAPSplitter->setEnabled(state >= DeviceState::ACTIVATED && m_Device->getWireless());
		ui.actionsWidget->setEnabled(state >= DeviceState::ACTIVATED && m_Device->getWireless());
		ui.signalFrame->setEnabled(state > DeviceState::ACTIVATED && m_Device->getWireless());

		ui.iconLabel->setPixmap(QPixmap(iconFile(m_Device)));
		ui.stateLabel->setText(toStringTr(state));

		if (state <= DeviceState::ACTIVATED) {
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
		QModelIndex const index = selectedIndex(ui.availableView);
		ScanResult const* const scan = m_AvailableAPModel->scanResultByModelIndex(index);

		bool accepted = false;
		if (scan) {
			if (scan->opmode == OPM_ADHOC) {
				CAdhocConfig dialog(m_Device->getWireless(), this);
				accepted = dialog.execute(*scan);
			}
			else {
				CAccessPointConfig dialog(m_Device->getWireless(), this);
				accepted = dialog.execute(*scan);
			}
		}
		else {
			CAccessPointConfig dialog(m_Device->getWireless(), this);
			accepted = dialog.execute();
		}
		if (accepted && m_AutoSaveNetworksAction->isChecked()) {
			m_SaveNetworksAction->trigger();
		}
	}

	void CWirelessSettings::addAdhoc() {
		CAdhocConfig dialog(m_Device->getWireless(), this);
		if (dialog.execute() && m_AutoSaveNetworksAction->isChecked()) {
			m_SaveNetworksAction->trigger();
		}
	}

	void CWirelessSettings::switchToSelectedNetwork() {
		ShortNetworkInfo const* const network = m_ManagedAPModel->networkInfoByModelIndex(selectedIndex(ui.managedView));
		if (!network) return;

		m_Device->getWireless()->getWpaSupplicant()->selectNetwork(network->id);
		if (m_AutoSaveNetworksAction->isChecked()) {
			m_SaveNetworksAction->trigger();
		}
	}

	void CWirelessSettings::removeSelectedNetwork() {
		ShortNetworkInfo const* const network = m_ManagedAPModel->networkInfoByModelIndex(selectedIndex(ui.managedView));
		if (!network) return;

		QString message = tr("Are you sure to remove \"%1\"?").arg(network->ssid);
		if (m_AutoSaveNetworksAction->isChecked()) {
			message += '\n' + tr("This action is not reversible.");
		}

		if (QMessageBox::question(this, tr("Removing a managed network"), message,
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
			m_Device->getWireless()->getWpaSupplicant()->removeNetwork(network->id);
			if (m_AutoSaveNetworksAction->isChecked()) {
				m_SaveNetworksAction->trigger();
			}
		}
	}

	void CWirelessSettings::configureSelectedNetwork() {
		ShortNetworkInfo const* const network = m_ManagedAPModel->networkInfoByModelIndex(selectedIndex(ui.managedView));
		if (!network) return;

		CAbstractWifiNetConfigDialog * dialog;

		if (network->adhoc) {
			dialog = new CAdhocConfig(m_Device->getWireless(), this);
		}
		else {
			dialog = new CAccessPointConfig(m_Device->getWireless(), this);
		}

		if (dialog->execute(network->id) && m_AutoSaveNetworksAction->isChecked()) {
			m_SaveNetworksAction->trigger();
		}

		delete dialog;
	}

	void CWirelessSettings::enableSelectedNetwork() {
		ShortNetworkInfo const* const network = m_ManagedAPModel->networkInfoByModelIndex(selectedIndex(ui.managedView));
		if (!network) return;

		m_Device->getWireless()->getWpaSupplicant()->enableNetwork(network->id);

		if (m_AutoSaveNetworksAction->isChecked()) {
			m_SaveNetworksAction->trigger();
		}
	}

	void CWirelessSettings::disableSelectedNetwork() {
		ShortNetworkInfo const* const network = m_ManagedAPModel->networkInfoByModelIndex(selectedIndex(ui.managedView));
		if (!network) return;

		m_Device->getWireless()->getWpaSupplicant()->disableNetwork(network->id);

		if (m_AutoSaveNetworksAction->isChecked()) {
			m_SaveNetworksAction->trigger();
		}
	}

	void CWirelessSettings::enableNetworks() {
		for (ShortNetworkInfo const& network: m_ManagedAPModel->cachedNetworks()) {
			m_Device->getWireless()->getWpaSupplicant()->enableNetwork(network.id);
		}

		if (m_AutoSaveNetworksAction->isChecked()) {
			m_SaveNetworksAction->trigger();
		}
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
		ShortNetworkInfo const* const network = m_ManagedAPModel->networkInfoByModelIndex(selectedIndex(ui.managedView));
		if (!network) return;

		QString fileName = QFileDialog::getSaveFileName(this, tr("Export network configuration"), QDir::currentPath(), tr("Configuration files (*.apconf *.conf)"));
		if (!fileName.isEmpty()) {
			QFile file(fileName);
			file.open(QFile::WriteOnly);
			QTextStream outStream(&file);

			m_Device->getWireless()->getWpaSupplicant()->getNetworkConfig(network->id).writeTo(outStream);
		}
	}

	void CWirelessSettings::exportMultipleNetworks() {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Export network configuration"), QDir::currentPath(), tr("Configuration files (*.apconf *.conf)"));
		if (!fileName.isEmpty()) {
			QFile file(fileName);
			file.open(QFile::WriteOnly);
			QTextStream outStream(&file);

			for (ShortNetworkInfo const& network: m_ManagedAPModel->cachedNetworks()) {
				m_Device->getWireless()->getWpaSupplicant()->getNetworkConfig(network.id).writeTo(outStream);
			}
		}
	}

	void CWirelessSettings::handleBSSIDSwitchRequest(const QString & data) {
		auto network = m_ManagedAPModel->currentNetworkInfo();
		if (!network) return;

		libnutcommon::MacAddress bssid(data);
		if (bssid.valid()) {
			m_Device->getWireless()->getWpaSupplicant()->setBssid(network->id, bssid);

			m_ManagedAPModel->updateNetworks();
			network = m_ManagedAPModel->currentNetworkInfo();
			if (!network) return;

			m_Device->getWireless()->getWpaSupplicant()->selectNetwork(network->id);

// 			m_Device->getWireless()->getWpaSupplicant()->reassociate();
		}
	}

	inline QString signalQualityToString(const libnutwireless::SignalQuality & signal) {
		return QString::number(signal.quality.value) + '/'+
			QString::number(signal.quality.maximum);
	}

	void CWirelessSettings::updateBSSIDMenu() {
		for (QAction* i: m_SetBSSIDMenu->actions()) {
			m_SetBSSIDMapper->removeMappings(i);
		}
		m_SetBSSIDMenu->clear();

		auto network = m_ManagedAPModel->currentNetworkInfo();

		if (m_Device->getState() > DeviceState::ACTIVATED && network) {
			QString ssid = network->ssid;
			for (auto const scanResult: m_AvailableAPModel->scanResultListBySSID(ssid)) {
				QString bssidString = scanResult->bssid.toString();
				QAction* currentAction = m_SetBSSIDMenu->addAction(
							tr("%1 (Quality: %2)")
							.arg(bssidString, signalQualityToString(scanResult->signal)));
				m_SetBSSIDMapper->setMapping(currentAction, bssidString);
				connect(currentAction, &QAction::triggered, m_SetBSSIDMapper, static_cast<void(QSignalMapper::*)()>(&QSignalMapper::map));
			}
		}

		if (m_SetBSSIDMenu->actions().isEmpty()) {
			QAction* currentAction = m_SetBSSIDMenu->addAction(tr("< no matching scan results >"));
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
