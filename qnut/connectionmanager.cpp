//
// C++ Implementation: connectionmanager
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include <QDate>
#include <libnutclient/cdevice.h>
#include "connectionmanager.h"
#include "overviewmodel.h"
#include "common.h"
#include "constants.h"

namespace qnut {
	using namespace libnutclient;
	using namespace libnutcommon;
	
	CConnectionManager::CConnectionManager(QWidget * parent) :
		QMainWindow(parent),
		m_DeviceManager(this),
		m_LogFile(this, UI_FILE_LOG),
		//TODO change this to "organization-application" parameters
		m_Settings(UI_FILE_CONFIG, QSettings::IniFormat, this),
		m_TrayIcon(this)
	{
		setWindowIcon(m_TrayIcon.icon());
		m_DeviceDetails.reserve(10);
		
		ui.setupUi(this);
		readSettings();
		
		m_LogEdit.setReadOnly(true);
		m_LogEdit.setAcceptRichText(false);
		
		//m_OverView.setSortingEnabled(true);
		m_OverView.setModel(new COverViewModel(&(m_DeviceManager), this));
		m_OverView.setContextMenuPolicy(Qt::ActionsContextMenu);
		m_OverView.setRootIsDecorated(false);
		m_OverView.setItemsExpandable(false);
		m_OverView.setAllColumnsShowFocus(true);
		m_OverView.setIconSize(QSize(32, 32));
		
		m_TabWidget.addTab(&m_OverView, tr("Overview"));
		
		if (ui.actionShowLog->isChecked())
			showLog(true);
		
		ui.centralwidget->layout()->addWidget(&m_TabWidget);
		
		connect(&m_DeviceManager, SIGNAL(deviceAdded(libnutclient::CDevice *)), this, SLOT(addUiDevice(libnutclient::CDevice *)));
		connect(&m_DeviceManager, SIGNAL(deviceAdded(libnutclient::CDevice *)), this, SLOT(updateTrayIconInfo()));
		connect(&m_DeviceManager, SIGNAL(deviceRemoved(libnutclient::CDevice *)), this, SLOT(removeUiDevice(libnutclient::CDevice *)));
		connect(&m_DeviceManager, SIGNAL(deviceRemoved(libnutclient::CDevice *)), this, SLOT(updateTrayIconInfo()));
		
		if (m_LogFile.error() != QFile::NoError)
			m_LogEdit.append(tr("ERROR: %1").arg(tr("Cannot create/open log file.")));
		
		connect(&m_LogFile, SIGNAL(printed(const QString &)), &m_LogEdit, SLOT(append(const QString &)));
		
		m_LogFile << tr("%1 (v%2) started").arg(UI_NAME, UI_VERSION);
		m_LogFile << QDateTime::currentDateTime().toString();
		
		createActions();
		distributeActions();
		ui.toolBar->addActions(ui.menuEdit->actions());
		
		connect(ui.actionShowLog, SIGNAL(toggled(bool)), this, SLOT(showLog(bool)));
		connect(ui.actionAboutQt, SIGNAL(triggered()), qApp , SLOT(aboutQt()));
		connect(ui.actionAboutQNUT, SIGNAL(triggered()), this , SLOT(showAbout()));
		connect(&m_TabWidget, SIGNAL(currentChanged(int)), this, SLOT(handleTabChanged(int)));
		connect(&m_TrayIcon, SIGNAL(messageClicked()), this, SLOT(show()));
		connect(m_OverView.selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(handleSelectionChanged(const QItemSelection &, const QItemSelection &)));
		connect(&m_OverView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(showDeviceDetails(const QModelIndex &)));
		
		m_TrayIcon.show();
		m_DeviceManager.init(&m_LogFile);
	}
	
	CConnectionManager::~CConnectionManager() {
		writeSettings();
	}
	
	inline void CConnectionManager::createActions() {
		//overViewMenu Actions
		m_RefreshDevicesAction   = new QAction(QIcon(UI_ICON_RELOAD), tr("&Refresh devices"), this);
		m_EnableDeviceAction     = new QAction(QIcon(UI_ICON_ENABLE), tr("&Enable"), this);
		m_DisableDeviceAction    = new QAction(QIcon(UI_ICON_DISABLE), tr("&Disable"), this);
		m_DeviceSettingsAction   = new QAction(QIcon(UI_ICON_SCRIPT_SETTINGS), tr("&Scripting settings..."), this);
		#ifndef QNUT_NO_WIRELESS
		m_WirelessSettingsAction = new QAction(QIcon(UI_ICON_AIR), tr("&Wireless settings..."), this);
		#endif
		m_ClearLogAction         = new QAction(QIcon(UI_ICON_CLEAR), tr("&Clear log"), this);
		
		m_EnableDeviceAction->setEnabled(false);
		m_DisableDeviceAction->setEnabled(false);
		m_DeviceSettingsAction->setEnabled(false);
		#ifndef QNUT_NO_WIRELESS
		m_WirelessSettingsAction->setEnabled(false);
		#endif
		
		m_OverView.addAction(m_RefreshDevicesAction);
		m_OverView.addAction(getSeparator(this));
		m_OverView.addAction(m_EnableDeviceAction);
		m_OverView.addAction(m_DisableDeviceAction);
		m_OverView.addAction(getSeparator(this));
		m_OverView.addAction(m_DeviceSettingsAction);
		#ifndef QNUT_NO_WIRELESS
		m_OverView.addAction(m_WirelessSettingsAction);
		#endif
		
		connect(m_RefreshDevicesAction, SIGNAL(triggered()), &m_DeviceManager, SLOT(refreshAll()));
		connect(m_ClearLogAction, SIGNAL(triggered()), &m_LogEdit, SLOT(clear()));
	}
	
	inline void CConnectionManager::distributeActions(int mode) {
		switch (mode) {
		case UI_ACTIONS_OVERVIEW:
			//general device actions
			ui.menuEdit->addActions(m_OverView.actions());
			break;
		case UI_ACTIONS_LOG:
			ui.menuEdit->addAction(m_RefreshDevicesAction);
			ui.menuEdit->addSeparator();
			ui.menuEdit->addAction(m_ClearLogAction);
			break;
		case UI_ACTIONS_DEVICE: {
				ui.menuEdit->addAction(m_RefreshDevicesAction);
				ui.menuEdit->addSeparator();
				
				CDeviceDetails * current = (CDeviceDetails *)(m_TabWidget.currentWidget());
				
				//current device actions
				ui.menuEdit->addActions(current->deviceActions());
				ui.menuEdit->addSeparator();
				ui.menuEdit->addActions(current->environmentTreeActions());
			}
			break;
		default:
			break;
		}
	}
	
	inline void CConnectionManager::readSettings() {
		m_Settings.beginGroup("Main");
		ui.actionShowBalloonTips->setChecked(m_Settings.value("showBalloonTips", true).toBool());
		ui.actionShowLog->setChecked(m_Settings.value("showLog", true).toBool());
		m_Settings.endGroup();
		
		m_Settings.beginGroup("ConnectionManager");
		resize(m_Settings.value("size", QSize(646, 322)).toSize());
		move(m_Settings.value("pos", QPoint(200, 200)).toPoint());
		m_Settings.endGroup();
	}
	
	inline void CConnectionManager::writeSettings() {
		m_Settings.beginGroup("Main");
		m_Settings.setValue("showBalloonTips", ui.actionShowBalloonTips->isChecked());
		m_Settings.setValue("showLog", ui.actionShowLog->isChecked());
		m_Settings.endGroup();
		
		m_Settings.beginGroup("ConnectionManager");
		m_Settings.setValue("size", size());
		m_Settings.setValue("pos", pos());
		m_Settings.endGroup();
	}
	
	void CConnectionManager::addUiDevice(CDevice * device) {
		CDeviceDetails * newDeviceOptions = new CDeviceDetails(device);
		
		m_TabWidget.insertTab(m_DeviceManager.getDevices().indexOf(device)+1, newDeviceOptions, device->getName());
		
		m_DeviceDetails.insert(device, newDeviceOptions);
		m_TrayIcon.addDeviceMenu(newDeviceOptions->trayMenu());
		
		connect(device, SIGNAL(stateChanged(libnutcommon::DeviceState)), this, SLOT(updateTrayIconInfo()));
		connect(newDeviceOptions, SIGNAL(showDetailsRequested(QWidget *)), this, SLOT(showDeviceDetails(QWidget *)));
		connect(newDeviceOptions, SIGNAL(showMessageRequested(QString, QString, QSystemTrayIcon *)),
		        this,             SLOT(showMessage(QString, QString, QSystemTrayIcon *)));
	}
	
	void CConnectionManager::removeUiDevice(CDevice * device) {
		m_OverView.clearSelection();
		CDeviceDetails * target = m_DeviceDetails[device];
		m_DeviceDetails.remove(device);
		
		m_TabWidget.removeTab(m_TabWidget.indexOf(target));
		m_TrayIcon.removeDeviceMenu(target->trayMenu());
		delete target;
	}
	
	void CConnectionManager::updateTrayIconInfo() {
		QStringList result;
		
		if (m_DeviceManager.getDevices().isEmpty())
			result << tr("no devices present");
		else
			foreach (CDevice * i, m_DeviceManager.getDevices()) {
				result << shortSummary(i);
			}
		
		m_TrayIcon.setToolTip(result.join("\n"));
	}
	
	void CConnectionManager::handleTabChanged(int index) {
		ui.menuEdit->clear();
		
		if (index == 0)
			distributeActions(UI_ACTIONS_OVERVIEW);
		else if ((ui.actionShowLog->isChecked()) && (index == m_TabWidget.count()-1))
			distributeActions(UI_ACTIONS_LOG);
		else
			distributeActions(UI_ACTIONS_DEVICE);
		
		ui.toolBar->setUpdatesEnabled(false);
		ui.toolBar->clear();
		ui.toolBar->addActions(ui.menuEdit->actions());
		ui.toolBar->setUpdatesEnabled(true);
	}
	
	void CConnectionManager::handleSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected) {
		QModelIndexList selectedIndexes = selected.indexes();
		QModelIndexList deselectedIndexes = deselected.indexes();
		
		if (!deselectedIndexes.isEmpty()) {
			CDevice * deselectedDevice = static_cast<CDevice *>(deselectedIndexes[0].internalPointer());
			
			disconnect(deselectedDevice, SIGNAL(stateChanged(libnutcommon::DeviceState)), this, SLOT(handleDeviceStateChange(libnutcommon::DeviceState)));
			disconnect(m_EnableDeviceAction, SIGNAL(triggered()), deselectedDevice, SLOT(enable()));
			disconnect(m_DisableDeviceAction, SIGNAL(triggered()), deselectedDevice, SLOT(disable()));
			disconnect(m_DeviceSettingsAction, SIGNAL(triggered()), m_DeviceDetails[deselectedDevice], SLOT(openScriptingSettings()));
			#ifndef QNUT_NO_WIRELESS
			disconnect(m_WirelessSettingsAction, SIGNAL(triggered()), m_DeviceDetails[deselectedDevice], SLOT(openWirelessSettings()));
			#endif
		}
		
		if (selectedIndexes.isEmpty()) {
			m_EnableDeviceAction->setEnabled(false);
			m_DisableDeviceAction->setEnabled(false);
			m_DeviceSettingsAction->setEnabled(false);
			#ifndef QNUT_NO_WIRELESS
			m_WirelessSettingsAction->setEnabled(false);
			#endif
		}
		else {
			CDevice * selectedDevice = static_cast<CDevice *>(selectedIndexes[0].internalPointer());
			
			connect(selectedDevice, SIGNAL(stateChanged(libnutcommon::DeviceState)), this, SLOT(handleDeviceStateChange(libnutcommon::DeviceState)));
			connect(m_EnableDeviceAction, SIGNAL(triggered()), selectedDevice, SLOT(enable()));
			connect(m_DisableDeviceAction, SIGNAL(triggered()), selectedDevice, SLOT(disable()));
			connect(m_DeviceSettingsAction, SIGNAL(triggered()), m_DeviceDetails[selectedDevice], SLOT(openScriptingSettings()));
			#ifndef QNUT_NO_WIRELESS
			connect(m_WirelessSettingsAction, SIGNAL(triggered()), m_DeviceDetails[selectedDevice], SLOT(openWirelessSettings()));
			#endif
			
			m_EnableDeviceAction->setEnabled(selectedDevice->getState() == DS_DEACTIVATED);
			m_DisableDeviceAction->setDisabled(selectedDevice->getState() == DS_DEACTIVATED);
			m_DeviceSettingsAction->setEnabled(true);
			
			#ifndef QNUT_NO_WIRELESS
			m_WirelessSettingsAction->setEnabled(selectedDevice->getWpaSupplicant());
			#endif
		}
	}
	
	void CConnectionManager::showMessage(QString title, QString message, QSystemTrayIcon * trayIcon) {
		if (ui.actionShowBalloonTips->isChecked()) {
			if (trayIcon)
				trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 4000);
			else
				m_TrayIcon.showMessage(title, message, QSystemTrayIcon::Information, 4000);
		}
	}
	
	void CConnectionManager::showAbout() {
		QMessageBox aboutBox(this);
		
		aboutBox.setIconPixmap(QPixmap(UI_ICON_QNUT));
		aboutBox.setWindowTitle(tr("About QNUT"));
		aboutBox.setText(UI_NAME + "\nv" + QString(UI_VERSION));
		aboutBox.setStandardButtons(QMessageBox::Ok);
		
		aboutBox.exec();
	}
	
	void CConnectionManager::handleDeviceStateChange(DeviceState state) {
		m_EnableDeviceAction->setEnabled(state == DS_DEACTIVATED);
		m_DisableDeviceAction->setDisabled(state == DS_DEACTIVATED);
	}
	
	void CConnectionManager::showLog(bool doShow) {
		if (doShow)
			m_TabWidget.addTab(&m_LogEdit, tr("Log"));
		else
			m_TabWidget.removeTab(m_TabWidget.count()-1);
	}
	
	void CConnectionManager::showDeviceDetails(QWidget * widget) {
		show();
		activateWindow();
		m_TabWidget.setCurrentWidget(widget);
	}
	
	void CConnectionManager::showDeviceDetails(const QModelIndex & index) {
		CDevice * selectedDevice = static_cast<CDevice *>(index.internalPointer());
		showDeviceDetails(m_DeviceDetails[selectedDevice]);
	}
}
