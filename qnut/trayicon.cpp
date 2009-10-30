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

namespace qnut {
	CTrayIcon::CTrayIcon(QObject * parent) : QSystemTrayIcon(QIcon(UI_ICON_QNUT_SMALL), parent) {
		QMenu * trayMenu = new QMenu("QNUT");
		trayMenu->addAction(tr("Open Connection &Manager"), parent, SLOT(show()));
		trayMenu->addSeparator();
		m_InsertMarker = trayMenu->addSeparator();
		trayMenu->addAction(tr("&Quit"), qApp, SLOT(quit()));
		trayMenu->setSeparatorsCollapsible(true);
		
		setContextMenu(trayMenu);
		
		connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			this, SLOT(handleClicks(QSystemTrayIcon::ActivationReason)));
	}
	
	void CTrayIcon::addDeviceMenu(QMenu * deviceMenu) {
		contextMenu()->insertMenu(m_InsertMarker, deviceMenu);
	}
	
	void CTrayIcon::removeDeviceMenu(QMenu * deviceMenu) {
		contextMenu()->removeAction(deviceMenu->menuAction());
	}
	
	void CTrayIcon::handleClicks(QSystemTrayIcon::ActivationReason reason) {
		QWidget * mainwin = qobject_cast<QWidget *>(parent());
		switch (reason) {
			case Trigger:
				if (mainwin->isMinimized() || !mainwin->isVisible()) {
					mainwin->show();
					mainwin->setWindowState((mainwin->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
				}
				else
					mainwin->close();
				break;
			default:
				break;
		}
	}
}
