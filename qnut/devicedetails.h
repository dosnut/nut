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
#ifndef QNUT_DEVICEDETAILS_H
#define QNUT_DEVICEDETAILS_H

#include <QSystemTrayIcon>
#include <QSettings>
#include <libnut/libnut_cli.h>
#include "ui/ui_devopt.h"

namespace qnut {
	class CWirelessSettings;
	class CDeviceDetails;
	typedef QHash<libnut::CDevice *, CDeviceDetails *> CDeviceDetailsHash;
		
	class CDeviceDetails : public QWidget {
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
		
		libnut::CDevice * device;
		
		QSystemTrayIcon * trayIcon;
		
		QMenu * deviceMenu;
		QAction * enableDeviceAction;
		QAction * disableDeviceAction;
		
		QAction * enterEnvironmentAction;
		
		QAction * deviceSettingsAction;
		QAction * ipConfigurationAction;
		QAction * wirelessSettingsAction;
		
		QAction * showAction;
		
		CDeviceDetails(libnut::CDevice * parentDevice, QWidget * parent = 0);
		~CDeviceDetails();
	private slots:
		void handleTrayActivated(QSystemTrayIcon::ActivationReason);
		void showTheeseDetails();
		void handleSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
		void handleDeviceStateChange(libnut::DeviceState state);
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
