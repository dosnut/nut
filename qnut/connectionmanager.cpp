#include <QDate>
#include "connectionmanager.h"
#include "overviewmodel.h"
#include "common.h"

namespace qnut {
	using namespace libnutclient;
	using namespace libnutcommon;
	
	CConnectionManager::CConnectionManager(QWidget * parent) :
		QMainWindow(parent),
		deviceManager(this),
		logFile(this, UI_FILE_LOG),
		settings(UI_FILE_CONFIG, QSettings::IniFormat, this),
		trayicon(this)
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
			showLog(true);
		
		ui.centralwidget->layout()->addWidget(&tabWidget);
		
		connect(&deviceManager, SIGNAL(deviceAdded(libnutclient::CDevice *)), this, SLOT(addUiDevice(libnutclient::CDevice *)));
		connect(&deviceManager, SIGNAL(deviceAdded(libnutclient::CDevice *)), this, SLOT(updateTrayIconInfo()));
		connect(&deviceManager, SIGNAL(deviceRemoved(libnutclient::CDevice *)), this, SLOT(removeUiDevice(libnutclient::CDevice *)));
		connect(&deviceManager, SIGNAL(deviceRemoved(libnutclient::CDevice *)), this, SLOT(updateTrayIconInfo()));
		
		if (logFile.error() != QFile::NoError)
			logEdit.append(tr("ERROR: %1").arg(tr("Cannot create/open log file.")));
		
		connect(&logFile, SIGNAL(printed(const QString &)), &logEdit, SLOT(append(const QString &)));
		
		logFile << tr("%1 (v%2) started").arg(UI_NAME, UI_VERSION);
		logFile << QDateTime::currentDateTime().toString();
		
		createActions();
		distributeActions();
		ui.toolBar->addActions(ui.menuDevice->actions());
		
		connect(ui.actionShowLog, SIGNAL(toggled(bool)), this, SLOT(showLog(bool)));
		connect(ui.actionAboutQt, SIGNAL(triggered()), qApp , SLOT(aboutQt()));
		connect(ui.actionAboutQNUT, SIGNAL(triggered()), this , SLOT(showAbout()));
		connect(&tabWidget, SIGNAL(currentChanged(int)), this, SLOT(handleTabChanged(int)));
		connect(&trayicon, SIGNAL(messageClicked()), this, SLOT(show()));
		connect(overView.selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(handleSelectionChanged(const QItemSelection &, const QItemSelection &)));
		
		trayicon.show();
		deviceManager.init(&logFile);
	}
	
	CConnectionManager::~CConnectionManager() {
		writeSettings();
	}
	
	inline void CConnectionManager::createActions() {
		//overViewMenu Actions
		refreshDevicesAction   = new QAction(QIcon(UI_ICON_RELOAD), tr("Refresh devices"), this);
		enableDeviceAction     = new QAction(QIcon(UI_ICON_ENABLE), tr("Enable"), this);
		disableDeviceAction    = new QAction(QIcon(UI_ICON_DISABLE), tr("Disable"), this);
		deviceSettingsAction   = new QAction(QIcon(UI_ICON_SCRIPT_SETTINGS), tr("Scripting settings..."), this);
		wirelessSettingsAction = new QAction(QIcon(UI_ICON_AIR), tr("Wireless settings..."), this);
		clearLogAction         = new QAction(QIcon(UI_ICON_CLEAR), tr("Clear log"), this);
		
		enableDeviceAction->setEnabled(false);
		disableDeviceAction->setEnabled(false);
		deviceSettingsAction->setEnabled(false);
		wirelessSettingsAction->setEnabled(false);
		
		overView.addAction(refreshDevicesAction);
		overView.addAction(getSeparator(this));
		overView.addAction(enableDeviceAction);
		overView.addAction(disableDeviceAction);
		overView.addAction(getSeparator(this));
		overView.addAction(deviceSettingsAction);
		overView.addAction(wirelessSettingsAction);
		
		connect(refreshDevicesAction, SIGNAL(triggered()), &deviceManager, SLOT(rebuild()));
		connect(clearLogAction, SIGNAL(triggered()), &logEdit, SLOT(clear()));
	}
	
	inline void CConnectionManager::distributeActions(int mode) {
		switch (mode) {
		case UI_ACTIONS_OVERVIEW:
			//general device actions
			ui.menuDevice->addActions(overView.actions());
			break;
		case UI_ACTIONS_LOG:
			ui.menuDevice->addAction(refreshDevicesAction);
			ui.menuDevice->addSeparator();
			ui.menuDevice->addAction(clearLogAction);
			break;
		case UI_ACTIONS_DEVICE: {
				ui.menuDevice->addAction(refreshDevicesAction);
				ui.menuDevice->addSeparator();
				
				CDeviceDetails * current = (CDeviceDetails *)(tabWidget.currentWidget());
				
				//current device actions
				ui.menuDevice->addAction(current->enableDeviceAction);
				ui.menuDevice->addAction(current->disableDeviceAction);
				ui.menuDevice->addSeparator();
				ui.menuDevice->addAction(current->deviceSettingsAction);
				ui.menuDevice->addAction(current->wirelessSettingsAction);
				ui.menuDevice->addSeparator();
				ui.menuDevice->addAction(current->enterEnvironmentAction);
				ui.menuDevice->addAction(current->ipConfigurationAction);
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
	
	void CConnectionManager::addUiDevice(CDevice * device) {
		CDeviceDetails * newDeviceOptions = new CDeviceDetails(device);
		
		tabWidget.insertTab(deviceManager.devices.indexOf(device)+1, newDeviceOptions, device->name);
		
		deviceOptions.insert(device, newDeviceOptions);
		trayicon.devicesMenu.addMenu(newDeviceOptions->deviceMenu);
		trayicon.devicesMenu.setEnabled(true);
		
		connect(device, SIGNAL(stateChanged(libnutcommon::DeviceState)), this, SLOT(updateTrayIconInfo()));
		connect(newDeviceOptions, SIGNAL(showOptionsRequested(QWidget *)), this, SLOT(showDeviceOptions(QWidget *)));
		connect(newDeviceOptions, SIGNAL(showMessageRequested(QString, QString, QSystemTrayIcon *)),
		        this,             SLOT(showMessage(QString, QString, QSystemTrayIcon *)));
	}
	
	void CConnectionManager::removeUiDevice(CDevice * device) {
		overView.clearSelection();
		CDeviceDetails * target = deviceOptions[device];
		deviceOptions.remove(device);
		
		tabWidget.removeTab(tabWidget.indexOf(target));
		
		trayicon.devicesMenu.removeAction(target->deviceMenu->menuAction());
		delete target;
		
		trayicon.devicesMenu.setDisabled(deviceManager.devices.isEmpty());
		enableDeviceAction->setDisabled(deviceManager.devices.isEmpty());
		disableDeviceAction->setDisabled(deviceManager.devices.isEmpty());
		
	}
	
	void CConnectionManager::updateTrayIconInfo() {
		QStringList result;
		
		if (deviceManager.devices.isEmpty())
			result << tr("no devices present");
		else
			foreach (CDevice * i, deviceManager.devices) {
				result << shortSummary(i);
			}
		
		trayicon.setToolTip(result.join("\n"));
	}
	
	void CConnectionManager::handleTabChanged(int index) {
		ui.menuDevice->clear();
		
		if (index == 0)
			distributeActions(UI_ACTIONS_OVERVIEW);
		else if ((ui.actionShowLog->isChecked()) && (index == tabWidget.count()-1))
			distributeActions(UI_ACTIONS_LOG);
		else
			distributeActions(UI_ACTIONS_DEVICE);
		
		ui.toolBar->setUpdatesEnabled(false);
		ui.toolBar->clear();
		ui.toolBar->addActions(ui.menuDevice->actions());
		ui.toolBar->setUpdatesEnabled(true);
	}
	
	void CConnectionManager::handleSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected) {
		QModelIndexList selectedIndexes = selected.indexes();
		QModelIndexList deselectedIndexes = deselected.indexes();
		
		if (!deselectedIndexes.isEmpty()) {
			CDevice * deselectedDevice = static_cast<CDevice *>(deselectedIndexes[0].internalPointer());
			disconnect(deselectedDevice, SIGNAL(stateChanged(libnutcommon::DeviceState)), this, SLOT(handleDeviceStateChange(libnutcommon::DeviceState)));
			disconnect(enableDeviceAction, SIGNAL(triggered()), deselectedDevice, SLOT(enable()));
			disconnect(disableDeviceAction, SIGNAL(triggered()), deselectedDevice, SLOT(disable()));
			disconnect(deviceSettingsAction, SIGNAL(triggered()), deviceOptions[deselectedDevice], SLOT(openDeviceSettings()));
			disconnect(wirelessSettingsAction, SIGNAL(triggered()), deviceOptions[deselectedDevice], SLOT(openWirelessSettings()));
		}
		
		if (!selectedIndexes.isEmpty()) {
			CDevice * selectedDevice = static_cast<CDevice *>(selectedIndexes[0].internalPointer());
			connect(selectedDevice, SIGNAL(stateChanged(libnutcommon::DeviceState)), this, SLOT(handleDeviceStateChange(libnutcommon::DeviceState)));
			connect(enableDeviceAction, SIGNAL(triggered()), selectedDevice, SLOT(enable()));
			connect(disableDeviceAction, SIGNAL(triggered()), selectedDevice, SLOT(disable()));
			connect(deviceSettingsAction, SIGNAL(triggered()), deviceOptions[selectedDevice], SLOT(openDeviceSettings()));
			connect(wirelessSettingsAction, SIGNAL(triggered()), deviceOptions[selectedDevice], SLOT(openWirelessSettings()));
			
			enableDeviceAction->setEnabled(selectedDevice->state == DS_DEACTIVATED);
			disableDeviceAction->setDisabled(selectedDevice->state == DS_DEACTIVATED);
			deviceSettingsAction->setEnabled(true);
			wirelessSettingsAction->setEnabled(selectedDevice->type == DT_AIR);
		}
		else {
			enableDeviceAction->setEnabled(false);
			disableDeviceAction->setEnabled(false);
			deviceSettingsAction->setEnabled(false);
			wirelessSettingsAction->setEnabled(false);
		}
	}
	
	void CConnectionManager::showMessage(QString title, QString message, QSystemTrayIcon * trayIcon) {
		if (ui.actionShowBalloonTips->isChecked()) {
			if (trayIcon)
				trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 4000);
			else
				trayicon.showMessage(title, message, QSystemTrayIcon::Information, 4000);
		}
	}
	
	void CConnectionManager::showAbout() {
		QMessageBox::about(this, tr("About QNUT"), UI_NAME + "\nv" + QString(UI_VERSION));
	}
	
	void CConnectionManager::handleDeviceStateChange(DeviceState state) {
		enableDeviceAction->setEnabled(state == DS_DEACTIVATED);
		disableDeviceAction->setDisabled(state == DS_DEACTIVATED);
	}
	
	void CConnectionManager::showLog(bool doShow) {
		if (doShow)
			tabWidget.addTab(&logEdit, tr("Log"));
		else
			tabWidget.removeTab(tabWidget.count()-1);
	}
	
	void CConnectionManager::showDeviceOptions(QWidget * widget) {
		show();
		activateWindow();
		tabWidget.setCurrentWidget(widget);
	}
};
