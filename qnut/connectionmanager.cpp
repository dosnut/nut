//
// C++ Implementation: connectionmanager
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include <QDate>
#include <QToolBar>
#include <QMenu>
#include <QAction>
#include <QMenuBar>
#include <QMessageBox>
#include <QTreeView>
#include <QTextEdit>
#include <QSortFilterProxyModel>
#include <libnutclient/cdevice.h>
#include <libnutwireless/cwireless.h>

#include "connectionmanager.h"

#include "common.h"
#include "constants.h"
#include "cuidevicemodel.h"
#include "cuidevice.h"
#include "cnotificationmanager.h"

namespace qnut {
	using namespace libnutclient;
	using namespace libnutcommon;
	
	CConnectionManager::CConnectionManager(QWidget * parent) :
		QMainWindow(parent),
		m_LogFile(this, UI_FILE_LOG)
	{
		m_DeviceManager = new CDeviceManager(this);
		if (CNotificationManager::trayIconsAvailable()) {
			m_NotificationManager = new CNotificationManager(this);
		}
		else
			m_NotificationManager = NULL;
		
		resize(600, 322);
		setWindowIcon(QIcon(UI_ICON_QNUT_SMALL));
		
		m_ToolBar = new QToolBar(tr("Main Toolbar"), this);
		m_ToolBar->setObjectName("MainToolbar");
		
		m_LogEdit = new QTextEdit(this);
		m_LogEdit->hide();
		m_LogEdit->setReadOnly(true);
		m_LogEdit->setAcceptRichText(false);
		
		m_UIDeviceModel = new CUIDeviceModel(this);
		
		m_OverView = new QTreeView(this);
		
		m_UIDeviceProxyModel = new QSortFilterProxyModel(this);
		m_UIDeviceProxyModel->setSourceModel(m_UIDeviceModel);
		
		m_OverView->setModel(m_UIDeviceProxyModel);
		m_OverView->setSortingEnabled(true);
		m_OverView->sortByColumn(0, Qt::AscendingOrder);
		m_OverView->setContextMenuPolicy(Qt::ActionsContextMenu);
		m_OverView->setRootIsDecorated(false);
		m_OverView->setItemsExpandable(false);
		m_OverView->setAllColumnsShowFocus(true);
		m_OverView->setIconSize(QSize(32, 32));
		
		m_TabWidget = new QTabWidget(this);
		m_TabWidget->addTab(m_OverView, tr("Overview"));
		
		setCentralWidget(m_TabWidget);
		
		createActions();
		readSettings();
		
		if (m_ShowLogAction->isChecked())
			showLog(true);
		
		connect(m_DeviceManager, SIGNAL(deviceAdded(libnutclient::CDevice *)), this, SLOT(addUiDevice(libnutclient::CDevice *)));
		connect(m_DeviceManager, SIGNAL(deviceAdded(libnutclient::CDevice *)), this, SLOT(updateTrayIconInfo()));
		connect(m_DeviceManager, SIGNAL(deviceRemoved(libnutclient::CDevice *)), this, SLOT(removeUiDevice(libnutclient::CDevice *)));
		connect(m_DeviceManager, SIGNAL(deviceRemoved(libnutclient::CDevice *)), this, SLOT(updateTrayIconInfo()));
		
		if (m_LogFile.error() != QFile::NoError)
			m_LogEdit->append(tr("ERROR: %1").arg(tr("Cannot create/open log file.")));
		
		connect(&m_LogFile, SIGNAL(printed(const QString &)), m_LogEdit, SLOT(append(const QString &)));
		
		m_LogFile << tr("%1 (v%2) started").arg(UI_NAME, UI_VERSION);
		m_LogFile << QDateTime::currentDateTime().toString();
		
		setWindowTitle(tr("QNUT - Connection Manager"));
		
		distributeActions();
		
		m_ToolBar->addActions(m_EditMenu->actions());
		
		connect(m_TabWidget, SIGNAL(currentChanged(int)), this, SLOT(handleTabChanged(int)));
		connect(m_OverView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(handleSelectionChanged(const QItemSelection &, const QItemSelection &)));
		connect(m_OverView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(showDeviceDetails(const QModelIndex &)));
		
		CUIDevice::init();
		connect(CUIDevice::showRequestMapper(), SIGNAL(mapped(QWidget*)), this, SLOT(showDeviceDetails(QWidget *)));
		
		m_DeviceManager->init(&m_LogFile);
		if (m_NotificationManager)
			m_NotificationManager->setIconVisible(true, NULL);
		else
			show();
	}
	
	CConnectionManager::~CConnectionManager() {
		writeSettings();
		
		m_TabWidget->clear();
		
		delete m_UIDeviceModel;
		delete m_DeviceManager;
		CUIDevice::cleanup();
	}
	
	inline void CConnectionManager::createActions() {
		QMenu * currentMenu;
		QAction * currentAction;
		
		currentMenu = menuBar()->addMenu(tr("&File"));
		
		if (m_NotificationManager) {
			currentMenu->addAction(tr("&Close"), this, SLOT(close()));
			currentMenu->addSeparator();
		}
		
		currentMenu->addAction(tr("&Quit"), qApp, SLOT(quit()), Qt::CTRL+Qt::Key_Q);
		
		m_EditMenu = menuBar()->addMenu(tr("&Edit"));
		
		currentMenu = menuBar()->addMenu(tr("&View"));
		
		currentAction = m_ToolBar->toggleViewAction();
		currentMenu->addAction(currentAction);
		currentAction->setText(tr("Show %1").arg(currentAction->text()));
		
		currentMenu->addSeparator();
		
		if (m_NotificationManager) {
			m_ShowBalloonTipsAction = currentMenu->addAction(tr("Show &balloon tips"));
			m_ShowBalloonTipsAction->setToolTip(tr("Show balloon tips on certain events like state changes"));
			m_ShowBalloonTipsAction->setCheckable(true);
			connect(m_ShowBalloonTipsAction, SIGNAL(toggled(bool)), m_NotificationManager, SLOT(setNotificationsEnabled(bool)));
		}
		
		m_ShowLogAction = currentMenu->addAction(tr("Show &log"));
		m_ShowLogAction->setToolTip(tr("Show log tab"));
		m_ShowLogAction->setCheckable(true);
		
		currentMenu = menuBar()->addMenu(tr("&Help"));
		
		currentAction = currentMenu->addAction(tr("&About QNUT"), this, SLOT(showAbout()));
		currentAction->setToolTip(tr("Show about dialog of QNUT"));
		
		currentAction = currentMenu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));
		currentAction->setToolTip(tr("Show about dialog of Qt"));
		
		//overViewMenu Actions
		m_RefreshDevicesAction   = new QAction(QIcon(UI_ICON_RELOAD), tr("&Refresh devices"), this);
		m_EnableDeviceAction     = new QAction(QIcon(UI_ICON_ENABLE), tr("&Enable"), this);
		m_DisableDeviceAction    = new QAction(QIcon(UI_ICON_DISABLE), tr("&Disable"), this);
		m_DeviceSettingsAction   = new QAction(QIcon(UI_ICON_CONFIGURE), tr("&Device settings..."), this);
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
		
		m_OverView->addAction(m_RefreshDevicesAction);
		m_OverView->addAction(getSeparator(this));
		m_OverView->addAction(m_EnableDeviceAction);
		m_OverView->addAction(m_DisableDeviceAction);
		m_OverView->addAction(getSeparator(this));
		m_OverView->addAction(m_DeviceSettingsAction);
#ifndef QNUT_NO_WIRELESS
		m_OverView->addAction(m_WirelessSettingsAction);
#endif
		
		connect(m_RefreshDevicesAction, SIGNAL(triggered()), m_DeviceManager, SLOT(refreshAll()));
		connect(m_ClearLogAction, SIGNAL(triggered()), m_LogEdit, SLOT(clear()));
		connect(m_ShowLogAction, SIGNAL(toggled(bool)), this, SLOT(showLog(bool)));
		
		addToolBar(m_ToolBar);
	}
	
	inline void CConnectionManager::distributeActions(int mode) {
		switch (mode) {
		case UI_ACTIONS_OVERVIEW:
			//general device actions
			m_EditMenu->addActions(m_OverView->actions());
			break;
		case UI_ACTIONS_LOG:
			m_EditMenu->addAction(m_RefreshDevicesAction);
			m_EditMenu->addSeparator();
			m_EditMenu->addAction(m_ClearLogAction);
			break;
		case UI_ACTIONS_DEVICE:
			{
				m_EditMenu->addAction(m_RefreshDevicesAction);
				m_EditMenu->addSeparator();
				
				CUIDevice * current = qobject_cast<CUIDevice *>(m_TabWidget->currentWidget());
				
				//current device actions
				m_EditMenu->addActions(current->deviceActions());
				m_EditMenu->addSeparator();
				m_EditMenu->addActions(current->environmentTreeActions());
			}
			break;
		default:
			break;
		}
	}
	
	inline void CConnectionManager::readSettings() {
		QSettings * settings = new QSettings(UI_STRING_ORGANIZATION, UI_STRING_APPNAME);
		
#ifndef QNUT_SETTINGS_NOCOMPAT
		bool readOld = QFile::exists(UI_FILE_CONFIG) && settings->childGroups().isEmpty();
		
		if (readOld) {
			delete settings;
			settings = new QSettings(UI_FILE_CONFIG, QSettings::IniFormat);
		}
#endif
		settings->beginGroup(UI_SETTINGS_MAIN);
		if (m_NotificationManager)
			m_ShowBalloonTipsAction->setChecked(settings->value(UI_SETTINGS_SHOWBALLOONTIPS, true).toBool());
		m_ShowLogAction->setChecked(settings->value(UI_SETTINGS_SHOWLOG, true).toBool());
		settings->endGroup();
		
		settings->beginGroup(UI_SETTINGS_CONNECTIONMANAGER);
		restoreGeometry(settings->value(UI_SETTINGS_GEOMETRY).toByteArray());
		restoreState(settings->value(UI_SETTINGS_WINDOWSTATE).toByteArray());
		settings->endGroup();
		
		delete settings;
	}
	
	inline void CConnectionManager::writeSettings() {
		QSettings settings(UI_STRING_ORGANIZATION, UI_STRING_APPNAME);
		
		settings.beginGroup(UI_SETTINGS_MAIN);
		if (m_NotificationManager)
			settings.setValue(UI_SETTINGS_SHOWBALLOONTIPS, m_ShowBalloonTipsAction->isChecked());
		settings.setValue(UI_SETTINGS_SHOWLOG, m_ShowLogAction->isChecked());
		settings.endGroup();
		
		settings.beginGroup(UI_SETTINGS_CONNECTIONMANAGER);
		settings.setValue(UI_SETTINGS_GEOMETRY, saveGeometry());
		settings.setValue(UI_SETTINGS_WINDOWSTATE, saveState());
		settings.endGroup();
	}
	
	void CConnectionManager::addUiDevice(CDevice * device) {
		CUIDevice * newUIDevice = m_UIDeviceModel->addUIDevice(device);
		
		if (m_NotificationManager) {
			m_NotificationManager->registerUIDevice(newUIDevice);
			connect(device, SIGNAL(stateChanged(libnutcommon::DeviceState)), this, SLOT(updateTrayIconInfo()));
		}
		
		m_TabWidget->insertTab(m_UIDeviceModel->uiDevices().size(), newUIDevice, device->getName());
	}
	
	void CConnectionManager::removeUiDevice(CDevice * device) {
		int targetPos = m_UIDeviceModel->findUIDevice(device);
		if (targetPos == -1)
			return;
		CUIDevice * target = m_UIDeviceModel->uiDevices()[targetPos];
		
		m_TabWidget->removeTab(m_TabWidget->indexOf(target));
		
		if (m_NotificationManager) {
			m_NotificationManager->unregisterUIDevice(target);
			disconnect(device, SIGNAL(stateChanged(libnutcommon::DeviceState)), this, SLOT(updateTrayIconInfo()));
		}
		m_UIDeviceModel->removeUIDevice(targetPos);
	}
	
	void CConnectionManager::updateTrayIconInfo() {
		if (!m_NotificationManager)
			return;
		
		QStringList result;
		
		if (m_DeviceManager->getDevices().isEmpty())
			result << tr("no devices present");
		else
			foreach (CDevice * i, m_DeviceManager->getDevices()) {
				result << shortSummary(i);
			}
		
		m_NotificationManager->setToolTip(result.join("\n"));
	}
	
	void CConnectionManager::handleTabChanged(int index) {
		m_EditMenu->clear();
		
		if (index == 0)
			distributeActions(UI_ACTIONS_OVERVIEW);
		else if ((m_ShowLogAction->isChecked()) && (index == m_TabWidget->count()-1))
			distributeActions(UI_ACTIONS_LOG);
		else
			distributeActions(UI_ACTIONS_DEVICE);
		
		m_ToolBar->setUpdatesEnabled(false);
		m_ToolBar->clear();
		m_ToolBar->addActions(m_EditMenu->actions());
		m_ToolBar->setUpdatesEnabled(true);
	}
	
	void CConnectionManager::handleSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected) {
		QModelIndexList selectedIndexes = selected.indexes();
		QModelIndexList deselectedIndexes = deselected.indexes();
		
		if (!deselectedIndexes.isEmpty()) {
			CUIDevice * deselectedUIDevice = static_cast<CUIDevice *>(m_UIDeviceProxyModel->mapToSource(deselectedIndexes[0]).internalPointer());
			
			disconnect(deselectedUIDevice->device(), SIGNAL(stateChanged(libnutcommon::DeviceState)),
				this, SLOT(handleDeviceStateChange(libnutcommon::DeviceState)));
			disconnect(m_EnableDeviceAction, SIGNAL(triggered()),
				deselectedUIDevice->device(), SLOT(enable()));
			disconnect(m_DisableDeviceAction, SIGNAL(triggered()),
				deselectedUIDevice->device(), SLOT(disable()));
			disconnect(m_DeviceSettingsAction, SIGNAL(triggered()),
				deselectedUIDevice, SLOT(openDeviceSettings()));
#ifndef QNUT_NO_WIRELESS
			disconnect(m_WirelessSettingsAction, SIGNAL(triggered()),
				deselectedUIDevice, SLOT(openWirelessSettings()));
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
			CUIDevice * selectedUIDevice = static_cast<CUIDevice *>(m_UIDeviceProxyModel->mapToSource(selectedIndexes[0]).internalPointer());
			
			connect(selectedUIDevice->device(), SIGNAL(stateChanged(libnutcommon::DeviceState)),
				this, SLOT(handleDeviceStateChange(libnutcommon::DeviceState)));
			connect(m_EnableDeviceAction, SIGNAL(triggered()),
				selectedUIDevice->device(), SLOT(enable()));
			connect(m_DisableDeviceAction, SIGNAL(triggered()),
				selectedUIDevice->device(), SLOT(disable()));
			connect(m_DeviceSettingsAction, SIGNAL(triggered()),
				selectedUIDevice, SLOT(openDeviceSettings()));
#ifndef QNUT_NO_WIRELESS
			connect(m_WirelessSettingsAction, SIGNAL(triggered()),
				selectedUIDevice, SLOT(openWirelessSettings()));
#endif
			
			m_EnableDeviceAction->setEnabled(selectedUIDevice->device()->getState() == DS_DEACTIVATED);
			m_DisableDeviceAction->setDisabled(selectedUIDevice->device()->getState() == DS_DEACTIVATED);
			m_DeviceSettingsAction->setEnabled(true);
			
#ifndef QNUT_NO_WIRELESS
			m_WirelessSettingsAction->setEnabled(selectedUIDevice->device()->getWireless());
#endif
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
			m_TabWidget->addTab(m_LogEdit, tr("Log"));
		else
			m_TabWidget->removeTab(m_TabWidget->count()-1);
	}
	
	void CConnectionManager::showDeviceDetails(QWidget * widget) {
		show();
		activateWindow();
		m_TabWidget->setCurrentWidget(widget);
	}
	
	void CConnectionManager::showDeviceDetails(const QModelIndex & index) {
		CUIDevice * selectedUIDevice = static_cast<CUIDevice *>(m_UIDeviceProxyModel->mapToSource(index).internalPointer());
		showDeviceDetails(selectedUIDevice);
	}
}
