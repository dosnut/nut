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
#include "libnut_cli.h"

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
        
        //bool undefined;
        
        QMenu * deviceMenu;
        QMenu * environmentsMenu;
        QAction * enableDeviceAction;
        QAction * disableDeviceAction;
        QAction * enterEnvironmentAction;
        QAction * activateInterfaceAction;
        QAction * deactivateInterfaceAction;
        QAction * editInterfaceAction;
        
        QAction * showAction;
        
        CDeviceOptions(CDevice * parentDevice, QTabWidget * parentTabWidget, QWidget * parent = 0);
        ~CDeviceOptions();
        
    public slots:
        void showThisTab();
        void updateDeviceIcons();
        void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
        void showPopup(const QPoint & pos);
        void changeIPConfiguration();
        void handleEnvironmentChange(CEnvironment * current, CEnvironment * previous); //todo
        
    signals:
        void showMessage(QString title, QString message);
    };
};

#endif
