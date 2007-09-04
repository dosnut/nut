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
#include "common.h"
#include <QHeaderView>
#include <QInputDialog>

namespace qnut {
    CDeviceOptions::CDeviceOptions(CDevice * parentDevice, QTabWidget * parentTabWidget, QWidget * parent) : QTreeView(parent) {
        device = parentDevice;
        tabWidget = parentTabWidget;
        setModel(new CDeviceOptionsModel(device));
        
        deviceMenu = new QMenu(device->name, NULL);
        enableDeviceAction  = deviceMenu->addAction(QIcon(UI_ICON_DEVICE_ENABLE) , tr("Enable device") , device, SLOT(enable()));
        disableDeviceAction = deviceMenu->addAction(QIcon(UI_ICON_DEVICE_DISABLE), tr("Disable device"), device, SLOT(disable()));
        deviceMenu->addSeparator();
        showAction = deviceMenu->addAction(QIcon(UI_ICON_ENVIRONMENT), tr("Environments..."),
                                 this, SLOT(uiShowThisTab()));
        
        //environmentsMenu = new QMenu(this);
        QAction * separator;
        separator                 = new QAction(this);
        enterEnvironmentAction    = new QAction(QIcon(UI_ICON_ENVIRONMENT_ENTER), tr("Enter environment"), this);
        activateInterfaceAction   = new QAction(QIcon(UI_ICON_INTERFACE_ACTIVATE), tr("Activate interface"), this);
        deactivateInterfaceAction = new QAction(QIcon(UI_ICON_INTERFACE_DEACTIVATE), tr("Deactivate interface"), this);
        editInterfaceAction       = new QAction(QIcon(UI_ICON_EDIT), tr("Edit IP Configuration..."), this);
        
        foreach(QAction * i, environmentsMenu->actions()) {
            i->setEnabled(false);
        }
        separator->setSeparator(true);
        
        addAction(enterEnvironmentAction);
        addAction(separator);
        addAction(activateInterfaceAction);
        addAction(deactivateInterfaceAction);
        addAction(separator);
        addAction(editInterfaceAction);
        
        setAllColumnsShowFocus(true);
        
        setDisabled(device->state == DS_DEACTIVATED);
        enableDeviceAction->setDisabled(device->state == DS_UP);
        disableDeviceAction->setDisabled(device->state == DS_DEACTIVATED);
        
        setContextMenuPolicy(Qt::ActionsContextMenu);
        setAllColumnsShowFocus(true);
        setAlternatingRowColors(true);
        setIconSize(QSize(18, 18));
        
        header()->setResizeMode(QHeaderView::ResizeToContents);
        
        connect(editInterfaceAction, SIGNAL(triggered()), this, SLOT(uiChangeIPConfiguration()));
        connect(device, SIGNAL(stateChanged(DeviceState)), this, SLOT(uiHandleStateChange(DeviceState)));
        connect(device, SIGNAL(environmentsUpdated()), this, SLOT(repaint()));
        
        connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                this            , SLOT(selectionChanged(const QItemSelection &, const QItemSelection &)));
        connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(uiShowPopup(const QPoint &)));
    }
    
    CDeviceOptions::~CDeviceOptions() {
//nicht nötig?
       disconnect(device, SIGNAL(stateChanged(bool)), enableDeviceAction , SLOT(setDisabled(bool)));
       disconnect(device, SIGNAL(stateChanged(bool)), disableDeviceAction, SLOT(setEnabled(bool)));
       disconnect(device, SIGNAL(stateChanged(bool)), this, SLOT(setEnabled(bool)));
//        CDeviceOptionsModel * targetTreeModel = (CDeviceOptionsModel *)targetDeviceOptions.environmentsTree->model();
//        targetDeviceOptions.environmentsTree->setModel(NULL);
//        delete targetTreeModel;
        delete environmentsMenu;
        delete deviceMenu;
    }
    
    void CDeviceOptions::updateDeviceIcons() {
        tabWidget->setTabIcon(tabWidget->indexOf(this), QIcon(getDeviceIcon(device)));
        deviceMenu->setIcon(QIcon(getDeviceIcon(device)));
    }
    
    void CDeviceOptions::uiShowThisTab() {
        tabWidget->setCurrentWidget(this);
    }
    
    void CDeviceOptions::uiSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected) {
        QModelIndexList deselectedIndexes = deselected.indexes();
        QModelIndexList selectedIndexes = selected.indexes();
        
        if (!deselectedIndexes.isEmpty()) {
            QModelIndex targetIndex = deselectedIndexes[0];
            if (!targetIndex.parent().isValid()) {
                CEnvironment * target = (CEnvironment *)(targetIndex.internalPointer());
                disconnect(target, SIGNAL(activeChanged(bool)), enterEnvironmentAction, SLOT(setDisabled(bool)));
                disconnect(enterEnvironmentAction, SIGNAL(triggered()), target, SLOT(enter()));
            }
            else {
                CInterface * target = (CInterface *)(targetIndex.internalPointer());
                disconnect(target, SIGNAL(activeChanged(bool)), activateInterfaceAction, SLOT(setDisabled(bool)));
                disconnect(target, SIGNAL(activeChanged(bool)), deactivateInterfaceAction, SLOT(setEnabled(bool)));
                disconnect(activateInterfaceAction, SIGNAL(triggered()), target, SLOT(activate()));
                disconnect(deactivateInterfaceAction, SIGNAL(triggered()), target, SLOT(deactivate()));
            }
        }
        
        if (!selectedIndexes.isEmpty()) {
            QModelIndex targetIndex = selectedIndexes[0];
            if (!targetIndex.parent().isValid()) {
                CEnvironment * target = (CEnvironment *)(targetIndex.internalPointer());
                connect(target, SIGNAL(activeChanged(bool)), enterEnvironmentAction, SLOT(setDisabled(bool)));
                connect(enterEnvironmentAction, SIGNAL(triggered()), target, SLOT(enter()));
                
                enterEnvironmentAction->setDisabled(target->active);
                activateInterfaceAction->setEnabled(false);
                deactivateInterfaceAction->setEnabled(false);
                editInterfaceAction->setEnabled(false);
            }
            else {
                CInterface * target = (CInterface *)(targetIndex.internalPointer());
                connect(target, SIGNAL(activeChanged(bool)), activateInterfaceAction, SLOT(setDisabled(bool)));
                connect(target, SIGNAL(activeChanged(bool)), deactivateInterfaceAction, SLOT(setEnabled(bool)));
                connect(activateInterfaceAction, SIGNAL(triggered()), target, SLOT(activate()));
                connect(deactivateInterfaceAction, SIGNAL(triggered()), target, SLOT(deactivate()));
                
                enterEnvironmentAction->setEnabled(false);
                activateInterfaceAction->setDisabled(target->active);
                deactivateInterfaceAction->setEnabled(target->active);
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
    
/*    void CDeviceOptions::uiShowPopup(const QPoint & pos) {
        environmentsMenu->exec(mapToGlobal(pos));
    }*/
    
    void CDeviceOptions::uiHandleEnvironmentChange(CEnvironment * current, CEnvironment * previous) {
/*        if ((current) && (current->interfaces.isEmpty()))
            emit showMessage(tr("User defined environment entered"), device->name + ' ' + tr("entered an environment, that needs to be configured in order to be active.\n\n Click here to open the connection manager."));*/
    }
    
    void CDeviceOptions::uiChangeIPConfiguration() {
        CIPConfiguration dialog(this);
        QModelIndex selectedIndex = (selectionModel()->selection().indexes())[0];
        
        dialog.execute((CInterface *)(selectedIndex.internalPointer()));
    }
    
    void CDeviceOptions::uiHandleStateChange(DeviceState state) {
        updateDeviceIcons();
        setDisabled(state == DS_DEACTIVATED);
        enableDeviceAction->setDisabled(state == DS_UP);
        disableDeviceAction->setDisabled(state == DS_DEACTIVATED);
    }
};
