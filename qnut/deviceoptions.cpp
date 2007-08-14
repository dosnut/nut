//
// C++ Implementation: deviceoptions
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "deviceoptions.h"
#include "deviceoptionsmodel.h"
#include "ipconfiguration.h"
#include "constants.h"

namespace qnut {
    CDeviceOptions::CDeviceOptions(CDevice * parentDevice, QTabWidget * parentTabWidget, QWidget * parent) : QTreeView(parent) {
        device = parentDevice;
        tabWidget = parentTabWidget;
        setModel(new CDeviceOptionsModel(device));
        deviceMenu = new QMenu(device->properties.name, NULL);
        enableDeviceAction  = deviceMenu->addAction(QIcon(UI_ICON_ENABLE_DEVICE) , tr("Enable device") , device, SLOT(enable()));
        disableDeviceAction = deviceMenu->addAction(QIcon(UI_ICON_DISABLE_DEVICE), tr("Disable device"), device, SLOT(disable()));
        deviceMenu->addSeparator();
        showAction = deviceMenu->addAction(QIcon(UI_ICON_ENVIRONMENT), tr("Environments..."), this, SLOT(showThisTab()));
        
        environmentsMenu = new QMenu(this);
        enterEnvironmentAction    = environmentsMenu->addAction(QIcon(UI_ICON_ENTER_ENVIRONMENT), tr("Enter environment"));
        environmentsMenu->addSeparator();
        activateInterfaceAction   = environmentsMenu->addAction(QIcon(UI_ICON_ACTIVATE_INTERFACE), tr("Activate interface"));
        deactivateInterfaceAction = environmentsMenu->addAction(QIcon(UI_ICON_DEACTIVATE_INTERFACE), tr("Deactivate interface"));
        editInterfaceAction       = environmentsMenu->addAction(QIcon(UI_ICON_EDIT), tr("Edit IP Configuration..."),
                                    this, SLOT(changeIPConfiguration()));
        
        enterEnvironmentAction->setEnabled(false);
        activateInterfaceAction->setEnabled(false);
        deactivateInterfaceAction->setEnabled(false);
        editInterfaceAction->setEnabled(false);
        
        enableDeviceAction->setDisabled(device->properties.enabled);
        disableDeviceAction->setEnabled(device->properties.enabled);
        setEnabled(device->properties.enabled);
        
        setContextMenuPolicy(Qt::CustomContextMenu);
        
        connect(device, SIGNAL(stateChanged(bool)), enableDeviceAction , SLOT(setDisabled(bool)));
        connect(device, SIGNAL(stateChanged(bool)), disableDeviceAction, SLOT(setEnabled(bool)));
        connect(device, SIGNAL(stateChanged(bool)), this, SLOT(setEnabled(bool)));
        connect(device, SIGNAL(environmentsUpdated()), this, SLOT(repaint()));
        
        connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                this            , SLOT(selectionChanged(const QItemSelection &, const QItemSelection &)));
        connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showPopup(const QPoint &)));
    }
    
    CDeviceOptions::~CDeviceOptions() {
//nicht nötig?
//        disconnect(device, SIGNAL(stateChanged(bool)), enableDeviceAction , SLOT(setDisabled(bool)));
//        disconnect(device, SIGNAL(stateChanged(bool)), disableDeviceAction, SLOT(setEnabled(bool)));
//        disconnect(device, SIGNAL(stateChanged(bool)), this, SLOT(setEnabled(bool)));
//        CDeviceOptionsModel * targetTreeModel = (CDeviceOptionsModel *)targetDeviceOptions.environmentsTree->model();
//        targetDeviceOptions.environmentsTree->setModel(NULL);
//        delete targetTreeModel;
        //delete environmentsMenu;
        delete deviceMenu;
    }
    
    void CDeviceOptions::showThisTab() {
        tabWidget->setCurrentWidget(this);
    }
    
    void CDeviceOptions::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected) {
        QModelIndexList deselectedIndexes = deselected.indexes();
        QModelIndexList selectedIndexes = selected.indexes();
        
        if (!deselectedIndexes.isEmpty()) {
            QModelIndex targetIndex = deselectedIndexes[0];
            if (targetIndex.column() == 0) {
                CEnvironment * target = (CEnvironment *)(targetIndex.internalPointer());
                disconnect(target, SIGNAL(stateChanged(bool)), enterEnvironmentAction, SLOT(setDisabled(bool)));
                disconnect(enterEnvironmentAction, SIGNAL(triggered()), target, SLOT());
            }
            else {
                CInterface * target = (CInterface *)(targetIndex.internalPointer());
                disconnect(target, SIGNAL(stateChanged(bool)), activateInterfaceAction, SLOT(setDisabled(bool)));
                disconnect(target, SIGNAL(stateChanged(bool)), deactivateInterfaceAction, SLOT(setEnabled(bool)));
                disconnect(activateInterfaceAction, SIGNAL(triggered()), target, SLOT(activate()));
                disconnect(deactivateInterfaceAction, SIGNAL(triggered()), target, SLOT(deactivate()));
            }
        }
        
        if (!selectedIndexes.isEmpty()) {
            QModelIndex targetIndex = selectedIndexes[0];
            if (targetIndex.column() == 0) {
                CEnvironment * target = (CEnvironment *)(targetIndex.internalPointer());
                connect(target, SIGNAL(stateChanged(bool)), enterEnvironmentAction, SLOT(setDisabled(bool)));
                connect(enterEnvironmentAction, SIGNAL(triggered()), target, SLOT());
                
                enterEnvironmentAction->setDisabled(target->properties.active);
                activateInterfaceAction->setEnabled(false);
                deactivateInterfaceAction->setEnabled(false);
                editInterfaceAction->setEnabled(false);
            }
            else if (targetIndex.column() > 0) {
                CInterface * target = (CInterface *)(targetIndex.internalPointer());
                connect(target, SIGNAL(stateChanged(bool)), activateInterfaceAction, SLOT(setDisabled(bool)));
                connect(target, SIGNAL(stateChanged(bool)), deactivateInterfaceAction, SLOT(setEnabled(bool)));
                connect(activateInterfaceAction, SIGNAL(triggered()), target, SLOT(activate()));
                connect(deactivateInterfaceAction, SIGNAL(triggered()), target, SLOT(deactivate()));
                
                enterEnvironmentAction->setEnabled(false);
                activateInterfaceAction->setDisabled(target->properties.active);
                deactivateInterfaceAction->setEnabled(target->properties.active);
                editInterfaceAction->setEnabled(true);
            }
        }
        else {
            enterEnvironmentAction->setEnabled(false);
            activateInterfaceAction->setEnabled(false);
            deactivateInterfaceAction->setEnabled(false);
            editInterfaceAction->setEnabled(false);
        }
    }
    
    void CDeviceOptions::showPopup(const QPoint & pos) {
        environmentsMenu->exec(mapToGlobal(pos));
    }
    
    void CDeviceOptions::changeIPConfiguration() {
        CIPConfiguration dialog(this);
        QModelIndex selectedIndex = (selectionModel()->selection().indexes())[0];
        dialog.execute((CInterface *)(selectedIndex.internalPointer()));
    }
};
