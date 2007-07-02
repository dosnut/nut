#ifndef QNUT_CONNECTIONMANAGER_H
#define QNUT_CONNECTIONMANAGER_H

#include <QtGui>
#include "ui_connman.h"
#include "trayicon.h"
#include "common.h"

namespace qnut {
    class CuiDevice : public QTreeWidgetItem {
    public:
        CDevice * device;
        CuiDevice(QTreeWidget * parent) : QTreeWidgetItem(parent) {};
    };
    
    class CuiEnvironment : public QTreeWidgetItem {
    public:
        CEnvironment * environment;
        CuiEnvironment(QTreeWidgetItem * parent) : QTreeWidgetItem(parent) {};
    };
    
    class CuiInterface : public QTreeWidgetItem {
    public:
        CInterface * interface;
        CuiInterface(QTreeWidget * parent) : QTreeWidgetItem(parent) {};
    };
    
    class CConnectionManager : public QMainWindow {
        Q_OBJECT
    private:
        Ui::ConnMan ui;
    public:
        CTrayIcon trayicon;
        QMenu overviewMenu;
        CDeviceManager deviceManager;
        
        CConnectionManager(QWidget * parent = 0);
    public slots:
        void uiShowOverviewPopup(const QPoint & pos);
        void uiUpdateDevices();
        void uiUpdateInterfaces();
    };
};

#endif
