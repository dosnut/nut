#ifndef QNUT_CONNECTIONMANAGER_H
#define QNUT_CONNECTIONMANAGER_H

#include <QtGui>
#include <QHash>
#include "ui_connman.h"
#include "trayicon.h"
#include "overviewlistmodel.h"
#include "deviceoptionsmodel.h"
#include "libnut_cli.h"

namespace qnut {
    using namespace libnut;

    struct DeviceOptions {
        QTreeView * environmentsTree;
        QMenu * contextMenu;
        QAction * enableAction;
        QAction * disableAction;
        //QAction * showTreeAction;
    };

    class CConnectionManager : public QMainWindow {
        Q_OBJECT
    private:
        Ui::ConnMan ui;
    public:
        CDeviceManager deviceManager;
        
        //QAction * enableDeviceAction;
        //QAction * disableDeviceAction;
        //QAction * toggleEnvironmentAction;
        //QAction * toggleInterfaceAction;
        //QAction * showEnvironmentsTreeAction;
        //QMenu overViewMenu;
        
        CTrayIcon trayicon;
        COverViewListModel overViewListModel;
        QHash<QString, DeviceOptions> deviceOptions;

        CConnectionManager(QWidget * parent = 0);
    public slots:
        void uiAddedDevice(CDevice * dev);
        void uiRemovedDevice(CDevice * dev);
        //void uiCurrentTabChanged(int index);
        //void uiCurrentDeviceChanged();
        void uiShowOverViewPopup(const QPoint & pos);
        //void uiShowEnvironmentsTree();
    };
};

#endif
