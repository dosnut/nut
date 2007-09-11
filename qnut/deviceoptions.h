//
// C++ Interface: deviceoptions
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QNUT_DEVICEOPTIONS_H
#define QNUT_DEVICEOPTIONS_H

#include <QTreeView>
#include <QMenu>
#include <QTabWidget>
#include <libnut/libnut_cli.h>

namespace qnut {
    using namespace libnut;
    
    class CDeviceOptions;
    typedef QHash<CDevice *, CDeviceOptions *> CDeviceOptionsHash;
    
    class CDeviceOptions : public QTreeView {
        Q_OBJECT
    protected:
        QTabWidget * tabWidget;
    public:
        CDevice * device;
        
        QMenu * deviceMenu;
        QAction * enableDeviceAction;
        QAction * disableDeviceAction;
        QAction * enterEnvironmentAction;
/*        QAction * activateInterfaceAction;
        QAction * deactivateInterfaceAction;*/
        QAction * editInterfaceAction;
        
        QAction * showAction;
        
        void updateDeviceIcons();
        
        CDeviceOptions(CDevice * parentDevice, QTabWidget * parentTabWidget, QWidget * parent = 0);
        ~CDeviceOptions();
        
    public slots:
        void uiShowThisTab();
        void uiSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
        void uiHandleEnvironmentChange(CEnvironment * current, CEnvironment * previous);
        void uiChangeIPConfiguration();
        void uiHandleStateChange(DeviceState state);
        
    signals:
        void showMessage(QString title, QString message);
    };
};

#endif
