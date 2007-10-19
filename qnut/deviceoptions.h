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
#include <QCheckBox>
#include <QSystemTrayIcon>
#include <QSettings>
#include <libnut/libnut_cli.h>

namespace qnut {
	using namespace libnut;
	
	class CDeviceOptions;
	typedef QHash<CDevice *, CDeviceOptions *> CDeviceOptionsHash;
	
	//todo: qtreeview und teile der device settings zusammenlegen
	//kein subclassing beim treeview nötig
	
	class CDeviceOptions : public QWidget {
		Q_OBJECT
	protected:
		//QTabWidget * tabWidget;
		QTreeView * environmentTree;
		QCheckBox * showTrayCheck;
		
		QSettings settings;
		
		inline void readSettings();
		inline void writeSettings();
		inline void createActions();
		inline void createView();
	public:
		quint8 scriptFlags;
		
		CDevice * device;
		
		QSystemTrayIcon * trayIcon;
		
		QMenu * deviceMenu;
		QAction * enableDeviceAction;
		QAction * disableDeviceAction;
		
		QAction * enterEnvironmentAction;
		
		QAction * deviceSettingsAction;
		QAction * ipConfigurationAction;
		
		QAction * showAction;
		
		
		CDeviceOptions(CDevice * parentDevice, QWidget * parent = 0);
		~CDeviceOptions();
		
	private slots:
		void uiHandleTrayActivated(QSystemTrayIcon::ActivationReason);
		
	public slots:
		void uiShowThisTab();
		void uiSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
		void uiChangeIPConfiguration();
		void uiChangeDeviceSettings();
		void uiHandleStateChange(DeviceState state);
		
	signals:
		void showMessage(QSystemTrayIcon * trayIcon, QString title, QString message);
		void showOptions(QWidget * widget);
	};
};

#endif
