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
        QAction * addEnvironmentAction;
        QAction * removeEnvironmentAction;
        QAction * addInterfaceAction;
        QAction * removeInterfaceAction;
        
        QAction * showAction;
        
        CDeviceOptions(CDevice * parentDevice, QTabWidget * parentTabWidget, QWidget * parent = 0);
        ~CDeviceOptions();
        
    public slots:
        void uiShowThisTab();
        void uiUpdateDeviceIcons();
        void uiSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
        void uiShowPopup(const QPoint & pos);
        void uiHandleEnvironmentChange(CEnvironment * current, CEnvironment * previous);
        void uiChangeIPConfiguration();
        void uiAddEnvironment();
        void uiRemoveEnvironment();
        void uiAddInterface();
        void uiRemoveInterface();
        
    signals:
        void showMessage(QString title, QString message);
    };
};

#endif
