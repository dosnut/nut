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

    class CConnectionManager : public QMainWindow {
        Q_OBJECT
    private:
        Ui::ConnMan ui;
    public:
        CDeviceManager deviceManager;
        
        QAction * enableDeviceAction;
        QAction * disableDeviceAction;
        QAction * toggleEnvironmentAction;
        QAction * toggleInterfaceAction;
        QAction * showDeviceOptionsAction;
        
//        QMenu overViewMenu;
        CTrayIcon trayicon;
        COverViewListModel overViewListModel;
        QHash<QString, QTreeView *> deviceOptionsTabs;

        CConnectionManager(QWidget * parent = 0);
    public slots:
        void uiShowDeviceOptions(CDevice * dev);
        void uiAddedDevice(CDevice * dev);
        void uiRemovedDevice(CDevice * dev);
        void uiCurrentTabChanged(int index);
        void uiCurrentDeviceChanged(const QItemSelection & selected, const QItemSelection & deselected);
        //void uiShowOverViewPopup(const QPoint & pos);
        //void uiShowDeviceOptionsPopup(const QPoint & pos);
    };
};

#endif
