#include "connectionmanager.h"
#include "constants.h"
#include <iostream>

namespace qnut {
    //CConnectionManager
    CConnectionManager::CConnectionManager(QWidget * parent) :
        QMainWindow(parent),
        deviceManager(this),
        trayicon(this),
        overViewListModel(&(deviceManager.devices))
    {
        deviceOptions.reserve(16);
        
        ui.setupUi(this);
        
        ui.overViewList->setContextMenuPolicy(Qt::CustomContextMenu);
        ui.overViewList->setModel(&overViewListModel);
        
/*        enableDeviceAction = new QAction(QIcon(UI_ICON_ENABLE_DEVICE), tr("Enable Device"), ui.toolBar);
        enableDeviceAction->setCheckable(false);
        enableDeviceAction->setEnabled(false);
        
        disableDeviceAction = new QAction(QIcon(UI_ICON_DISABLE_DEVICE), tr("Disable Device"), ui.toolBar);
        disableDeviceAction->setCheckable(false);
        disableDeviceAction->setEnabled(false);
        
        toggleEnvironmentAction = new QAction(QIcon(UI_ICON_TOGGLE_ENVIRONMENT), tr("(De-)Activate Environment"), ui.toolBar);
        toggleEnvironmentAction->setCheckable(true);
        toggleEnvironmentAction->setEnabled(false);
        
        toggleInterfaceAction = new QAction(QIcon(UI_ICON_TOGGLE_INTERFACE), tr("(De-)Activate Interface"), ui.toolBar);
        toggleInterfaceAction->setCheckable(true);
        toggleInterfaceAction->setEnabled(false);
        
        showDeviceOptionsAction = new QAction(QIcon(UI_ICON_TOGGLE_ENVIRONMENT), tr("Environments..."), &overViewMenu);
        showDeviceOptionsAction->setCheckable(false);
        showDeviceOptionsAction->setEnabled(false);
        
        ui.toolBar->addAction(enableDeviceAction);
        ui.toolBar->addAction(disableDeviceAction);
        ui.toolBar->addSeparator();
        ui.toolBar->addAction(toggleEnvironmentAction);
        ui.toolBar->addAction(toggleInterfaceAction);
        
        overViewMenu.addAction(enableDeviceAction);
        overViewMenu.addAction(disableDeviceAction);
        overViewMenu.addAction(showDeviceOptionsAction);
        
        connect(showDeviceOptionsAction, SIGNAL(triggered()),
                this                   , SLOT(uiShowDeviceOptions()));*/
        
        connect(ui.overViewList, SIGNAL(customContextMenuRequested (const QPoint)),
                this           , SLOT(uiShowOverViewPopup(const QPoint)));
/*        connect(ui.overViewList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                this                             , SLOT(uiCurrentDeviceChanged()));*/
/*        connect(ui.tabWidget, SIGNAL(currentChanged(int)),
                this        , SLOT(uiCurrentTabChanged(int)));*/
        connect(&deviceManager, SIGNAL(deviceAdded(CDevice *)),
                this          , SLOT(uiAddedDevice(CDevice *)));
        connect(&deviceManager, SIGNAL(deviceRemoved(CDevice *)),
                this          , SLOT(uiRemovedDevice(CDevice *)));
        
        trayicon.show();
    }
    
    void CConnectionManager::uiAddedDevice(CDevice * dev) {
        DeviceOptions newDeviceOptions;
        
        newDeviceOptions.environmentsTree = new QTreeView();
        newDeviceOptions.environmentsTree->setModel(new CDeviceOptionsModel(dev));
        
        newDeviceOptions.contextMenu   = new QMenu(dev->properties.name, NULL);
        
        newDeviceOptions.enableAction  = newDeviceOptions.contextMenu->addAction(
            QIcon(UI_ICON_ENABLE_DEVICE) , tr("Enable device") , dev, SLOT(enable()));
        newDeviceOptions.disableAction = newDeviceOptions.contextMenu->addAction(
            QIcon(UI_ICON_DISABLE_DEVICE), tr("Disable device"), dev, SLOT(disable()));
        
        connect(dev, SIGNAL(stateChanged(bool)), ui.overViewList, SLOT(repaint()));
        connect(dev, SIGNAL(stateChanged(bool)), newDeviceOptions.enableAction , SLOT(setDisabled(bool)));
        connect(dev, SIGNAL(stateChanged(bool)), newDeviceOptions.disableAction, SLOT(setEnabled(bool)));
        
        deviceOptions.insert(dev->properties.name, newDeviceOptions);
        ui.tabWidget->addTab(newDeviceOptions.environmentsTree, QIcon(UI_ICON_DEVICE), dev->properties.name);
        
        trayicon.devicesMenu.addMenu(newDeviceOptions.contextMenu);
        ui.overViewList->repaint();
        newDeviceOptions.enableAction->setDisabled(dev->properties.enabled);
        newDeviceOptions.disableAction->setEnabled(dev->properties.enabled);
    }
    
    void CConnectionManager::uiRemovedDevice(CDevice * dev) {
        disconnect(dev);
        
        DeviceOptions targetDeviceOptions = deviceOptions[dev->properties.name];
        CDeviceOptionsModel * targetTreeModel = (CDeviceOptionsModel *)targetDeviceOptions.environmentsTree->model();
        
        targetDeviceOptions.environmentsTree->setModel(NULL);
        ui.tabWidget->removeTab(ui.tabWidget->indexOf(targetDeviceOptions.environmentsTree));
        trayicon.devicesMenu.removeAction(targetDeviceOptions.contextMenu->menuAction());
        targetDeviceOptions.contextMenu->clear();
        
        delete targetTreeModel;
        delete targetDeviceOptions.environmentsTree;
        delete targetDeviceOptions.contextMenu;
        
        deviceOptions.remove(dev->properties.name);
        ui.overViewList->repaint();
    }
    
//     void CConnectionManager::uiCurrentTabChanged(int index) {
//         if (index == 0) {
//             uiCurrentDeviceChanged();
//         }
//         //todo: toolbar
//     }
//     
//     void CConnectionManager::uiCurrentDeviceChanged() {
//         
//     }

    void CConnectionManager::uiShowOverViewPopup(const QPoint & pos) {
        QModelIndex selected = (ui.overViewList->selectionModel()->selectedIndexes())[0];
        deviceOptions[((CDevice *)(selected.internalPointer()))->properties.name].contextMenu->exec(ui.overViewList->mapToGlobal(pos));
    }

/*    void CConnectionManager::uiShowEnvironmentsTree() {
        QModelIndex currentIndex = ui.overViewList->selectionModel()->selectedIndexes()[0];
        CDevice * currentDevice = deviceManager.devices[currentIndex.row()];
        ui.tabWidget->setCurrentWidget(deviceOptionsTabs[currentDevice->name]);
    }*/
};
