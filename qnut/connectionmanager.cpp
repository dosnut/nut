#include "connectionmanager.h"
#include "constants.h"
#include <QDate>

namespace qnut {
    CConnectionManager::CConnectionManager(QWidget * parent) :
        QMainWindow(parent),
        deviceManager(this),
        logFile(this, UI_FILE_LOG),
        trayicon(this),
        overViewListModel(&(deviceManager.devices))
    {
        setWindowIcon(trayicon.icon());
        deviceOptions.reserve(10);
        ui.setupUi(this);
        
        connect(&logFile, SIGNAL(printed(const QString &)), ui.logEdit, SLOT(append(const QString &)));
        
        if (logFile.error() != QFile::NoError)
            ui.logEdit->append(tr("ERROR:") + " " + tr("Cannot create/open log file."));
        
        logFile << UI_NAME + " (v" + QString(UI_VERSION) + ") " + tr("started");
        logFile << QDateTime::currentDateTime().toString();
        
        try {
            deviceManager.init(&logFile);
        }
        catch (Exception & e) {
            logFile << tr("ERROR:") + " " + QString(e.what());
        }
        
        ui.overViewList->setContextMenuPolicy(Qt::CustomContextMenu);
        ui.overViewList->setModel(&overViewListModel);
        
        createActions();
        distributeActions();
        
        ui.toolBar->addActions(overViewMenu.actions());
        ui.menuDevice->addActions(overViewMenu.actions());
        
        connect(refreshDevicesAction, SIGNAL(triggered()), ui.overViewList, SLOT(repaint()));
        connect(ui.actionAboutQt, SIGNAL(triggered()), qApp , SLOT(aboutQt()));
        connect(ui.actionAboutQNUT, SIGNAL(triggered()), this , SLOT(uiShowAbout()));
        connect(ui.overViewList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                this                             , SLOT(uiSelectedDeviceChanged(const QItemSelection &, const QItemSelection &)));
        connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(uiCurrentTabChanged(int)));
        connect(ui.overViewList, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(uiShowOverViewPopup(const QPoint)));
        connect(&trayicon, SIGNAL(messageClicked()), this, SLOT(show()));
        connect(&deviceManager, SIGNAL(deviceAdded(CDevice *)), this, SLOT(uiAddedDevice(CDevice *)));
        connect(&deviceManager, SIGNAL(deviceRemoved(CDevice *)), this, SLOT(uiRemovedDevice(CDevice *)));
        
        trayicon.show();
    }
    
    void CConnectionManager::createActions() {
        //overViewMenu Actions
        refreshDevicesAction    = overViewMenu.addAction(QIcon(UI_ICON_REFRESH), tr("Refresh devices"), &deviceManager, SLOT(refreshAll()));
        overViewMenu.addSeparator();
        enableDeviceAction      = overViewMenu.addAction(QIcon(UI_ICON_ENABLE_DEVICE), tr("Enable device"));
        disableDeviceAction     = overViewMenu.addAction(QIcon(UI_ICON_DISABLE_DEVICE), tr("Disable device"));
/*        overViewMenu.addSeparator();
        addEnvironmentAction    = overViewMenu.addAction(QIcon(UI_ICON_ADD_ENVIRONMENT), tr("Add environment"));
        removeEnvironmentAction = overViewMenu.addAction(QIcon(UI_ICON_REMOVE_ENVIRONMENT), tr("Remove environment"));*/
        
        enableDeviceAction->setEnabled(false);
        disableDeviceAction->setEnabled(false);
/*        addEnvironmentAction->setEnabled(false);
        removeEnvironmentAction->setEnabled(false);*/
    }
    
    void CConnectionManager::distributeActions(int mode) {
        switch (mode) {
            case 0:
                ui.menuEnvironment->setEnabled(false);
                ui.menuInterface->setEnabled(false);
                
                //general device actions
                ui.toolBar->addActions(overViewMenu.actions());
                ui.menuDevice->addActions(overViewMenu.actions());
                break;
            case 1:
                ui.toolBar->addAction(refreshDevicesAction);
                ui.menuDevice->addAction(refreshDevicesAction);
                break;
            case 2:
                ui.menuEnvironment->setEnabled(true);
                ui.menuInterface->setEnabled(true);
                
                CDeviceOptions * current = (CDeviceOptions *)(ui.tabWidget->currentWidget());
                
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
        CDeviceOptions * newDeviceOptions = new CDeviceOptions(dev, ui.tabWidget);
        
        ui.tabWidget->insertTab(ui.tabWidget->count()-1, newDeviceOptions, dev->name);
        newDeviceOptions->uiUpdateDeviceIcons();
        
        deviceOptions.insert(dev, newDeviceOptions);
        trayicon.devicesMenu.addMenu(newDeviceOptions->deviceMenu);
        trayicon.devicesMenu.setEnabled(true);
        
        ui.overViewList->repaint();
        connect(dev, SIGNAL(stateChanged(bool)), ui.overViewList, SLOT(repaint()));
        connect(newDeviceOptions->showAction, SIGNAL(triggered()), this, SLOT(show()));
        connect(newDeviceOptions, SIGNAL(showMessage(QString, QString)), this, SLOT(uiShowMessage(QString, QString)));
    }
    
    void CConnectionManager::uiRemovedDevice(CDevice * dev) {
        CDeviceOptions * target = deviceOptions[dev];
        
        //disconnect(dev, SIGNAL(stateChanged(bool)), ui.overViewList, SLOT(repaint())); nicht nötig?
        
        ui.tabWidget->removeTab(ui.tabWidget->indexOf(target));
        ui.overViewList->repaint();
        
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
        else if (index == ui.tabWidget->count()-1) {
            distributeActions(1);
        }
        else {
            distributeActions(2);
        }
        ui.toolBar->setUpdatesEnabled(true);
    }
    
    void CConnectionManager::uiSelectedDeviceChanged(const QItemSelection & selected, const QItemSelection & deselected) {
        QModelIndexList selectedIndexes = selected.indexes();
        QModelIndexList deselectedIndexes = selected.indexes();
        
        if (!deselectedIndexes.isEmpty()) {
            CDevice * deselectedDevice = (CDevice *)(deselectedIndexes[0].internalPointer());
            disconnect(deselectedDevice, SIGNAL(stateChanged(bool)), enableDeviceAction, SLOT(setDisabled(bool)));
            disconnect(deselectedDevice, SIGNAL(stateChanged(bool)), disableDeviceAction, SLOT(setEnabled(bool)));
            disconnect(enableDeviceAction, SIGNAL(triggered()), deselectedDevice, SLOT(enable()));
            disconnect(disableDeviceAction, SIGNAL(triggered()), deselectedDevice, SLOT(disable()));
        }
        
        if (!selectedIndexes.isEmpty()) {
            CDevice * selectedDevice = (CDevice *)(selectedIndexes[0].internalPointer());
            connect(selectedDevice, SIGNAL(stateChanged(bool)), enableDeviceAction, SLOT(setDisabled(bool)));
            connect(selectedDevice, SIGNAL(stateChanged(bool)), disableDeviceAction, SLOT(setEnabled(bool)));
            connect(enableDeviceAction, SIGNAL(triggered()), selectedDevice, SLOT(enable()));
            connect(disableDeviceAction, SIGNAL(triggered()), selectedDevice, SLOT(disable()));
            
            enableDeviceAction->setDisabled(selectedDevice->enabled);
            disableDeviceAction->setEnabled(selectedDevice->enabled);
        }
        else {
            enableDeviceAction->setEnabled(false);
            disableDeviceAction->setEnabled(false);
        }
    }
    
    void CConnectionManager::uiShowOverViewPopup(const QPoint & pos) {
        overViewMenu.exec(ui.overViewList->mapToGlobal(pos));
    }
    
    void CConnectionManager::uiShowMessage(QString title, QString message) {
        trayicon.showMessage(title, message);
    }
    
    void CConnectionManager::uiShowAbout() {
        QMessageBox::about(this, tr("About QNUT"), UI_NAME + "\nv" + QString(UI_VERSION));
    }
};
