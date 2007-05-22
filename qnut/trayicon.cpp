#include "trayicon.h"

CTrayIcon::CTrayIcon(QObject * parent) : QSystemTrayIcon(parent) {
    //Kontextmenü erstellen
    devicesMenu.setTitle(tr("&Network Devices"));
    devicesMenu.addAction(tr("empty..."));
    
    trayMenu.setTitle("KNut");
    trayMenu.addAction(tr("open Connection &Manager"), this, SLOT(nothingHere()));
    
    trayMenu.addMenu(&devicesMenu);
    trayMenu.addSeparator();
    trayMenu.addAction(tr("&Quit"), qApp, SLOT(quit()));

    setContextMenu(&trayMenu);
}

void CTrayIcon::nothingHere() {
    showMessage(tr("no Function here"), tr("This Funktion is not implemented yet"));
}
