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
    class CConnectionManager : public QMainWindow {
        Q_OBJECT
    private:
        Ui::ConnMan ui;
    public:
        CDeviceManager deviceManager;
        
        QAction * toggleDeviceAction;
        QAction * toggleEnvironmentAction;
        QAction * toggleInterfaceAction;
        
        QMenu overViewMenu;
        QMenu deviceOptionsMenu;
        CTrayIcon trayicon;
        COverViewListModel overViewListModel;
        QHash<QString, QTreeView *> deviceOptionsTabs;

        CConnectionManager(QWidget * parent = 0);
    public slots:
        void uiAddedDevice(CDevice * dev);
        void uiRemovedDevice(CDevice * dev);
        //void uiShowOverViewPopup(const QPoint & pos);
        //void uiShowDeviceOptionsPopup(const QPoint & pos);
        void uiTabChanged(int index);
        //void uiChangeDeviceState();
    };
};

#endif
