#include "cnotificationmanager.h"

#include <QWidget>
#include <QSystemTrayIcon>
#include <QIcon>
#include <QMenu>
#include <QAction>

#include "cuidevice.h"
#include "constants.h"
#include <libnutclient/cdevice.h>

namespace qnut {
	using namespace libnutclient;
	
	CNotificationManager::CNotificationManager(QWidget * parent) : QObject(parent) {
		m_MainIcon = new QSystemTrayIcon(QIcon(UI_ICON_QNUT_SMALL), this);
		QMenu * trayMenu = new QMenu("QNUT");
		trayMenu->addAction(tr("Open Connection &Manager"), parent, SLOT(show()));
		trayMenu->addSeparator();
		m_InsertMarker = trayMenu->addSeparator();
		trayMenu->addAction(tr("&Quit"), qApp, SLOT(quit()));
		trayMenu->setSeparatorsCollapsible(true);
		
		m_MainIcon->setContextMenu(trayMenu);
		
		connect(m_MainIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			this, SLOT(handleMainIconClicks(QSystemTrayIcon::ActivationReason)));
		
		if (parent)
			connect(m_MainIcon, SIGNAL(messageClicked()), parent, SLOT(show()));
	}
	
	CNotificationManager::~CNotificationManager() {}
	
	bool CNotificationManager::notificationsEnabled(CUIDevice * uiDevice) const {
		if (uiDevice)
			return uiDevice->m_NotificationsEnabled;
		else
			return m_NotificationsEnabled;
	}
	
	bool CNotificationManager::isIconVisible(CUIDevice * uiDevice) {
		if (uiDevice && m_UIDeviceIcons.contains(uiDevice))
			return m_UIDeviceIcons[uiDevice]->isVisible();
		else
			return m_MainIcon->isVisible();
	}
	
	void CNotificationManager::setIconVisible(bool value, CUIDevice * uiDevice) {
		if (uiDevice && m_UIDeviceIcons.contains(uiDevice))
			m_UIDeviceIcons[uiDevice]->setVisible(value);
		else
			m_MainIcon->setVisible(value);
	}
	
	void CNotificationManager::setIcon(QIcon icon, CUIDevice * uiDevice) {
		if (uiDevice && m_UIDeviceIcons.contains(uiDevice))
			m_UIDeviceIcons[uiDevice]->setIcon(icon);
		else
			m_MainIcon->setIcon(icon);
	}
	
	void CNotificationManager::registerUIDevice(CUIDevice * uiDevice) {
		QSystemTrayIcon * newTrayIcon = new QSystemTrayIcon(this);
		m_UIDeviceIcons.insert(uiDevice, newTrayIcon);
		
		newTrayIcon->setContextMenu(uiDevice->deviceMenu());
		m_MainIcon->contextMenu()->insertMenu(m_InsertMarker, uiDevice->deviceMenu());
		
		connect(uiDevice, SIGNAL(showNotificationRequested(libnutcommon::DeviceState)),
			this, SLOT(showNotification(libnutcommon::DeviceState)));
		
		connect(newTrayIcon, SIGNAL(messageClicked()), uiDevice, SLOT(showTheeseDetails()));
		connect(newTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			this, SLOT(handleDeviceIconActivated(QSystemTrayIcon::ActivationReason)));
	}
	
	void CNotificationManager::unregisterUIDevice(CUIDevice * uiDevice) {
		if (!m_UIDeviceIcons.contains(uiDevice))
			return;
		
		m_MainIcon->contextMenu()->removeAction(uiDevice->deviceMenu()->menuAction());
		delete m_UIDeviceIcons.take(uiDevice);
	}
	
	void CNotificationManager::setNotificationsEnabled(bool value) {
		setNotificationsEnabled(value, NULL);
	}
	
	void CNotificationManager::setNotificationsEnabled(bool value, CUIDevice * uiDevice) {
		if (uiDevice && m_UIDeviceIcons.contains(uiDevice))
			uiDevice->m_NotificationsEnabled = value;
		else
			m_NotificationsEnabled = value;
	}
	
	void CNotificationManager::showMessage(QString title, QString message) {
		CUIDevice * uiDevice = qobject_cast<CUIDevice *>(sender());
		if (uiDevice && m_UIDeviceIcons.contains(uiDevice) && m_UIDeviceIcons[uiDevice]->isVisible())
			m_UIDeviceIcons[uiDevice]->showMessage(title, message, QSystemTrayIcon::Information, 4000);
		else
			m_MainIcon->showMessage(title, message, QSystemTrayIcon::Information, 4000);
	}
	
	void CNotificationManager::showNotification(libnutcommon::DeviceState state) {
		CUIDevice * uiDevice = qobject_cast<CUIDevice *>(sender());
		if (!uiDevice)
			return;
		
		QString title;
		QString message;
		QSystemTrayIcon * trayIcon = m_UIDeviceIcons[uiDevice];
		if (trayIcon->isVisible()) {
			title = tr("QNUT - %1 ...").arg(uiDevice->device()->getName());
			switch (state) {
			case libnutcommon::DS_UP:
				message = tr("... is now up and running.");
				break;
			case libnutcommon::DS_UNCONFIGURED:
				message = tr("... got carrier but needs configuration.\n\nKlick here to open the device details.");
				break;
			case libnutcommon::DS_ACTIVATED:
				message = tr("... is now activated an waits for carrier.");
				break;
			case libnutcommon::DS_DEACTIVATED: 
				message = tr("... is now deactivated");
				break;
			default:
				return;
			}
		}
		else {
			title = tr("QNUT");
			switch (state) {
			case libnutcommon::DS_UP:
				message = tr("%1 is now up and running.");
				break;
			case libnutcommon::DS_UNCONFIGURED:
				message = tr("%1 got carrier but needs configuration.\n\nClick here to open the device details.");
				break;
			case libnutcommon::DS_ACTIVATED:
				message = tr("%1 is now activated an waits for carrier.");
				break;
			case libnutcommon::DS_DEACTIVATED:
				message = tr("%1 is now deactivated");
				break;
			default:
				return;
			}
			
			message = message.arg(uiDevice->device()->getName());
			trayIcon = m_MainIcon;
		}
		
		trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 4000);
	}
	
	void CNotificationManager::setToolTip(QString toolTip, CUIDevice * uiDevice) {
		if (uiDevice && m_UIDeviceIcons.contains(uiDevice))
			m_UIDeviceIcons[uiDevice]->setToolTip(toolTip);
		else
			m_MainIcon->setToolTip(toolTip);
	}
	
	void CNotificationManager::handleDeviceIconActivated(QSystemTrayIcon::ActivationReason reason) {
		if (reason == QSystemTrayIcon::Trigger) {
			CUIDevice * target = m_UIDeviceIcons.key(qobject_cast<QSystemTrayIcon *>(sender()), NULL);
			if (target)
				target->showTheeseDetails();
		}
	}
	
	void CNotificationManager::handleMainIconClicks(QSystemTrayIcon::ActivationReason reason) {
		QWidget * mainwin = qobject_cast<QWidget *>(parent());
		if (!mainwin)
			return;
		
		switch (reason) {
			case QSystemTrayIcon::Trigger:
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
