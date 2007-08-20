#include "connectionmanager.h"
#include "constants.h"
#include <iostream>

namespace qnut {
    CConnectionManager::CConnectionManager(QWidget * parent) :
        QMainWindow(parent),
        deviceManager(this),
        trayicon(this),
        overViewListModel(&(deviceManager.devices))
    {
        deviceOptions.reserve(10);
        ui.setupUi(this);
        
        ui.overViewList->setContextMenuPolicy(Qt::CustomContextMenu);
        ui.overViewList->setModel(&overViewListModel);
        
        enableDeviceAction = new QAction(QIcon(UI_ICON_ENABLE_DEVICE), tr("Enable Device"), &overViewMenu);
        enableDeviceAction->setCheckable(false);
        enableDeviceAction->setEnabled(false);
        
        disableDeviceAction = new QAction(QIcon(UI_ICON_DISABLE_DEVICE), tr("Disable Device"), &overViewMenu);
        disableDeviceAction->setCheckable(false);
        disableDeviceAction->setEnabled(false);
        
        overViewMenu.addAction(enableDeviceAction);
        overViewMenu.addAction(disableDeviceAction);
        
        connect(&trayicon, SIGNAL(messageClicked()),
                this     , SLOT(show()));
        connect(ui.actionAboutQt, SIGNAL(triggered()),
                qApp            , SLOT(aboutQt()));
        connect(ui.overViewList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                this                             , SLOT(uiSelectedDeviceChanged(const QItemSelection &, const QItemSelection &)));
        connect(ui.tabWidget, SIGNAL(currentChanged(int)),
                this        , SLOT(uiCurrentTabChanged(int)));
        connect(ui.overViewList, SIGNAL(customContextMenuRequested (const QPoint)),
                this           , SLOT(uiShowOverViewPopup(const QPoint)));
        connect(&deviceManager, SIGNAL(deviceAdded(CDevice *)),
                this          , SLOT(uiAddedDevice(CDevice *)));
        connect(&deviceManager, SIGNAL(deviceRemoved(CDevice *)),
                this          , SLOT(uiRemovedDevice(CDevice *)));
        
        uiCurrentTabChanged(0); //nicht nötig?
        trayicon.show();
    }
    
    void CConnectionManager::uiAddedDevice(CDevice * dev) {
        CDeviceOptions * newDeviceOptions = new CDeviceOptions(dev, ui.tabWidget);
        
        ui.tabWidget->addTab(newDeviceOptions, dev->properties.name);
        newDeviceOptions->updateDeviceIcons();
        
        deviceOptions.insert(dev, newDeviceOptions);
        trayicon.devicesMenu.addMenu(newDeviceOptions->deviceMenu);
        trayicon.devicesMenu.setEnabled(true);
        
        ui.overViewList->repaint();
        connect(dev, SIGNAL(stateChanged(bool)), ui.overViewList, SLOT(repaint()));
        connect(newDeviceOptions->showAction, SIGNAL(triggered()), this, SLOT(show()));
    }
    
    void CConnectionManager::uiRemovedDevice(CDevice * dev) {
        CDeviceOptions * target = deviceOptions[dev];
        
        disconnect(dev, SIGNAL(stateChanged(bool)), ui.overViewList, SLOT(repaint()));
        
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
            ui.menuEnvironment->setEnabled(false);
            ui.menuInterface->setEnabled(false);
            
            //general device actions
            ui.toolBar->addAction(enableDeviceAction);
            ui.toolBar->addAction(disableDeviceAction);
            ui.menuDevice->addAction(enableDeviceAction);
            ui.menuDevice->addAction(disableDeviceAction);
        }
        else {
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
            
            enableDeviceAction->setDisabled(selectedDevice->properties.enabled);
            disableDeviceAction->setEnabled(selectedDevice->properties.enabled);
        }
        else {
            enableDeviceAction->setEnabled(false);
            disableDeviceAction->setEnabled(false);
        }
    }
    
    void CConnectionManager::uiShowOverViewPopup(const QPoint & pos) {
        overViewMenu.exec(ui.overViewList->mapToGlobal(pos));
    }
    
    void CConnectionManager::uiShowUserInputMessage() {
        trayicon.showMessage(tr("User defined environment entered"), tr("A device entered an environment, that needs to be configured in order to be active.\n\n Click here to open the connection manager."));
    }
    
/*    void CConnectionManager::uiShowEnvironmentsTree() {
        QModelIndex currentIndex = ui.overViewList->selectionModel()->selectedIndexes()[0];
        CDevice * currentDevice = deviceManager.devices[currentIndex.row()];
        ui.tabWidget->setCurrentWidget(deviceOptionsTabs[currentDevice->name]);
    }*/
};
