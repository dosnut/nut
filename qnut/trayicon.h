//
// C++ Interface: trayicon
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef QNUT_TRAYICON_H
#define QNUT_TRAYICON_H

#include <QSystemTrayIcon>

namespace qnut {
	/**
	 * @brief CTrayIcon provides an icon in the system tray for the main user control interface of QNUT.
	 * @author Oliver Groß <z.o.gross@gmx.de>
	 * 
	 * On creation, the CTrayIcon sets up an icon in the system tray to manage the basic application
	 * behavior such as showing/hiding the main window.
	 * 
	 * The class provides public functions to add and remove sub menus in the "Network devices" sub menu.
	 */
	class CTrayIcon : public QSystemTrayIcon {
		Q_OBJECT
	private:
		QAction * m_InsertMarker;
	public:
		/**
		 * @brief Adds a given sub menu to the "Network devices" sub menu.
		 * @param deviceMenu menu to add
		 */
		void addDeviceMenu(QMenu * deviceMenu);
		
		/**
		 * @brief Removes an existing sub menu to the "Network devices" sub menu.
		 * @param deviceMenu menu to remove
		 */
		void removeDeviceMenu(QMenu * deviceMenu);
		
		/**
		 * @brief Creates the class and initializes the tray icon and its context menu.
		 * @param parent parent object
		 */
		CTrayIcon(QObject * parent = 0);
	private slots:
		void handleClicks(QSystemTrayIcon::ActivationReason reason);
	};
}

#endif
