#include "connectionmanager.h"
#include "constants.h"
#include <iostream>

namespace qnut {
    //CConnectionManager
    CConnectionManager::CConnectionManager(QWidget * parent) : QMainWindow(parent),
                                                               trayicon(this),
                                                               deviceManager(this)  {
        ui.setupUi(this);
        connect(ui.overviewWidget, SIGNAL(customContextMenuRequested (const QPoint)),
                this             , SLOT(uiShowOverviewPopup(const QPoint)));
        
        ui.overviewWidget->setContextMenuPolicy(Qt::CustomContextMenu);
        
        trayicon.show();
    }
    
    void CConnectionManager::uiShowOverviewPopup(const QPoint & pos) {
        QTreeWidgetItem * currentItem = ui.overviewWidget->currentItem();
        
        if (currentItem != NULL) {
            if ((QTreeWidget *)currentItem->parent() == ui.overviewWidget) {
                if (((CuiDevice *)currentItem)->device->enabled) {
                    overviewMenu.addAction(
                        tr("disable"),
                        ((CuiDevice *)currentItem)->device,
                        SLOT(disable())
                    );
                }
                else {
                    overviewMenu.addAction(
                        tr("enable"),
                        ((CuiDevice *)currentItem)->device,
                        SLOT(enable())
                    );
                }
            }
            else {
                overviewMenu.addAction(
                        tr("activate"),
                        ((CuiEnvironment *)currentItem)->environment,
                        SLOT(activate())
                    )->setEnabled(not ((CuiEnvironment *)currentItem)->environment->active);
            }
            overviewMenu.exec(ui.overviewWidget->mapToGlobal(pos));
            overviewMenu.clear();
        }
//        std::cout << "x: " << pos.x() << "; y: " << pos.y() << std::endl;
    }
    
    void CConnectionManager::uiUpdateDevices() {
        CDeviceList::iterator i, iEnd;
        CEnvironmentList::iterator j, jEnd;
        CuiDevice * newDevice;
        CuiEnvironment * newEnvironment;
        
       iEnd = deviceManager.devices.end();
        for (i = deviceManager.devices.begin(); i != iEnd; i++) {
            newDevice = new CuiDevice(ui.overviewWidget);
            newDevice->setText(0, (*i)->name);
            newDevice->setIcon(0, QIcon(UI_ICON_DEVICE));
            newDevice->device = *i;
            
            jEnd = newDevice->device->environments.end();
            for (j = newDevice->device->environments.begin(); j != jEnd; j++) {
                newEnvironment = new CuiEnvironment(newDevice);
                newEnvironment->setText(0, (*j)->name);
                newEnvironment->setIcon(0, QIcon(UI_ICON_DEVICE));
                newEnvironment->environment = *j;
            }
        }
    }
    
    void CConnectionManager::uiUpdateInterfaces() {
        CInterfaceList::Iterator i, iEnd;
        CuiInterface * newInterface;
    }
};
