#ifndef QNUT_CONNECTIONMANAGER_H
#define QNUT_CONNECTIONMANAGER_H

#include <QtGui>
#include <QHash>
#include <libnut/libnut_cli.h>
#include "ui/ui_connman.h"
#include "trayicon.h"
#include "overviewlistmodel.h"
#include "deviceoptions.h"

namespace qnut {
    using namespace libnut;

    class CConnectionManager : public QMainWindow {
        Q_OBJECT
    private:
        Ui::connMan ui;
        
        void createActions();
        void distributeActions(int mode = 0);
    public:
        CDeviceManager deviceManager;
        
        CLog logFile;
        
        QAction * refreshDevicesAction;
        
        QAction * enableDeviceAction;
        QAction * disableDeviceAction;
        
        QMenu overViewMenu;
        CTrayIcon trayicon;
        COverViewListModel overViewListModel;
        CDeviceOptionsHash deviceOptions;

        CConnectionManager(QWidget * parent = 0);
    public slots:
        void uiAddedDevice(CDevice * dev);
        void uiRemovedDevice(CDevice * dev);
        void uiCurrentTabChanged(int index);
        void uiSelectedDeviceChanged(const QItemSelection & selected, const QItemSelection & deselected);
        void uiShowOverViewPopup(const QPoint & pos);
        void uiShowMessage(QString title, QString message);
        void uiShowAbout();
    };
};

#endif
