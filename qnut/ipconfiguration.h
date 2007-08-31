//
// C++ Interface: ipconf
//
// Description: 
//
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QNUTIPCONF_H
#define QNUTIPCONF_H

#include <QDialog>
#include "ui_ipconf.h"
#include "libnut_cli.h"

namespace qnut {
    using namespace libnut;

    class CIPConfiguration : public QDialog {
        Q_OBJECT
    private:
        Ui::ipconf ui;
    public:
        bool execute(CInterface * interface);
        bool execute(bool & isStatic, QHostAddress & ip, QHostAddress & netmask, QHostAddress & gateway);
        
        CIPConfiguration(QWidget * parent = 0);
        ~CIPConfiguration();
    };

};

#endif
