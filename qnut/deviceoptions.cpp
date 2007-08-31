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
        enableDeviceAction  = deviceMenu->addAction(QIcon(UI_ICON_ENABLE_DEVICE) , tr("Enable device") , device, SLOT(enable()));
        disableDeviceAction = deviceMenu->addAction(QIcon(UI_ICON_DISABLE_DEVICE), tr("Disable device"), device, SLOT(disable()));
        deviceMenu->addSeparator();
        showAction = deviceMenu->addAction(QIcon(UI_ICON_ENVIRONMENT), tr("Environments..."),
                                 this, SLOT(uiShowThisTab()));
        
        environmentsMenu = new QMenu(this);
        enterEnvironmentAction    = environmentsMenu->addAction(QIcon(UI_ICON_ENTER_ENVIRONMENT), tr("Enter environment"));
        environmentsMenu->addSeparator();
        addEnvironmentAction      = environmentsMenu->addAction(QIcon(UI_ICON_ADD_ENVIRONMENT), tr("Add environment"),
                                    this, SLOT(uiAddEnvironment()));
        removeEnvironmentAction   = environmentsMenu->addAction(QIcon(UI_ICON_REMOVE_ENVIRONMENT), tr("Remove environment"),
                                    this, SLOT(uiRemoveEnvironment()));
        environmentsMenu->addSeparator();
        activateInterfaceAction   = environmentsMenu->addAction(QIcon(UI_ICON_ACTIVATE_INTERFACE), tr("Activate interface"));
        deactivateInterfaceAction = environmentsMenu->addAction(QIcon(UI_ICON_DEACTIVATE_INTERFACE), tr("Deactivate interface"));
        environmentsMenu->addSeparator();
        editInterfaceAction       = environmentsMenu->addAction(QIcon(UI_ICON_EDIT), tr("Edit IP Configuration..."),
                                    this, SLOT(uiChangeIPConfiguration()));
        environmentsMenu->addSeparator();
        addInterfaceAction        = environmentsMenu->addAction(QIcon(UI_ICON_ADD_INTERFACE), tr("Add interface"),
                                    this, SLOT(uiAddInterface()));
        removeInterfaceAction     = environmentsMenu->addAction(QIcon(UI_ICON_REMOVE_INTERFACE), tr("Remove interface"),
                                    this, SLOT(uiRemoveInterface()));
        
        foreach(QAction * i, environmentsMenu->actions()) {
            i->setEnabled(false);
        }
        
        setAllColumnsShowFocus(true);
        
        enableDeviceAction->setDisabled(device->enabled);
        disableDeviceAction->setEnabled(device->enabled);
        setEnabled(device->enabled);
        
        setContextMenuPolicy(Qt::CustomContextMenu);
        setAllColumnsShowFocus(true);
        setAlternatingRowColors(true);
        setIconSize(QSize(18, 18));
        
        header()->setResizeMode(QHeaderView::ResizeToContents);
        
        connect(device, SIGNAL(enabledChanged(bool)), enableDeviceAction , SLOT(setDisabled(bool)));
        connect(device, SIGNAL(enabledChanged(bool)), disableDeviceAction, SLOT(setEnabled(bool)));
        connect(device, SIGNAL(enabledChanged(bool)), this, SLOT(setEnabled(bool)));
        connect(device, SIGNAL(enabledChanged(bool)), this, SLOT(updateDeviceIcons()));
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
        delete environmentsMenu;
        delete deviceMenu;
    }
    
    void CDeviceOptions::uiShowThisTab() {
        tabWidget->setCurrentWidget(this);
    }
    
    void CDeviceOptions::uiUpdateDeviceIcons() {
        tabWidget->setTabIcon(tabWidget->indexOf(this), QIcon(getDeviceIcon(device)));
        deviceMenu->setIcon(QIcon(getDeviceIcon(device)));
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
    
    void CDeviceOptions::uiShowPopup(const QPoint & pos) {
        environmentsMenu->exec(mapToGlobal(pos));
    }
    
    void CDeviceOptions::uiHandleEnvironmentChange(CEnvironment * current, CEnvironment * previous) {
        if ((current) && (current->interfaces.isEmpty()))
            emit showMessage(tr("User defined environment entered"), device->name + ' ' + tr("entered an environment, that needs to be configured in order to be active.\n\n Click here to open the connection manager."));
    }
    
    void CDeviceOptions::uiChangeIPConfiguration() {
        CIPConfiguration dialog(this);
        QModelIndex selectedIndex = (selectionModel()->selection().indexes())[0];
        
        dialog.execute((CInterface *)(selectedIndex.internalPointer()));
    }
    void CDeviceOptions::uiAddEnvironment() {
        QString newName;
        newName = QInputDialog::getText(this, tr("New environment"), tr("Please enter a unique name for the new environment."));
        //hier weiter!!
    }
    
    void CDeviceOptions::uiRemoveEnvironment() {
    
    }
    void CDeviceOptions::uiAddInterface() {
        QModelIndex selectedIndex = (selectionModel()->selection().indexes())[0];
        CIPConfiguration dialog(this);
        bool isStatic;
        QHostAddress ip, netmask, gateway;
        
        dialog.execute(isStatic, ip, netmask, gateway);
        ((CEnvironment *)(selectedIndex.internalPointer()))->addInterface(isStatic, ip, netmask, gateway);
    }
    
    void CDeviceOptions::uiRemoveInterface() {
    
    }
};
