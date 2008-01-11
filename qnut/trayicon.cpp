//
// C++ Implementation: trayicon
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include <QtGui>
#include <QTranslator>
#include "trayicon.h"
#include "constants.h"

using namespace std;

namespace qnut {
	CTrayIcon::CTrayIcon(QObject * parent) : QSystemTrayIcon(QIcon(UI_ICON_QNUT_SMALL), parent) {
		m_DevicesMenu.setTitle(tr("Network &devices"));

		m_TrayMenu.setTitle("QNUT");
		m_TrayMenu.addAction(tr("Open Connection &Manager"), parent, SLOT(show()));

		m_TrayMenu.addMenu(&m_DevicesMenu);
		m_DevicesMenu.setEnabled(false);
		m_TrayMenu.addSeparator();
		m_TrayMenu.addAction(tr("&Quit"), qApp, SLOT(quit()));

		setContextMenu(&m_TrayMenu);

		connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			this, SLOT(handleClicks(QSystemTrayIcon::ActivationReason)));
	}
	
	void CTrayIcon::addDeviceMenu(QMenu * deviceMenu) {
		m_DevicesMenu.addMenu(deviceMenu);
		m_DevicesMenu.setEnabled(true);
	}
	
	void CTrayIcon::removeDeviceMenu(QMenu * deviceMenu) {
		m_DevicesMenu.removeAction(deviceMenu->menuAction());
		m_DevicesMenu.setDisabled(m_DevicesMenu.isEmpty());
	}
	
	void CTrayIcon::handleClicks(QSystemTrayIcon::ActivationReason reason) {
		QWidget * mainwin = dynamic_cast<QWidget *>(parent());
		switch (reason) {
			case Trigger:
				if (mainwin->isVisible()) {
					mainwin->close();
				}
				else {
					mainwin->show();
				}
				break;
			default:
				break;
		}
	}
}
