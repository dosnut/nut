#ifndef QNUT_CONNECTIONMANAGER_H
#define QNUT_CONNECTIONMANAGER_H

#include <QtGui>
#include <QHash>
#include <libnut/libnut_cli.h>
#include "ui/ui_connman.h"
#include "trayicon.h"
#include "overviewmodel.h"
#include "deviceoptions.h"

namespace qnut {
    using namespace libnut;

    class CConnectionManager : public QMainWindow {
        Q_OBJECT
    private:
        Ui::connMan ui;
        
        CDeviceManager deviceManager;
        CLog logFile;
        CDeviceOptionsHash deviceOptions;
        
        CTrayIcon trayicon;
        
        QAction * refreshDevicesAction;
        QAction * enableDeviceAction;
        QAction * disableDeviceAction;
        
        QTabWidget tabWidget;
        QTreeView overView;
        QTextEdit logEdit;
        
        void createActions();
        void distributeActions(int mode = 0);
        
    public:
        CConnectionManager(QWidget * parent = 0);
        
    public slots:
        void uiUpdateTrayIconInfo();
        void uiAddedDevice(CDevice * dev);
        void uiRemovedDevice(CDevice * dev);
        void uiCurrentTabChanged(int index);
        void uiSelectedDeviceChanged(const QItemSelection & selected, const QItemSelection & deselected);
        void uiShowMessage(QString title, QString message);
        void uiShowAbout();
        void uiHandleDeviceStateChanged(DeviceState state);
    };
};

#endif
