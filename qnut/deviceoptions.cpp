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
        
        enableDeviceAction->setDisabled(device->properties.enabled);
        disableDeviceAction->setEnabled(device->properties.enabled);
        setEnabled(device->properties.enabled);
        
        connect(device, SIGNAL(stateChanged(bool)), enableDeviceAction , SLOT(setDisabled(bool)));
        connect(device, SIGNAL(stateChanged(bool)), disableDeviceAction, SLOT(setEnabled(bool)));
        connect(device, SIGNAL(stateChanged(bool)), this, SLOT(setEnabled(bool)));
    }
    
    CDeviceOptions::~CDeviceOptions() {
//nicht nötig?
//        disconnect(device, SIGNAL(stateChanged(bool)), enableDeviceAction , SLOT(setDisabled(bool)));
//        disconnect(device, SIGNAL(stateChanged(bool)), disableDeviceAction, SLOT(setEnabled(bool)));
//        disconnect(device, SIGNAL(stateChanged(bool)), this, SLOT(setEnabled(bool)));
//        CDeviceOptionsModel * targetTreeModel = (CDeviceOptionsModel *)targetDeviceOptions.environmentsTree->model();
//        targetDeviceOptions.environmentsTree->setModel(NULL);
//        delete targetTreeModel;
        delete deviceMenu;
    }
    
    void CDeviceOptions::showThisTab() {
        tabWidget->setCurrentWidget(this);
    }
};
