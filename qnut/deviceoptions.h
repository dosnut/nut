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
#include <QLabel>
#include <libnut/libnut_cli.h>
#include "ui/ui_devopt.h"
#include "wirelesssettings.h"

namespace qnut {
	using namespace libnut;
	
	class CDeviceOptions;
	typedef QHash<CDevice *, CDeviceOptions *> CDeviceOptionsHash;
		
	class CDeviceOptions : public QWidget {
		Q_OBJECT
	protected:
		Ui::devopt ui;
		
		CWirelessSettings * wirelessSettings;
		
		QSettings settings;
		
		inline void readSettings();
		inline void writeSettings();
		inline void createActions();
		inline void createView();
		inline void setHeadInfo();
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
		QAction * wirelessSettingsAction;
		
		QAction * showAction;
		
		CDeviceOptions(CDevice * parentDevice, QWidget * parent = 0);
		~CDeviceOptions();
	private slots:
		void handleTrayActivated(QSystemTrayIcon::ActivationReason);
		void showTheeseOptions();
		void handleSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
		void handleDeviceStateChange(DeviceState state);
		void openIPConfiguration();
	public slots:
		void openDeviceSettings();
		void openWirelessSettings();
	signals:
		void showMessageRequested(QString title, QString message, QSystemTrayIcon * trayIcon = NULL);
		void showOptionsRequested(QWidget * widget);
	};
};

#endif
