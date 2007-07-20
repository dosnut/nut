#include "connectionmanager.h"
#include "constants.h"
//#include <iostream>

namespace qnut {
    //CConnectionManager
    CConnectionManager::CConnectionManager(QWidget * parent) :
        QMainWindow(parent),
        trayicon(this),
        deviceManager(this),
        overViewListModel(&(deviceManager.devices))
    {
        deviceOptionsTabs.reserve(16);
        ui.setupUi(this);
        
        ui.overViewList->setContextMenuPolicy(Qt::CustomContextMenu);
        ui.overViewList->setModel(&overViewListModel);
        
        toggleDeviceAction = new QAction(QIcon(UI_ICON_TOGGLE_DEVICE), tr("Enable/Disable Device"), ui.toolBar);
        toggleDeviceAction->setCheckable(true);
        toggleDeviceAction->setEnabled(false);
        
        toggleEnvironmentAction = new QAction(QIcon(UI_ICON_TOGGLE_ENVIRONMENT), tr("(De-)Activate Environment"), ui.toolBar);
        toggleEnvironmentAction->setCheckable(true);
        toggleEnvironmentAction->setEnabled(false);
        
        toggleInterfaceAction = new QAction(QIcon(UI_ICON_TOGGLE_INTERFACE), tr("(De-)Activate Interface"), ui.toolBar);
        toggleInterfaceAction->setCheckable(true);
        toggleInterfaceAction->setEnabled(false);
        
        ui.toolBar->addAction(toggleDeviceAction);
        ui.toolBar->addSeparator();
        ui.toolBar->addAction(toggleEnvironmentAction);
        ui.toolBar->addAction(toggleInterfaceAction);
        
        //connect(ui.overViewList, SIGNAL(customContextMenuRequested (const QPoint)),
        //        this           , SLOT(uiShowOverviewPopup(const QPoint)));
        connect(&deviceManager, SIGNAL(deviceAdded(CDevice *)),
                this          , SLOT(uiAddedDevice(CDevice *)));
        connect(&deviceManager, SIGNAL(deviceRemoved(CDevice *)),
                this          , SLOT(uiRemovedDevice(CDevice *)));
        
        trayicon.show();
    }
    
    void CConnectionManager::uiAddedDevice(CDevice * dev) {
        QTreeView * newDeviceOptions;
        
        newDeviceOptions = new QTreeView();
        newDeviceOptions->setModel(new CDeviceOptionsModel(dev));
        deviceOptionsTabs.insert(dev->name, newDeviceOptions);
        ui.tabWidget->addTab(newDeviceOptions, QIcon(UI_ICON_DEVICE), dev->name);
    }
    
    void CConnectionManager::uiRemovedDevice(CDevice * dev) {
        QTreeView * targetDeviceOptionsTab = deviceOptionsTabs[dev->name];
        CDeviceOptionsModel * targetDeviceOptionsModel = (CDeviceOptionsModel *)targetDeviceOptionsTab->model();
        
        ui.tabWidget->removeTab(ui.tabWidget->indexOf(targetDeviceOptionsTab));
        targetDeviceOptionsTab->setModel(NULL);
        deviceOptionsTabs.remove(dev->name);
        delete targetDeviceOptionsModel;
        delete targetDeviceOptionsTab;
    }
    
/*    void CConnectionManager::uiShowOverViewPopup(const QPoint & pos) {
        QModelIndex currentIndex = ui.overViewList->currentIndex();
        
        if ((currentIndex.isValid()) && (currentIndex.column() == 0)) {
            CDevice * currentDevice = deviceManager.devices[currentIndex.row()];
            if (currentDevice->enabled) {
                overViewMenu.addAction(
                    tr("disable"),
                    currentDevice,
                    SLOT(disable())
                );
            }
            else {
                overViewMenu.addAction(
                    tr("enable"),
                    currentDevice,
                    SLOT(enable())
                );
            }
            overViewMenu.exec(ui.overViewList->mapToGlobal(pos));
            overViewMenu.clear();
        }
//        std::cout << "x: " << pos.x() << "; y: " << pos.y() << std::endl;
    }
    
    void CConnectionManager::uiShowDeviceOptionsPopup(const QPoint & pos) {
        QTreeView * currentDeviceOptions = (QTreeView *)(ui.tabWidget->currentWidget());
        
        
    }*/
    
    void CConnectionManager::uiTabChanged(int index) {
        //ui.toolBar->clear();
        toggleDeviceAction->setEnabled((index > 0) or (ui.overViewList->currentIndex().isValid()));
    }
};
