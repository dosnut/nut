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
        
        deviceOptions.insert(dev, newDeviceOptions);
        ui.tabWidget->addTab(newDeviceOptions, QIcon(UI_ICON_DEVICE), dev->properties.name);
        trayicon.devicesMenu.addMenu(newDeviceOptions->deviceMenu);
        
        ui.overViewList->repaint();
        connect(dev, SIGNAL(stateChanged(bool)), ui.overViewList, SLOT(repaint()));
        connect(newDeviceOptions->showAction, SIGNAL(triggered()), this, SLOT(show()));
    }
    
    void CConnectionManager::uiRemovedDevice(CDevice * dev) {
        CDeviceOptions * target = deviceOptions[dev];
        
        disconnect(dev, SIGNAL(stateChanged(bool)), ui.overViewList, SLOT(repaint()));
        
        ui.tabWidget->removeTab(ui.tabWidget->indexOf(target));
        deviceOptions.remove(dev);
        
        trayicon.devicesMenu.removeAction(target->deviceMenu->menuAction());
        delete target;
        
        ui.overViewList->repaint();
    }
    
    void CConnectionManager::uiCurrentTabChanged(int index) {
        ui.toolBar->setUpdatesEnabled(false);
        ui.toolBar->clear();
        if (index == 0) {
            //general device actions
            ui.toolBar->addAction(enableDeviceAction);
            ui.toolBar->addAction(disableDeviceAction);
        }
        else {
            CDeviceOptions * current = (CDeviceOptions *)(ui.tabWidget->currentWidget());
            //current device actions
            ui.toolBar->addAction(current->enableDeviceAction);
            ui.toolBar->addAction(current->disableDeviceAction);
            //environment actions
            //ui.toolBar->addAction(activateEnvironmentAction);
            //ui.toolBar->addAction(deactivateEnvironmentAction);
            //interface actions
            //ui.toolBar->addAction(activateInterfaceAction);
            //ui.toolBar->addAction(deactivateInterfaceAction);
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

/*    void CConnectionManager::uiShowEnvironmentsTree() {
        QModelIndex currentIndex = ui.overViewList->selectionModel()->selectedIndexes()[0];
        CDevice * currentDevice = deviceManager.devices[currentIndex.row()];
        ui.tabWidget->setCurrentWidget(deviceOptionsTabs[currentDevice->name]);
    }*/
};
