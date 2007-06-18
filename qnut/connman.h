#ifndef QNUT_CONNMAN_H
#define QNUT_CONNMAN_H

#include <QtGui>
#include "ui_connman.h"
#include "trayicon.h"

namespace qnut {
    class CConnectionManager : public QMainWindow {
        Q_OBJECT
    private:
        Ui::ConnMan ui;
    public:
        CTrayIcon trayicon;
        CConnectionManager(QWidget * parent = 0);
    };
};

#endif
