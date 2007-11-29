#include <QDate>
#include "connectionmanager.h"
#include "overviewmodel.h"
#include "common.h"

namespace qnut {
	using namespace libnutclient;
	using namespace libnutcommon;
	
	CConnectionManager::CConnectionManager(QWidget * parent) :
		QMainWindow(parent),
		mDeviceManager(this),
		mLogFile(this, UI_FILE_LOG),
		mStettings(UI_FILE_CONFIG, QSettings::IniFormat, this),
		mTrayIcon(this)
	{
		setWindowIcon(mTrayIcon.icon());
		mDeviceDetails.reserve(10);
		
		ui.setupUi(this);
		readSettings();
		
		mLogEdit.setReadOnly(true);
		mLogEdit.setAcceptRichText(false);
		
		//mOverView.setSortingEnabled(true);
		mOverView.setModel(new COverViewModel(&(mDeviceManager)));
		mOverView.setContextMenuPolicy(Qt::ActionsContextMenu);
		mOverView.setRootIsDecorated(false);
		mOverView.setItemsExpandable(false);
		mOverView.setAllColumnsShowFocus(true);
		mOverView.setIconSize(QSize(32, 32));
		
		mTabWidget.addTab(&mOverView, tr("Overview"));
		
		if (ui.actionShowLog->isChecked())
			showLog(true);
		
		ui.centralwidget->layout()->addWidget(&mTabWidget);
		
		connect(&mDeviceManager, SIGNAL(deviceAdded(libnutclient::CDevice *)), this, SLOT(addUiDevice(libnutclient::CDevice *)));
		connect(&mDeviceManager, SIGNAL(deviceAdded(libnutclient::CDevice *)), this, SLOT(updateTrayIconInfo()));
		connect(&mDeviceManager, SIGNAL(deviceRemoved(libnutclient::CDevice *)), this, SLOT(removeUiDevice(libnutclient::CDevice *)));
		connect(&mDeviceManager, SIGNAL(deviceRemoved(libnutclient::CDevice *)), this, SLOT(updateTrayIconInfo()));
		
		if (mLogFile.error() != QFile::NoError)
			mLogEdit.append(tr("ERROR: %1").arg(tr("Cannot create/open log file.")));
		
		connect(&mLogFile, SIGNAL(printed(const QString &)), &mLogEdit, SLOT(append(const QString &)));
		
		mLogFile << tr("%1 (v%2) started").arg(UI_NAME, UI_VERSION);
		mLogFile << QDateTime::currentDateTime().toString();
		
		createActions();
		distributeActions();
		ui.toolBar->addActions(ui.menuDevice->actions());
		
		connect(ui.actionShowLog, SIGNAL(toggled(bool)), this, SLOT(showLog(bool)));
		connect(ui.actionAboutQt, SIGNAL(triggered()), qApp , SLOT(aboutQt()));
		connect(ui.actionAboutQNUT, SIGNAL(triggered()), this , SLOT(showAbout()));
		connect(&mTabWidget, SIGNAL(currentChanged(int)), this, SLOT(handleTabChanged(int)));
		connect(&mTrayIcon, SIGNAL(messageClicked()), this, SLOT(show()));
		connect(mOverView.selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(handleSelectionChanged(const QItemSelection &, const QItemSelection &)));
		
		mTrayIcon.show();
		mDeviceManager.init(&mLogFile);
	}
	
	CConnectionManager::~CConnectionManager() {
		writeSettings();
	}
	
	inline void CConnectionManager::createActions() {
		//overViewMenu Actions
		mRefreshDevicesAction   = new QAction(QIcon(UI_ICON_RELOAD), tr("&Refresh devices"), this);
		mEnableDeviceAction     = new QAction(QIcon(UI_ICON_ENABLE), tr("&Enable"), this);
		mDisableDeviceAction    = new QAction(QIcon(UI_ICON_DISABLE), tr("&Disable"), this);
		mDeviceSettingsAction   = new QAction(QIcon(UI_ICON_SCRIPT_SETTINGS), tr("&Scripting settings..."), this);
		mWirelessSettingsAction = new QAction(QIcon(UI_ICON_AIR), tr("&Wireless settings..."), this);
		mClearLogAction         = new QAction(QIcon(UI_ICON_CLEAR), tr("&Clear log"), this);
		
		mEnableDeviceAction->setEnabled(false);
		mDisableDeviceAction->setEnabled(false);
		mDeviceSettingsAction->setEnabled(false);
		mWirelessSettingsAction->setEnabled(false);
		
		mOverView.addAction(mRefreshDevicesAction);
		mOverView.addAction(getSeparator(this));
		mOverView.addAction(mEnableDeviceAction);
		mOverView.addAction(mDisableDeviceAction);
		mOverView.addAction(getSeparator(this));
		mOverView.addAction(mDeviceSettingsAction);
		mOverView.addAction(mWirelessSettingsAction);
		
		connect(mRefreshDevicesAction, SIGNAL(triggered()), &mDeviceManager, SLOT(rebuild()));
		connect(mClearLogAction, SIGNAL(triggered()), &mLogEdit, SLOT(clear()));
	}
	
	inline void CConnectionManager::distributeActions(int mode) {
		switch (mode) {
		case UI_ACTIONS_OVERVIEW:
			//general device actions
			ui.menuDevice->addActions(mOverView.actions());
			break;
		case UI_ACTIONS_LOG:
			ui.menuDevice->addAction(mRefreshDevicesAction);
			ui.menuDevice->addSeparator();
			ui.menuDevice->addAction(mClearLogAction);
			break;
		case UI_ACTIONS_DEVICE: {
				ui.menuDevice->addAction(mRefreshDevicesAction);
				ui.menuDevice->addSeparator();
				
				CDeviceDetails * current = (CDeviceDetails *)(mTabWidget.currentWidget());
				
				//current device actions
				ui.menuDevice->addAction(current->mEnableDeviceAction);
				ui.menuDevice->addAction(current->mDisableDeviceAction);
				ui.menuDevice->addSeparator();
				ui.menuDevice->addAction(current->mDeviceSettingsAction);
				ui.menuDevice->addAction(current->mWirelessSettingsAction);
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
		mStettings.beginGroup("Main");
		ui.actionShowBalloonTips->setChecked(mStettings.value("showBalloonTips", true).toBool());
		ui.actionShowLog->setChecked(mStettings.value("showLog", true).toBool());
		mStettings.endGroup();
		
		mStettings.beginGroup("ConnectionManager");
		resize(mStettings.value("size", QSize(646, 322)).toSize());
		move(mStettings.value("pos", QPoint(200, 200)).toPoint());
		mStettings.endGroup();
	}
	
	inline void CConnectionManager::writeSettings() {
		mStettings.beginGroup("Main");
		mStettings.setValue("showBalloonTips", ui.actionShowBalloonTips->isChecked());
		mStettings.setValue("showLog", ui.actionShowLog->isChecked());
		mStettings.endGroup();
		
		mStettings.beginGroup("ConnectionManager");
		mStettings.setValue("size", size());
		mStettings.setValue("pos", pos());
		mStettings.endGroup();
	}
	
	void CConnectionManager::addUiDevice(CDevice * device) {
		CDeviceDetails * newDeviceOptions = new CDeviceDetails(device);
		
		mTabWidget.insertTab(mDeviceManager.devices.indexOf(device)+1, newDeviceOptions, device->name);
		
		mDeviceDetails.insert(device, newDeviceOptions);
		mTrayIcon.devicesMenu.addMenu(newDeviceOptions->deviceMenu);
		mTrayIcon.devicesMenu.setEnabled(true);
		
		connect(device, SIGNAL(stateChanged(libnutcommon::DeviceState)), this, SLOT(updateTrayIconInfo()));
		connect(newDeviceOptions, SIGNAL(showOptionsRequested(QWidget *)), this, SLOT(showDeviceOptions(QWidget *)));
		connect(newDeviceOptions, SIGNAL(showMessageRequested(QString, QString, QSystemTrayIcon *)),
		        this,             SLOT(showMessage(QString, QString, QSystemTrayIcon *)));
	}
	
	void CConnectionManager::removeUiDevice(CDevice * device) {
		mOverView.clearSelection();
		CDeviceDetails * target = mDeviceDetails[device];
		mDeviceDetails.remove(device);
		
		mTabWidget.removeTab(mTabWidget.indexOf(target));
		
		mTrayIcon.devicesMenu.removeAction(target->deviceMenu->menuAction());
		delete target;
		
		mTrayIcon.devicesMenu.setDisabled(mDeviceManager.devices.isEmpty());
		mEnableDeviceAction->setDisabled(mDeviceManager.devices.isEmpty());
		mDisableDeviceAction->setDisabled(mDeviceManager.devices.isEmpty());
		
	}
	
	void CConnectionManager::updateTrayIconInfo() {
		QStringList result;
		
		if (mDeviceManager.devices.isEmpty())
			result << tr("no devices present");
		else
			foreach (CDevice * i, mDeviceManager.devices) {
				result << shortSummary(i);
			}
		
		mTrayIcon.setToolTip(result.join("\n"));
	}
	
	void CConnectionManager::handleTabChanged(int index) {
		ui.menuDevice->clear();
		
		if (index == 0)
			distributeActions(UI_ACTIONS_OVERVIEW);
		else if ((ui.actionShowLog->isChecked()) && (index == mTabWidget.count()-1))
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
			disconnect(mEnableDeviceAction, SIGNAL(triggered()), deselectedDevice, SLOT(enable()));
			disconnect(mDisableDeviceAction, SIGNAL(triggered()), deselectedDevice, SLOT(disable()));
			disconnect(mDeviceSettingsAction, SIGNAL(triggered()), mDeviceDetails[deselectedDevice], SLOT(openDeviceSettings()));
			disconnect(mWirelessSettingsAction, SIGNAL(triggered()), mDeviceDetails[deselectedDevice], SLOT(openWirelessSettings()));
		}
		
		if (!selectedIndexes.isEmpty()) {
			CDevice * selectedDevice = static_cast<CDevice *>(selectedIndexes[0].internalPointer());
			connect(selectedDevice, SIGNAL(stateChanged(libnutcommon::DeviceState)), this, SLOT(handleDeviceStateChange(libnutcommon::DeviceState)));
			connect(mEnableDeviceAction, SIGNAL(triggered()), selectedDevice, SLOT(enable()));
			connect(mDisableDeviceAction, SIGNAL(triggered()), selectedDevice, SLOT(disable()));
			connect(mDeviceSettingsAction, SIGNAL(triggered()), mDeviceDetails[selectedDevice], SLOT(openDeviceSettings()));
			connect(mWirelessSettingsAction, SIGNAL(triggered()), mDeviceDetails[selectedDevice], SLOT(openWirelessSettings()));
			
			mEnableDeviceAction->setEnabled(selectedDevice->state == DS_DEACTIVATED);
			mDisableDeviceAction->setDisabled(selectedDevice->state == DS_DEACTIVATED);
			mDeviceSettingsAction->setEnabled(true);
			mWirelessSettingsAction->setEnabled(selectedDevice->type == DT_AIR);
		}
		else {
			mEnableDeviceAction->setEnabled(false);
			mDisableDeviceAction->setEnabled(false);
			mDeviceSettingsAction->setEnabled(false);
			mWirelessSettingsAction->setEnabled(false);
		}
	}
	
	void CConnectionManager::showMessage(QString title, QString message, QSystemTrayIcon * trayIcon) {
		if (ui.actionShowBalloonTips->isChecked()) {
			if (trayIcon)
				trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 4000);
			else
				mTrayIcon.showMessage(title, message, QSystemTrayIcon::Information, 4000);
		}
	}
	
	void CConnectionManager::showAbout() {
		QMessageBox::about(this, tr("About QNUT"), UI_NAME + "\nv" + QString(UI_VERSION));
	}
	
	void CConnectionManager::handleDeviceStateChange(DeviceState state) {
		mEnableDeviceAction->setEnabled(state == DS_DEACTIVATED);
		mDisableDeviceAction->setDisabled(state == DS_DEACTIVATED);
	}
	
	void CConnectionManager::showLog(bool doShow) {
		if (doShow)
			mTabWidget.addTab(&mLogEdit, tr("Log"));
		else
			mTabWidget.removeTab(mTabWidget.count()-1);
	}
	
	void CConnectionManager::showDeviceOptions(QWidget * widget) {
		show();
		activateWindow();
		mTabWidget.setCurrentWidget(widget);
	}
};
