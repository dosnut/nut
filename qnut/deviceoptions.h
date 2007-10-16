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
#include <QSystemTrayIcon>
#include <libnut/libnut_cli.h>

namespace qnut {
	using namespace libnut;
	
	class CDeviceOptions;
	typedef QHash<CDevice *, CDeviceOptions *> CDeviceOptionsHash;
	
	class CDeviceOptions : public QTreeView {
		Q_OBJECT
	protected:
		QTabWidget * tabWidget;
		QString statusMessage(DeviceState state);
	public:
		CDevice * device;
		
		QSystemTrayIcon * trayIcon;
		
		QMenu * deviceMenu;
		QAction * enableDeviceAction;
		QAction * disableDeviceAction;
		QAction * enterEnvironmentAction;
		QAction * editInterfaceAction;
		
		QAction * showAction;
		
		void updateDeviceIcons();
		
		CDeviceOptions(CDevice * parentDevice, QTabWidget * parentTabWidget, QWidget * parent = 0);
		~CDeviceOptions();
		
	private slots:
		void uiHandleTrayActivated(QSystemTrayIcon::ActivationReason);
		
	public slots:
		void uiShowThisTab();
		void uiSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
		void uiChangeIPConfiguration();
		void uiHandleStateChange(DeviceState state);
		
	signals:
		void showMessage(QSystemTrayIcon * trayIcon, QString title, QString message);
	};
};

#endif
