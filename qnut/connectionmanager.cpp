#include <QDate>
#include "connectionmanager.h"
#include "common.h"

namespace qnut {
	CConnectionManager::CConnectionManager(QWidget * parent) :
		QMainWindow(parent),
		deviceManager(this),
		logFile(this, UI_FILE_LOG),
		trayicon(this),
		settings(UI_FILE_CONFIG, QSettings::IniFormat, this)
	{
		setWindowIcon(trayicon.icon());
		deviceOptions.reserve(10);
		ui.setupUi(this);
		
		overView.setModel(new COverViewModel(&(deviceManager.devices)));
		overView.setContextMenuPolicy(Qt::ActionsContextMenu);
		overView.setRootIsDecorated(false);
		overView.setItemsExpandable(false);
		overView.setAllColumnsShowFocus(true);
		overView.setIconSize(QSize(32, 32));
		
		logEdit.setReadOnly(true);
		logEdit.setAcceptRichText(false);
		
		tabWidget.addTab(&overView, tr("Overview"));
		tabWidget.addTab(&logEdit, tr("Log"));
		
		ui.centralwidget->layout()->addWidget(&tabWidget);
		
		connect(&logFile, SIGNAL(printed(const QString &)), &logEdit, SLOT(append(const QString &)));
		connect(&deviceManager, SIGNAL(deviceAdded(CDevice *)), this, SLOT(uiAddedDevice(CDevice *)));
		connect(&deviceManager, SIGNAL(deviceAdded(CDevice *)), this, SLOT(uiUpdateTrayIconInfo()));
		connect(&deviceManager, SIGNAL(deviceRemoved(CDevice *)), this, SLOT(uiRemovedDevice(CDevice *)));
		connect(&deviceManager, SIGNAL(deviceRemoved(CDevice *)), this, SLOT(uiUpdateTrayIconInfo()));
		
		if (logFile.error() != QFile::NoError)
			logEdit.append(tr("ERROR:") + " " + tr("Cannot create/open log file."));
		
		logFile << UI_NAME + " (v" + QString(UI_VERSION) + ") " + tr("started");
		logFile << QDateTime::currentDateTime().toString();
		
		createActions();
		
		try {
			deviceManager.init(&logFile);
		}
		catch (Exception & e) {
			logFile << tr("ERROR") + ": " + QString(e.what());
			refreshDevicesAction->setEnabled(false);
		}
		
		distributeActions();
		
		ui.toolBar->addActions(overView.actions());
		ui.menuDevice->addActions(overView.actions());
		
		readSettings();
		
		connect(refreshDevicesAction, SIGNAL(triggered()), &deviceManager, SLOT(rebuild()));
		connect(refreshDevicesAction, SIGNAL(triggered()), &overView, SLOT(reset()));
		connect(ui.actionShowBalloonTips, SIGNAL(toggled(bool)), this, SLOT(uiSetShowBalloonTips(bool)));
		connect(ui.actionAboutQt, SIGNAL(triggered()), qApp , SLOT(aboutQt()));
		connect(ui.actionAboutQNUT, SIGNAL(triggered()), this , SLOT(uiShowAbout()));
		connect(overView.selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
		        this                     , SLOT(uiSelectedDeviceChanged(const QItemSelection &, const QItemSelection &)));
		connect(&tabWidget, SIGNAL(currentChanged(int)), this, SLOT(uiCurrentTabChanged(int)));
		connect(&trayicon, SIGNAL(messageClicked()), this, SLOT(show()));
		
		trayicon.show();
	}
	
	CConnectionManager::~CConnectionManager() {
		writeSettings();
	}
	
	void CConnectionManager::createActions() {
		//overViewMenu Actions
		refreshDevicesAction  = new QAction(QIcon(UI_ICON_REFRESH), tr("Refresh devices"), &overView);
		enableDeviceAction    = new QAction(QIcon(UI_ICON_DEVICE_ENABLE), tr("Enable device"), &overView);
		disableDeviceAction   = new QAction(QIcon(UI_ICON_DEVICE_DISABLE), tr("Disable device"), &overView);
		deviceSettingsAction  = new QAction(QIcon(UI_ICON_DEVICE_SETTINGS), tr("General device settings..."), &overView);
		ipConfigurationAction = new QAction(QIcon(UI_ICON_EDIT), tr("Set IP Configuration..."), &overView);
		
		enableDeviceAction->setEnabled(false);
		disableDeviceAction->setEnabled(false);
		deviceSettingsAction->setEnabled(false);
		ipConfigurationAction->setEnabled(false);
		
		overView.addAction(refreshDevicesAction);
		overView.addAction(getSeparator(&overView));
		overView.addAction(enableDeviceAction);
		overView.addAction(disableDeviceAction);
		overView.addAction(getSeparator(&overView));
		overView.addAction(deviceSettingsAction);
		overView.addAction(ipConfigurationAction);
	}
	
	void CConnectionManager::distributeActions(int mode) {
		switch (mode) {
		case 0: {
			//general device actions
			//ui.toolBar->addActions(overView.actions());
			ui.menuDevice->addActions(overView.actions());
			}
			break;
		case 1: {
			//ui.toolBar->addAction(refreshDevicesAction);
			ui.menuDevice->addAction(refreshDevicesAction);
			}
			break;
		case 2: {
			ui.menuDevice->addAction(refreshDevicesAction);
			ui.menuDevice->addSeparator();
			
			CDeviceOptions * current = (CDeviceOptions *)(tabWidget.currentWidget());
			
			//current device actions
			ui.menuDevice->addAction(current->enableDeviceAction);
			ui.menuDevice->addAction(current->disableDeviceAction);
			ui.menuDevice->addSeparator();
			ui.menuDevice->addAction(current->deviceSettingsAction);
			ui.menuDevice->addAction(current->ipConfigurationAction);
			ui.menuDevice->addSeparator();
			ui.menuDevice->addAction(current->enterEnvironmentAction);
			//interface actions
			//...
			//ui.toolBar->addActions(ui.menuDevice->actions());
			}
			break;
		default:
			break;
		}
	}
	
	void CConnectionManager::readSettings() {
		settings.beginGroup("Main");
		showBalloonTips = settings.value("showBalloonTips", true).toBool();
		ui.actionShowBalloonTips->setChecked(showBalloonTips);
		settings.endGroup();
		
		settings.beginGroup("ConnectionManager");
		resize(settings.value("size", QSize(646, 322)).toSize());
		move(settings.value("pos", QPoint(200, 200)).toPoint());
		settings.endGroup();
	}
	
	void CConnectionManager::writeSettings() {
		settings.beginGroup("Main");
		settings.setValue("showBalloonTips", showBalloonTips);
		settings.endGroup();
		
		settings.beginGroup("ConnectionManager");
		settings.setValue("size", size());
		settings.setValue("pos", pos());
		settings.endGroup();
	}
	
	void CConnectionManager::uiAddedDevice(CDevice * dev) {
		CDeviceOptions * newDeviceOptions = new CDeviceOptions(dev, &tabWidget);
		
		tabWidget.insertTab(tabWidget.count()-1, newDeviceOptions, dev->name);
		
		newDeviceOptions->updateDeviceIcons();
		
		deviceOptions.insert(dev, newDeviceOptions);
		trayicon.devicesMenu.addMenu(newDeviceOptions->deviceMenu);
		trayicon.devicesMenu.setEnabled(true);
		
		connect(dev, SIGNAL(stateChanged(DeviceState)), &overView, SLOT(reset()));
		connect(dev, SIGNAL(stateChanged(DeviceState)), this, SLOT(uiUpdateTrayIconInfo()));
		//connect(newDeviceOptions->showAction, SIGNAL(triggered()), this, SLOT(show()));
		connect(newDeviceOptions, SIGNAL(showOptions(QWidget *)), ui.toolBar, SLOT(setCurrentWidget(QWidget *)));
		connect(newDeviceOptions, SIGNAL(showMessage(QSystemTrayIcon *, QString, QString)),
		        this,             SLOT(uiShowMessage(QSystemTrayIcon *, QString, QString)));
		overView.reset();
		overView.clearSelection();
	}
	
	void CConnectionManager::uiRemovedDevice(CDevice * dev) {
		overView.reset();
		disconnect(dev, SIGNAL(stateChanged(DeviceState)), this, SLOT(uiUpdateTrayIconInfo()));
		disconnect(dev, SIGNAL(stateChanged(DeviceState)), &overView, SLOT(reset()));
		CDeviceOptions * target = deviceOptions[dev];
		
		tabWidget.removeTab(tabWidget.indexOf(target));
		trayicon.devicesMenu.removeAction(target->deviceMenu->menuAction());
		trayicon.devicesMenu.setDisabled(deviceManager.devices.isEmpty());
		
		enableDeviceAction->setDisabled(deviceManager.devices.isEmpty());
		disableDeviceAction->setDisabled(deviceManager.devices.isEmpty());
		
		deviceOptions.remove(dev);
		delete target;
	}
	
	void CConnectionManager::uiUpdateTrayIconInfo() {
		QStringList result;
		
		if (deviceManager.devices.isEmpty())
			result << tr("no devcies present");
		else
			foreach (CDevice * i, deviceManager.devices) {
				result << shortSummary(i);
			}
		
		trayicon.setToolTip(result.join("\n"));
	}
	
	void CConnectionManager::uiCurrentTabChanged(int index) {
		ui.menuDevice->clear();
		if (index == 0) {
			distributeActions(0);
		}
		else if (index == tabWidget.count()-1) {
			distributeActions(1);
		}
		else {
			distributeActions(2);
		}
		
		ui.toolBar->setUpdatesEnabled(false);
		ui.toolBar->clear();
		ui.toolBar->addActions(ui.menuDevice->actions());
		ui.toolBar->setUpdatesEnabled(true);
		
		if (ui.toolBar->actions().isEmpty())
			logFile << "BUG!";
	}
	
	void CConnectionManager::uiSelectedDeviceChanged(const QItemSelection & selected, const QItemSelection & deselected) {
		QModelIndexList selectedIndexes = selected.indexes();
		QModelIndexList deselectedIndexes = deselected.indexes();
		
		if (!deselectedIndexes.isEmpty()) {
			CDevice * deselectedDevice = (CDevice *)(deselectedIndexes[0].internalPointer());
			disconnect(deselectedDevice, SIGNAL(stateChanged(DeviceState)), this, SLOT(uiHandleDeviceStateChanged(DeviceState)));
			disconnect(enableDeviceAction, SIGNAL(triggered()), deselectedDevice, SLOT(enable()));
			disconnect(disableDeviceAction, SIGNAL(triggered()), deselectedDevice, SLOT(disable()));
			disconnect(deviceSettingsAction, SIGNAL(triggered()), deviceOptions[deselectedDevice], SLOT(uiChangeDeviceSettings()));
			disconnect(ipConfigurationAction, SIGNAL(triggered()), deviceOptions[deselectedDevice], SLOT(uiChangeIPConfiguration()));
		}
		
		if (!selectedIndexes.isEmpty()) {
			CDevice * selectedDevice = (CDevice *)(selectedIndexes[0].internalPointer());
			connect(selectedDevice, SIGNAL(stateChanged(DeviceState)), this, SLOT(uiHandleDeviceStateChanged(DeviceState)));
			connect(enableDeviceAction, SIGNAL(triggered()), selectedDevice, SLOT(enable()));
			connect(disableDeviceAction, SIGNAL(triggered()), selectedDevice, SLOT(disable()));
			connect(deviceSettingsAction, SIGNAL(triggered()), deviceOptions[selectedDevice], SLOT(uiChangeDeviceSettings()));
			connect(ipConfigurationAction, SIGNAL(triggered()), deviceOptions[selectedDevice], SLOT(uiChangeIPConfiguration()));
			
			enableDeviceAction->setDisabled(selectedDevice->state == DS_UP);
			disableDeviceAction->setDisabled(selectedDevice->state == DS_DEACTIVATED);
			deviceSettingsAction->setEnabled(true);
			ipConfigurationAction->setEnabled(selectedDevice->state == DS_UNCONFIGURED);
		}
		else {
			enableDeviceAction->setEnabled(false);
			disableDeviceAction->setEnabled(false);
			deviceSettingsAction->setEnabled(false);
			ipConfigurationAction->setEnabled(false);
		}
	}
	
	void CConnectionManager::uiShowMessage(QSystemTrayIcon * trayIcon, QString title, QString message) {
		if (showBalloonTips) {
			if (trayIcon)
				trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 4000);
			else
				trayicon.showMessage(title, message, QSystemTrayIcon::Information, 4000);
		}
	}
	
	void CConnectionManager::uiShowAbout() {
		QMessageBox::about(this, tr("About QNUT"), UI_NAME + "\nv" + QString(UI_VERSION));
	}
	
	void CConnectionManager::uiHandleDeviceStateChanged(DeviceState state) {
		enableDeviceAction->setDisabled(state == DS_UP);
		disableDeviceAction->setDisabled(state == DS_DEACTIVATED);
	}
	
	void CConnectionManager::uiSetShowBalloonTips(bool value) {
		showBalloonTips = value;
	}
};
