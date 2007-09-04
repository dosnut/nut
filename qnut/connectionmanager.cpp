#include <QDate>
#include "connectionmanager.h"
#include "common.h"
#include <iostream>

namespace qnut {
    CConnectionManager::CConnectionManager(QWidget * parent) :
        QMainWindow(parent),
        deviceManager(this),
        logFile(this, UI_FILE_LOG),
        trayicon(this)
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
        connect(&deviceManager, SIGNAL(deviceRemoved(CDevice *)), this, SLOT(uiRemovedDevice(CDevice *)));
        
        if (logFile.error() != QFile::NoError)
            logEdit.append(tr("ERROR:") + " " + tr("Cannot create/open log file."));
        
        logFile << UI_NAME + " (v" + QString(UI_VERSION) + ") " + tr("started");
        logFile << QDateTime::currentDateTime().toString();
        
        createActions();
        
        try {
            deviceManager.init(&logFile);
        }
        catch (Exception & e) {
            logFile << tr("ERROR:") + " " + QString(e.what());
            refreshDevicesAction->setEnabled(false);
        }
        
        
        distributeActions();
        
        ui.toolBar->addActions(overView.actions());
        ui.menuDevice->addActions(overView.actions());
        
        connect(refreshDevicesAction, SIGNAL(triggered()), &deviceManager, SLOT(refreshAll()));
        connect(refreshDevicesAction, SIGNAL(triggered()), &overView, SLOT(reset()));
        connect(ui.actionAboutQt, SIGNAL(triggered()), qApp , SLOT(aboutQt()));
        connect(ui.actionAboutQNUT, SIGNAL(triggered()), this , SLOT(uiShowAbout()));
        connect(overView.selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                this                     , SLOT(uiSelectedDeviceChanged(const QItemSelection &, const QItemSelection &)));
        connect(&tabWidget, SIGNAL(currentChanged(int)), this, SLOT(uiCurrentTabChanged(int)));
        connect(&trayicon, SIGNAL(messageClicked()), this, SLOT(show()));
        
        trayicon.show();
    }
    
    void CConnectionManager::createActions() {
        //overViewMenu Actions
        refreshDevicesAction = new QAction(QIcon(UI_ICON_REFRESH), tr("Refresh devices"), &overView);
        enableDeviceAction   = new QAction(QIcon(UI_ICON_DEVICE_ENABLE), tr("Enable device"), &overView);
        disableDeviceAction  = new QAction(QIcon(UI_ICON_DEVICE_DISABLE), tr("Disable device"), &overView);
        
        enableDeviceAction->setEnabled(false);
        disableDeviceAction->setEnabled(false);
        
        overView.addAction(refreshDevicesAction);
        overView.addAction(getSeparator(&overView));
        overView.addAction(enableDeviceAction);
        overView.addAction(disableDeviceAction);
    }
    
    void CConnectionManager::distributeActions(int mode) {
        switch (mode) {
        case 0:
            ui.menuEnvironment->setEnabled(false);
            ui.menuInterface->setEnabled(false);
            
            //general device actions
            ui.toolBar->addActions(overView.actions());
            ui.menuDevice->addActions(overView.actions());
            break;
        case 1:
            ui.toolBar->addAction(refreshDevicesAction);
            ui.menuDevice->addAction(refreshDevicesAction);
            break;
        case 2:
            ui.menuEnvironment->setEnabled(true);
            ui.menuInterface->setEnabled(true);
            
            CDeviceOptions * current = (CDeviceOptions *)(tabWidget.currentWidget());
            
            //current device actions
            ui.toolBar->addAction(current->enableDeviceAction);
            ui.toolBar->addAction(current->disableDeviceAction);
            ui.menuDevice->addAction(current->enableDeviceAction);
            ui.menuDevice->addAction(current->disableDeviceAction);
            ui.toolBar->addSeparator();
            //environment actions
            ui.toolBar->addAction(current->enterEnvironmentAction);
            ui.menuEnvironment->addAction(current->enterEnvironmentAction);
            ui.toolBar->addSeparator();
            //interface actions
            ui.toolBar->addAction(current->activateInterfaceAction);
            ui.toolBar->addAction(current->deactivateInterfaceAction);
            ui.menuInterface->addAction(current->activateInterfaceAction);
            ui.menuInterface->addAction(current->deactivateInterfaceAction);
            break;
        }
    }
    
    void CConnectionManager::uiAddedDevice(CDevice * dev) {
        CDeviceOptions * newDeviceOptions = new CDeviceOptions(dev, &tabWidget);
        
        tabWidget.insertTab(tabWidget.count()-1, newDeviceOptions, dev->name);
        
        newDeviceOptions->updateDeviceIcons();
        
        deviceOptions.insert(dev, newDeviceOptions);
        trayicon.devicesMenu.addMenu(newDeviceOptions->deviceMenu);
        trayicon.devicesMenu.setEnabled(true);
        
        connect(dev, SIGNAL(stateChanged(DeviceState)), &overView, SLOT(reset()));
        connect(newDeviceOptions->showAction, SIGNAL(triggered()), this, SLOT(show()));
        connect(newDeviceOptions, SIGNAL(showMessage(QString, QString)), this, SLOT(uiShowMessage(QString, QString)));
        overView.reset();
    }
    
    void CConnectionManager::uiRemovedDevice(CDevice * dev) {
        overView.reset();
        disconnect(dev, SIGNAL(stateChanged(DeviceState)), &overView, SLOT(reset())); //nicht nötig?
        CDeviceOptions * target = deviceOptions[dev];
        
        tabWidget.removeTab(tabWidget.indexOf(target));
        
        trayicon.devicesMenu.removeAction(target->deviceMenu->menuAction());
        trayicon.devicesMenu.setDisabled(deviceManager.devices.isEmpty());
        deviceOptions.remove(dev);
        delete target;
    }
    
    void CConnectionManager::uiCurrentTabChanged(int index) {
        ui.menuDevice->clear();
        ui.menuEnvironment->clear();
        ui.menuInterface->clear();
        ui.toolBar->setUpdatesEnabled(false);
        ui.toolBar->clear();
        if (index == 0) {
            distributeActions(0);
        }
        else if (index == tabWidget.count()-1) {
            distributeActions(1);
        }
        else {
            distributeActions(2);
        }
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
        }
        
        if (!selectedIndexes.isEmpty()) {
            CDevice * selectedDevice = (CDevice *)(selectedIndexes[0].internalPointer());
            connect(selectedDevice, SIGNAL(stateChanged(DeviceState)), this, SLOT(uiHandleDeviceStateChanged(DeviceState)));
            connect(enableDeviceAction, SIGNAL(triggered()), selectedDevice, SLOT(enable()));
            connect(disableDeviceAction, SIGNAL(triggered()), selectedDevice, SLOT(disable()));
            
            enableDeviceAction->setDisabled(selectedDevice->state == DS_UP);
            disableDeviceAction->setDisabled(selectedDevice->state == DS_DEACTIVATED);
        }
        else {
            enableDeviceAction->setEnabled(false);
            disableDeviceAction->setEnabled(false);
        }
    }
    
/*    void CConnectionManager::uiShowOverViewPopup(const QPoint & pos) {
        overViewMenu.exec(overView.mapToGlobal(pos));
    }*/
    
    void CConnectionManager::uiShowMessage(QString title, QString message) {
        trayicon.showMessage(title, message);
    }
    
    void CConnectionManager::uiShowAbout() {
        QMessageBox::about(this, tr("About QNUT"), UI_NAME + "\nv" + QString(UI_VERSION));
    }
    
    void CConnectionManager::uiHandleDeviceStateChanged(DeviceState state) {
        enableDeviceAction->setDisabled(state == DS_UP);
        disableDeviceAction->setDisabled(state == DS_DEACTIVATED);
    }
};
