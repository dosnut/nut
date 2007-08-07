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

    class CDeviceOptions : public QTreeView {
        Q_OBJECT
    protected:
        QTabWidget * tabWidget;
    public:
        CDevice * device;
        QMenu * deviceMenu;
        QAction * enableDeviceAction;
        QAction * disableDeviceAction;
        QAction * showAction;
    
        CDeviceOptions(CDevice * parentDevice, QTabWidget * parentTabWidget, QWidget * parent = 0);
        ~CDeviceOptions();
    public slots:
        void showThisTab();
    };
};

#endif
