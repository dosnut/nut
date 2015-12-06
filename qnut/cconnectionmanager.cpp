//
// C++ Implementation: CConnectionManager
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include "cconnectionmanager.h"

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

#include "common.h"
#include "constants.h"
#include "modelview/cuidevicemodel.h"
#include "cuidevice.h"
#include "cnotificationmanager.h"

namespace qnut {
	using namespace libnutclient;
	using namespace libnutcommon;

	CConnectionManager::CConnectionManager(QWidget * parent) :
		QMainWindow(parent),
		m_LogFile(this, UI_FILE_LOG)
	{
		auto service = new CNutService(this);
		connect(service, &CNutService::log, &m_LogFile, &libnutclient::CLog::log);

		m_DeviceManager = new CDeviceManager(service);

		resize(600, 322);
		setWindowIcon(QIcon(UI_ICON_QNUT_SMALL));

		m_ToolBar = new QToolBar(tr("Main Toolbar"), this);
		m_ToolBar->setObjectName("MainToolbar");
		m_ToolBar->setToolButtonStyle(Qt::ToolButtonFollowStyle);

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

		if (CNotificationManager::trayIconsAvailable()) {
			m_NotificationManager = new CNotificationManager(this);
			// connect(m_NotificationManager, &CNotificationManager::requestedUIDeviceWidget, m_TabWidget, &QTabWidget::setCurrentWidget);
		}
		else
			m_NotificationManager = NULL;

		createActions();
		readSettings();

		if (m_ShowLogAction->isChecked())
			showLog(true);

		connect(m_DeviceManager, &CDeviceManager::deviceAdded, this, &CConnectionManager::addUiDevice);
		connect(m_DeviceManager, &CDeviceManager::deviceAdded, this, &CConnectionManager::updateTrayIconInfo);
		connect(m_DeviceManager, &CDeviceManager::deviceRemoved, this, &CConnectionManager::removeUiDevice);
		connect(m_DeviceManager, &CDeviceManager::deviceRemoved, this, &CConnectionManager::updateTrayIconInfo);

		if (m_LogFile.error() != QFile::NoError)
			m_LogEdit->append(tr("ERROR: %1").arg(tr("Cannot create/open log file.")));

		connect(&m_LogFile, &libnutclient::CLog::printed, m_LogEdit, &QTextEdit::append);

		m_LogFile << tr("%1 (v%2) started").arg(UI_STRING_NAME_TR, libnutcommon::version());
		m_LogFile << QDateTime::currentDateTime().toString();

		setWindowTitle(tr("QNUT - Connection Manager"));

		distributeActions();

		m_ToolBar->addActions(m_EditMenu->actions());

		connect(m_TabWidget, &QTabWidget::currentChanged, this, &CConnectionManager::handleTabChanged);
		connect(m_OverView->selectionModel(), &QItemSelectionModel::selectionChanged,
			this, &CConnectionManager::handleSelectionChanged);
		connect(m_OverView, &QTreeView::doubleClicked, this, &CConnectionManager::showDeviceDetailsByIndex);

		CUIDevice::init();
		connect(CUIDevice::showRequestMapper(), static_cast<void(QSignalMapper::*)(QWidget*)>(&QSignalMapper::mapped), this, &CConnectionManager::showDeviceDetails);

		if (!m_NotificationManager) {
			show();
		}
	}

	CConnectionManager::~CConnectionManager() {
		writeSettings();

		m_TabWidget->clear();

		m_UIDeviceModel->disconnect(this);
		m_DeviceManager->disconnect(this);

		delete m_UIDeviceModel;
		delete m_DeviceManager;
		if (m_NotificationManager)
			delete m_NotificationManager;
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
			connect(m_ShowBalloonTipsAction, &QAction::toggled, m_NotificationManager, &CNotificationManager::setNotificationsEnabled);
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
#ifndef NUT_NO_WIRELESS
		m_WirelessSettingsAction = new QAction(QIcon(UI_ICON_AP), tr("&Wireless settings..."), this);
#endif
		m_ClearLogAction         = new QAction(QIcon(UI_ICON_CLEAR), tr("&Clear log"), this);

		m_EnableDeviceAction->setEnabled(false);
		m_DisableDeviceAction->setEnabled(false);
		m_DeviceSettingsAction->setEnabled(false);
#ifndef NUT_NO_WIRELESS
		m_WirelessSettingsAction->setEnabled(false);
#endif

		m_OverView->addAction(m_RefreshDevicesAction);
		m_OverView->addAction(getSeparator(this));
		m_OverView->addAction(m_EnableDeviceAction);
		m_OverView->addAction(m_DisableDeviceAction);
		m_OverView->addAction(getSeparator(this));
		m_OverView->addAction(m_DeviceSettingsAction);
#ifndef NUT_NO_WIRELESS
		m_OverView->addAction(m_WirelessSettingsAction);
#endif

		connect(m_RefreshDevicesAction, &QAction::triggered, m_DeviceManager, &CDeviceManager::refreshAll);
		connect(m_ClearLogAction, &QAction::triggered, m_LogEdit, &QTextEdit::clear);
		connect(m_ShowLogAction, &QAction::toggled, this, &CConnectionManager::showLog);

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
			connect(device, &CDevice::stateChanged, this, &CConnectionManager::updateTrayIconInfo);
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
			disconnect(device, &CDevice::stateChanged, this, &CConnectionManager::updateTrayIconInfo);
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

			disconnect(deselectedUIDevice->device(), &CDevice::stateChanged,
				this, &CConnectionManager::handleDeviceStateChange);
			disconnect(m_EnableDeviceAction, &QAction::triggered,
				deselectedUIDevice->device(), &CDevice::enable);
			disconnect(m_DisableDeviceAction, &QAction::triggered,
				deselectedUIDevice->device(), &CDevice::disable);
			disconnect(m_DeviceSettingsAction, &QAction::triggered,
				deselectedUIDevice, &CUIDevice::openDeviceSettings);
#ifndef NUT_NO_WIRELESS
			disconnect(m_WirelessSettingsAction, &QAction::triggered,
				deselectedUIDevice, &CUIDevice::openWirelessSettings);
#endif
		}

		if (selectedIndexes.isEmpty()) {
			m_EnableDeviceAction->setEnabled(false);
			m_DisableDeviceAction->setEnabled(false);
			m_DeviceSettingsAction->setEnabled(false);
#ifndef NUT_NO_WIRELESS
			m_WirelessSettingsAction->setEnabled(false);
#endif
		}
		else {
			CUIDevice * selectedUIDevice = static_cast<CUIDevice *>(m_UIDeviceProxyModel->mapToSource(selectedIndexes[0]).internalPointer());

			connect(selectedUIDevice->device(), &CDevice::stateChanged,
				this, &CConnectionManager::handleDeviceStateChange);
			connect(m_EnableDeviceAction, &QAction::triggered,
				selectedUIDevice->device(), &CDevice::enable);
			connect(m_DisableDeviceAction, &QAction::triggered,
				selectedUIDevice->device(), &CDevice::disable);
			connect(m_DeviceSettingsAction, &QAction::triggered,
				selectedUIDevice, &CUIDevice::openDeviceSettings);
#ifndef NUT_NO_WIRELESS
			connect(m_WirelessSettingsAction, &QAction::triggered,
				selectedUIDevice, &CUIDevice::openWirelessSettings);
#endif

			m_EnableDeviceAction->setEnabled(selectedUIDevice->device()->getState() == DeviceState::DEACTIVATED);
			m_DisableDeviceAction->setDisabled(selectedUIDevice->device()->getState() == DeviceState::DEACTIVATED);
			m_DeviceSettingsAction->setEnabled(true);

#ifndef NUT_NO_WIRELESS
			m_WirelessSettingsAction->setEnabled(selectedUIDevice->device()->getWireless());
#endif
		}
	}

	void CConnectionManager::showAbout() {
		QMessageBox aboutBox(this);

		aboutBox.setIconPixmap(QPixmap(UI_ICON_QNUT));
		aboutBox.setWindowTitle(tr("About QNUT"));
		aboutBox.setText(UI_STRING_NAME_TR + "\nv" + libnutcommon::version());
		aboutBox.setStandardButtons(QMessageBox::Ok);

		aboutBox.exec();
	}

	void CConnectionManager::handleDeviceStateChange(DeviceState state) {
		m_EnableDeviceAction->setEnabled(state == DeviceState::DEACTIVATED);
		m_DisableDeviceAction->setDisabled(state == DeviceState::DEACTIVATED);
	}

	void CConnectionManager::showLog(bool doShow) {
		if (doShow)
			m_TabWidget->addTab(m_LogEdit, tr("Log"));
		else
			m_TabWidget->removeTab(m_TabWidget->count()-1);
	}

	void CConnectionManager::showDeviceDetails(QWidget * widget) {
		m_TabWidget->setCurrentWidget(widget);
		show();
		activateWindow();
	}

	void CConnectionManager::showDeviceDetailsByIndex(const QModelIndex & index) {
		CUIDevice * selectedUIDevice = static_cast<CUIDevice *>(m_UIDeviceProxyModel->mapToSource(index).internalPointer());
		m_TabWidget->setCurrentWidget(selectedUIDevice);
	}
}
