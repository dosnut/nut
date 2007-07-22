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
        
        enableDeviceAction = new QAction(QIcon(UI_ICON_ENABLE_DEVICE), tr("Enable Device"), ui.toolBar);
        enableDeviceAction->setCheckable(true);
        enableDeviceAction->setEnabled(false);
        
        disableDeviceAction = new QAction(QIcon(UI_ICON_DISABLE_DEVICE), tr("Disable Device"), ui.toolBar);
        disableDeviceAction->setCheckable(true);
        disableDeviceAction->setEnabled(false);
        
        toggleEnvironmentAction = new QAction(QIcon(UI_ICON_TOGGLE_ENVIRONMENT), tr("(De-)Activate Environment"), ui.toolBar);
        toggleEnvironmentAction->setCheckable(true);
        toggleEnvironmentAction->setEnabled(false);
        
        toggleInterfaceAction = new QAction(QIcon(UI_ICON_TOGGLE_INTERFACE), tr("(De-)Activate Interface"), ui.toolBar);
        toggleInterfaceAction->setCheckable(true);
        toggleInterfaceAction->setEnabled(false);
        
        ui.toolBar->addAction(enableDeviceAction);
        ui.toolBar->addAction(disableDeviceAction);
        ui.toolBar->addSeparator();
        ui.toolBar->addAction(toggleEnvironmentAction);
        ui.toolBar->addAction(toggleInterfaceAction);
        
        //connect(ui.overViewList, SIGNAL(customContextMenuRequested (const QPoint)),
        //        this           , SLOT(uiShowOverviewPopup(const QPoint)));
        connect(ui.overViewList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                this                             , SLOT(uiCurrentDeviceChanged(const QItemSelection &, const QItemSelection &)));
        connect(ui.tabWidget, SIGNAL(currentChanged(int)),
                this      , SLOT(uiCurrentTabChanged(int)));
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
        connect(dev, SIGNAL(stateChanged()), ui.overViewList, SLOT(repaint())); //stateChanged fehlt
        
        ui.overViewList->repaint();
    }
    
    void CConnectionManager::uiRemovedDevice(CDevice * dev) {
        disconnect(dev, SIGNAL(stateChanged()), ui.overViewList, SLOT(repaint()));
        
        QTreeView * targetDeviceOptionsTab = deviceOptionsTabs[dev->name];
        CDeviceOptionsModel * targetDeviceOptionsModel = (CDeviceOptionsModel *)targetDeviceOptionsTab->model();
        
        ui.tabWidget->removeTab(ui.tabWidget->indexOf(targetDeviceOptionsTab));
        targetDeviceOptionsTab->setModel(NULL);
        deviceOptionsTabs.remove(dev->name);
        delete targetDeviceOptionsModel;
        delete targetDeviceOptionsTab;
        
        ui.overViewList->repaint();
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
    
    void CConnectionManager::uiCurrentTabChanged(int index) {
        enableDeviceAction->setEnabled((index > 0) or (ui.overViewList->selectionModel()->hasSelection()));
        disableDeviceAction->setEnabled((index > 0) or (ui.overViewList->selectionModel()->hasSelection()));
    }
    
    void CConnectionManager::uiCurrentDeviceChanged(const QItemSelection & selected, const QItemSelection & deselected) {
        CDevice * currentDevice;
        QModelIndexList::iterator i, iEnd;
        
        iEnd = deselected.indexes().end();
        for (i = deselected.indexes().begin(); i != iEnd; i++) {
            currentDevice = (CDevice *)(i->internalPointer());
            disconnect(enableDeviceAction, SIGNAL(triggerd()), currentDevice, SLOT(enable()));
            disconnect(disableDeviceAction, SIGNAL(triggerd()), currentDevice, SLOT(disable()));
        }
        
        iEnd = selected.indexes().end();
        for (i = selected.indexes().begin(); i != iEnd; i++) {
            currentDevice = (CDevice *)(i->internalPointer());
            connect(enableDeviceAction, SIGNAL(triggerd()), currentDevice, SLOT(enable()));
            connect(disableDeviceAction, SIGNAL(triggerd()), currentDevice, SLOT(disable()));
        }
        
        enableDeviceAction->setEnabled(selected.indexes().count() > 0);
        disableDeviceAction->setEnabled(selected.indexes().count() > 0);
    }
};
