#include "connectionmanager.h"
#include "constants.h"
#include <iostream>
#include <QDate>

namespace qnut {
    CConnectionManager::CConnectionManager(QWidget * parent) :
        QMainWindow(parent),
        deviceManager(this),
        logFile(UI_FILE_LOG),
        trayicon(this),
        overViewListModel(&(deviceManager.devices))
    {
        deviceOptions.reserve(10);
        ui.setupUi(this);
        if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text | QIODevice::Append))
            uiPrintToLog(tr("ERROR:") + " " + tr("Cannot create/open log file."));
        
        uiPrintToLog(tr("Network UTility (NUT) client QNUT started."));
        uiPrintToLog(QDateTime::currentDateTime().toString());
        
        try {
            deviceManager.init();
        }
        catch (Exception & e) {
            uiPrintToLog(tr("ERROR:") + " " + QString(e.what()));
        }
        
        ui.overViewList->setContextMenuPolicy(Qt::CustomContextMenu);
        ui.overViewList->setModel(&overViewListModel);
        
        overViewMenu.addAction(QIcon(UI_ICON_REFRESH), tr("Refresh devices"), &deviceManager, SLOT(refreshAll()));
        
        overViewMenu.addSeparator();
        
        enableDeviceAction = overViewMenu.addAction(QIcon(UI_ICON_ENABLE_DEVICE), tr("Enable Device"));
        enableDeviceAction->setEnabled(false);
        
        disableDeviceAction = overViewMenu.addAction(QIcon(UI_ICON_DISABLE_DEVICE), tr("Disable Device"));
        disableDeviceAction->setEnabled(false);
        
        ui.toolBar->addActions(overViewMenu.actions());
        ui.menuDevice->addActions(overViewMenu.actions());
        
        connect(ui.actionAboutQt, SIGNAL(triggered()), qApp , SLOT(aboutQt()));
        connect(ui.overViewList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                this                             , SLOT(uiSelectedDeviceChanged(const QItemSelection &, const QItemSelection &)));
        connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(uiCurrentTabChanged(int)));
        connect(ui.overViewList, SIGNAL(customContextMenuRequested(const QPoint)), this, SLOT(uiShowOverViewPopup(const QPoint)));
        connect(&trayicon, SIGNAL(messageClicked()), this, SLOT(show()));
        connect(&deviceManager, SIGNAL(deviceAdded(CDevice *)), this, SLOT(uiAddedDevice(CDevice *)));
        connect(&deviceManager, SIGNAL(deviceRemoved(CDevice *)), this, SLOT(uiRemovedDevice(CDevice *)));
        
        trayicon.show();
    }
    
    void CConnectionManager::uiAddedDevice(CDevice * dev) {
        CDeviceOptions * newDeviceOptions = new CDeviceOptions(dev, ui.tabWidget);
        
        ui.tabWidget->insertTab(ui.tabWidget->count()-1, newDeviceOptions, dev->name);
        newDeviceOptions->updateDeviceIcons();
        
        deviceOptions.insert(dev, newDeviceOptions);
        trayicon.devicesMenu.addMenu(newDeviceOptions->deviceMenu);
        trayicon.devicesMenu.setEnabled(true);
        
        ui.overViewList->repaint();
        connect(dev, SIGNAL(stateChanged(bool)), ui.overViewList, SLOT(repaint()));
        connect(newDeviceOptions->showAction, SIGNAL(triggered()), this, SLOT(show()));
        connect(dev, SIGNAL(printToLog(QString)), this, SLOT(uiPrintToLog(QString)));
    }
    
    void CConnectionManager::uiRemovedDevice(CDevice * dev) {
        CDeviceOptions * target = deviceOptions[dev];
        
        //disconnect(dev, SIGNAL(stateChanged(bool)), ui.overViewList, SLOT(repaint())); nicht nötig?
        
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
            ui.toolBar->addActions(overViewMenu.actions());
            ui.menuDevice->addActions(overViewMenu.actions());
        }
        else if (index == ui.tabWidget->count()-1) {
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
            
            enableDeviceAction->setDisabled(selectedDevice->enabled);
            disableDeviceAction->setEnabled(selectedDevice->enabled);
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
    
    void CConnectionManager::uiPrintToLog(QString output) {
        ui.logEdit->append(output + "\n");
        if (logFile.error() == QFile::NoError) {
            QTextStream outStream(&logFile);
            outStream << output << endl;
        }
    }
};
