#ifndef QNUT_CONNMAN_H
#define QNUT_CONNMAN_H

#include <QtGui>
#include "ui_connman.h"
#include "trayicon.h"
#include "common.h"

namespace qnut {
    class CuiDevice : public QListWidgetItem {
        Q_OBJECT
    public slots:
        void uiEnvironmentAdd();
        void uiEnvironmentRemove();
    };
    
    class CuiEnvironment : public QListWidgetItem {
        Q_OBJECT
    public slots:
        void uiInterfaceAdd();
        void uiInterfacePemove();
    };

    class CConnectionManager : public QMainWindow {
        Q_OBJECT
    private:
        Ui::ConnMan ui;
    public:
        CTrayIcon trayicon;
        CDeviceManager DeviceManager;
        
        CConnectionManager(QWidget * parent = 0);
    public slots:
        void uiDeviceAdd(QString name);
        void uiDeviceRemove(QString name);
    };
};

#endif
