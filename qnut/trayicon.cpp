#include "trayicon.h"
#include "constants.h"

using namespace std;

namespace qnut {
	CTrayIcon::CTrayIcon (QObject * parent) : QSystemTrayIcon(QIcon(UI_ICON_SYSTRAY), parent) {
		devicesMenu.setTitle (tr("Network &Devices"));

		trayMenu.setTitle("KNut");
		trayMenu.addAction(tr("Open Connection &Manager"), parent, SLOT(show()));

		trayMenu.addMenu(&devicesMenu);
		devicesMenu.setEnabled (false);
		trayMenu.addSeparator();
		trayMenu.addAction(tr("&Quit"), qApp, SLOT(quit()));

		setContextMenu(&trayMenu);

		connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
		        this, SLOT(handleClicks(QSystemTrayIcon::ActivationReason)));
	}

	void CTrayIcon::handleClicks (QSystemTrayIcon::ActivationReason reason) {
		QWidget * mainwin = (QWidget *)(parent());
		switch (reason) {
			case Trigger:
				if (mainwin->isVisible()) {
					mainwin->close();
// 					if (mainwin->isActiveWindow())
// 						mainwin->close();
// 					else {
// 						mainwin->activateWindow();
// 						mainwin->raise();
// 					}
				}
				else {
					mainwin->show();
					//mainwin->activateWindow();
				}
				break;
			default:
				break;
		}
	}

	void CTrayIcon::nothingHere(){
		showMessage(tr("rno Function here"), tr("This Funktion is not implemented yet"));
	}
};
