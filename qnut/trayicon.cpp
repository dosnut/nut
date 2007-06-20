#include "trayicon.h"
#include <iostream>
using namespace std;

namespace qnut {
    CTrayIcon::CTrayIcon(QObject * parent) : QSystemTrayIcon(QIcon("res/network.svg"), parent) {
        devicesMenu.setTitle(tr("&Network Devices"));
        devicesMenu.addAction(tr("empty..."));
        
        trayMenu.setTitle("KNut");
        trayMenu.addAction(tr("open Connection &Manager"), parent, SLOT(show()));
        
        trayMenu.addMenu(&devicesMenu);
        trayMenu.addSeparator();
        trayMenu.addAction(tr("&Quit"), qApp, SLOT(quit()));
        
        setContextMenu(&trayMenu);
        
        connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(handleClicks(QSystemTrayIcon::ActivationReason)));
    }

    void CTrayIcon::handleClicks(QSystemTrayIcon::ActivationReason reason) {
        QWidget * mainwin = (QWidget *)(parent());
        switch (reason) {
            case Trigger:
                if (mainwin->isVisible())
                    mainwin->close();
                else
                    mainwin->show();
                break;
            default:
                break;
        }
    }

    void CTrayIcon::nothingHere() {
        showMessage(tr("no Function here"), tr("This Funktion is not implemented yet"));
    }
};
