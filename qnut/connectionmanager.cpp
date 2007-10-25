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
		readSettings();
		
		logEdit.setReadOnly(true);
		logEdit.setAcceptRichText(false);
		
		//overView.setSortingEnabled(true);
		overView.setModel(new COverViewModel(&(deviceManager)));
		overView.setContextMenuPolicy(Qt::ActionsContextMenu);
		overView.setRootIsDecorated(false);
		overView.setItemsExpandable(false);
		overView.setAllColumnsShowFocus(true);
		overView.setIconSize(QSize(32, 32));
		
		tabWidget.addTab(&overView, tr("Overview"));
		
		if (ui.actionShowLog->isChecked())
			uiHandleShowLogToggle(true);
		
		ui.centralwidget->layout()->addWidget(&tabWidget);
		
		connect(&deviceManager, SIGNAL(deviceAdded(CDevice *)), this, SLOT(uiAddedDevice(CDevice *)));
		connect(&deviceManager, SIGNAL(deviceAdded(CDevice *)), this, SLOT(uiUpdateTrayIconInfo()));
		connect(&deviceManager, SIGNAL(deviceRemoved(CDevice *)), this, SLOT(uiRemovedDevice(CDevice *)));
		connect(&deviceManager, SIGNAL(deviceRemoved(CDevice *)), this, SLOT(uiUpdateTrayIconInfo()));
		
		if (logFile.error() != QFile::NoError)
			logEdit.append(tr("ERROR:") + " " + tr("Cannot create/open log file."));
		
		connect(&logFile, SIGNAL(printed(const QString &)), &logEdit, SLOT(append(const QString &)));
		
		logFile << UI_NAME + " (v" + QString(UI_VERSION) + ") " + tr("started");
		logFile << QDateTime::currentDateTime().toString();
		
		createActions();
		distributeActions();
		ui.toolBar->addActions(ui.menuDevice->actions());
		
		connect(ui.actionShowLog, SIGNAL(toggled(bool)), this, SLOT(uiHandleShowLogToggle(bool)));
		connect(ui.actionAboutQt, SIGNAL(triggered()), qApp , SLOT(aboutQt()));
		connect(ui.actionAboutQNUT, SIGNAL(triggered()), this , SLOT(uiShowAbout()));
		connect(refreshDevicesAction, SIGNAL(triggered()), &deviceManager, SLOT(rebuild()));
		connect(&tabWidget, SIGNAL(currentChanged(int)), this, SLOT(uiCurrentTabChanged(int)));
		connect(&trayicon, SIGNAL(messageClicked()), this, SLOT(show()));
		connect(overView.selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(uiSelectedDeviceChanged(const QItemSelection &, const QItemSelection &)));
		
		trayicon.show();
		try {
			deviceManager.init(&logFile);
		}
		catch (Exception & e) {
			logFile << tr("ERROR") + ": " + QString(e.what());
			refreshDevicesAction->setEnabled(false);
		}
		
	}
	
	CConnectionManager::~CConnectionManager() {
		writeSettings();
	}
	
	inline void CConnectionManager::createActions() {
		//overViewMenu Actions
		refreshDevicesAction   = new QAction(QIcon(UI_ICON_REFRESH), tr("Refresh devices"), this);
		enableDeviceAction     = new QAction(QIcon(UI_ICON_DEVICE_ENABLE), tr("Enable"), this);
		disableDeviceAction    = new QAction(QIcon(UI_ICON_DEVICE_DISABLE), tr("Disable"), this);
		deviceSettingsAction   = new QAction(QIcon(UI_ICON_SCRIPT_SETTINGS), tr("Scripting settings..."), this);
		ipConfigurationAction  = new QAction(QIcon(UI_ICON_EDIT), tr("Set IP Configuration..."), this);
		wirelessSettingsAction = new QAction(QIcon(UI_ICON_AIR_SETTINGS), tr("Wireless settings..."), this);
		
		enableDeviceAction->setEnabled(false);
		disableDeviceAction->setEnabled(false);
		deviceSettingsAction->setEnabled(false);
		ipConfigurationAction->setEnabled(false);
		wirelessSettingsAction->setEnabled(false);
		
		overView.addAction(refreshDevicesAction);
		overView.addAction(getSeparator(this));
		overView.addAction(enableDeviceAction);
		overView.addAction(disableDeviceAction);
		overView.addAction(getSeparator(this));
		overView.addAction(deviceSettingsAction);
		overView.addAction(ipConfigurationAction);
		overView.addAction(getSeparator(this));
		overView.addAction(wirelessSettingsAction);
	}
	
	void CConnectionManager::distributeActions(int mode) {
		switch (mode) {
		case 0:
			//general device actions
			ui.menuDevice->addActions(overView.actions());
			break;
		case 1:
			ui.menuDevice->addAction(refreshDevicesAction);
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
			ui.menuDevice->addAction(current->wirelessSettingsAction);
			ui.menuDevice->addSeparator();
			ui.menuDevice->addAction(current->enterEnvironmentAction);
			}
			break;
		default:
			break;
		}
	}
	
	inline void CConnectionManager::readSettings() {
		settings.beginGroup("Main");
		ui.actionShowBalloonTips->setChecked(settings.value("showBalloonTips", true).toBool());
		ui.actionShowLog->setChecked(settings.value("showLog", true).toBool());
		settings.endGroup();
		
		settings.beginGroup("ConnectionManager");
		resize(settings.value("size", QSize(646, 322)).toSize());
		move(settings.value("pos", QPoint(200, 200)).toPoint());
		settings.endGroup();
	}
	
	inline void CConnectionManager::writeSettings() {
		settings.beginGroup("Main");
		settings.setValue("showBalloonTips", ui.actionShowBalloonTips->isChecked());
		settings.setValue("showLog", ui.actionShowLog->isChecked());
		settings.endGroup();
		
		settings.beginGroup("ConnectionManager");
		settings.setValue("size", size());
		settings.setValue("pos", pos());
		settings.endGroup();
	}
	
	void CConnectionManager::uiAddedDevice(CDevice * dev) {
		CDeviceOptions * newDeviceOptions = new CDeviceOptions(dev);
		
		tabWidget.insertTab(deviceManager.devices.indexOf(dev)+1, newDeviceOptions, dev->name);
		
		deviceOptions.insert(dev, newDeviceOptions);
		trayicon.devicesMenu.addMenu(newDeviceOptions->deviceMenu);
		trayicon.devicesMenu.setEnabled(true);
		
		connect(dev, SIGNAL(stateChanged(DeviceState)), this, SLOT(uiUpdateTrayIconInfo()));
		connect(newDeviceOptions, SIGNAL(showOptions(QWidget *)), this, SLOT(uiShowOptions(QWidget *)));
		connect(newDeviceOptions, SIGNAL(showMessage(QSystemTrayIcon *, QString, QString)),
		        this,             SLOT(uiShowMessage(QSystemTrayIcon *, QString, QString)));
	}
	
	void CConnectionManager::uiRemovedDevice(CDevice * dev) {
		//disconnect(dev, SIGNAL(stateChanged(DeviceState)), this, SLOT(uiUpdateTrayIconInfo()));
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
			result << tr("no devices present");
		else
			foreach (CDevice * i, deviceManager.devices) {
				result << shortSummary(i);
			}
		
		trayicon.setToolTip(result.join("\n"));
	}
	
	void CConnectionManager::uiCurrentTabChanged(int index) {
		ui.menuDevice->clear();
		
		if (index == 0)
			distributeActions(0);
		else if ((ui.actionShowLog->isChecked()) && (index == tabWidget.count()-1))
			distributeActions(1);
		else
			distributeActions(2);
		
		ui.toolBar->setUpdatesEnabled(false);
		ui.toolBar->clear();
		ui.toolBar->addActions(ui.menuDevice->actions());
		ui.toolBar->setUpdatesEnabled(true);
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
			disconnect(wirelessSettingsAction, SIGNAL(triggered()), deviceOptions[deselectedDevice], SLOT(uiOpenWirelessSettings()));
		}
		
		if (!selectedIndexes.isEmpty()) {
			CDevice * selectedDevice = (CDevice *)(selectedIndexes[0].internalPointer());
			connect(selectedDevice, SIGNAL(stateChanged(DeviceState)), this, SLOT(uiHandleDeviceStateChanged(DeviceState)));
			connect(enableDeviceAction, SIGNAL(triggered()), selectedDevice, SLOT(enable()));
			connect(disableDeviceAction, SIGNAL(triggered()), selectedDevice, SLOT(disable()));
			connect(deviceSettingsAction, SIGNAL(triggered()), deviceOptions[selectedDevice], SLOT(uiChangeDeviceSettings()));
			connect(ipConfigurationAction, SIGNAL(triggered()), deviceOptions[selectedDevice], SLOT(uiChangeIPConfiguration()));
			connect(wirelessSettingsAction, SIGNAL(triggered()), deviceOptions[selectedDevice], SLOT(uiOpenWirelessSettings()));
			
			enableDeviceAction->setEnabled(selectedDevice->state == DS_DEACTIVATED);
			disableDeviceAction->setDisabled(selectedDevice->state == DS_DEACTIVATED);
			deviceSettingsAction->setEnabled(true);
			ipConfigurationAction->setEnabled(selectedDevice->state == DS_UNCONFIGURED);
			wirelessSettingsAction->setEnabled((selectedDevice->type == DT_AIR) && (selectedDevice->state != DS_DEACTIVATED));
		}
		else {
			enableDeviceAction->setEnabled(false);
			disableDeviceAction->setEnabled(false);
			deviceSettingsAction->setEnabled(false);
			ipConfigurationAction->setEnabled(false);
			wirelessSettingsAction->setEnabled(false);
		}
	}
	
	void CConnectionManager::uiShowMessage(QSystemTrayIcon * trayIcon, QString title, QString message) {
		if (ui.actionShowBalloonTips->isChecked()) {
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
		ipConfigurationAction->setEnabled(state == DS_UNCONFIGURED);
		//TODO: folgendes Ekelhafte durch was sinnvolleres ersetzen
		QModelIndexList selectedIndexes = overView.selectionModel()->selectedIndexes();
		CDevice * currentDevice = static_cast<CDevice *>(selectedIndexes[0].internalPointer());
		wirelessSettingsAction->setEnabled((currentDevice->type == DT_AIR) && (state != DS_DEACTIVATED));
	}
	
	void CConnectionManager::uiHandleShowLogToggle(bool state) {
		if (state) {
			tabWidget.addTab(&logEdit, tr("Log"));
		}
		else {
			tabWidget.removeTab(tabWidget.count()-1);
		}
	}
	
	void CConnectionManager::uiShowOptions(QWidget * widget) {
		show();
		activateWindow();
		tabWidget.setCurrentWidget(widget);
	}
};
