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
#include <libnutclient/client.h>
#include "ui/ui_devdet.h"

namespace qnut {
	class CWirelessSettings;
	class CDeviceDetails;
	typedef QHash<libnutclient::CDevice *, CDeviceDetails *> CDeviceDetailsHash;
	
	/** @brief CDeviceDetails interacts directly with CDevice
		The device details class provides functions to open the windows for
		scritping settings and wireless settings (if the device has wireless
		extensions).
	*/
	class CDeviceDetails : public QWidget {
		Q_OBJECT
	protected:
		Ui::devdet ui;
		QSettings m_Stettings;
		quint8 m_ScriptFlags;
		
		libnutclient::CDevice * m_Device;
		
		CWirelessSettings * m_WirelessSettings;
		
		QMenu * m_DeviceMenu;
		QList<QAction *> m_DeviceActions;
		
		QAction * m_EnterEnvironmentAction;
		QAction * m_IPConfigurationAction;
		
		QSystemTrayIcon * m_trayIcon;
		
		inline void readSettings();
		inline void writeSettings();
		inline void createActions();
		inline void createView();
		inline void setHeadInfo();
	public:
		inline quint8 scriptFlags() const { return m_ScriptFlags; }
		inline void setScriptFlags(quint8 value) { m_ScriptFlags = value; }
		inline libnutclient::CDevice * device() const { return m_Device; }
		inline QMenu * trayMenu() const { return m_DeviceMenu; }
		
		inline QList<QAction *> deviceActions() const { return m_DeviceActions; }
		inline QList<QAction *> environmentTreeActions() const { return ui.environmentTree->actions(); }
		
		CDeviceDetails(libnutclient::CDevice * parentDevice, QWidget * parent = 0);
		~CDeviceDetails();
	protected slots:
		void handleTrayActivated(QSystemTrayIcon::ActivationReason);
		void handleSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
		void handleDeviceStateChange(libnutcommon::DeviceState state);
		void showTheeseDetails();
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
