#include "cnotificationmanager.h"

#include <QWidget>
#include <QSystemTrayIcon>
#include <QIcon>
#include <QMenu>
#include <QAction>

#include <QApplication>

#include "cuidevice.h"
#include "constants.h"
#include "common.h"
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
		
		if (parent) {
			connect(m_MainIcon, SIGNAL(messageClicked()), parent, SLOT(show()));
			qApp->setQuitOnLastWindowClosed(false);
		}
	}
	
	CNotificationManager::~CNotificationManager() {}
	
	bool CNotificationManager::isIconVisible(CUIDevice * uiDevice) {
		if (uiDevice && m_UIDeviceIcons.contains(uiDevice))
			return m_UIDeviceIcons[uiDevice]->isVisible();
		else
			return m_MainIcon->isVisible();
	}
	
	void CNotificationManager::setIconVisible(bool value) {
		setIconVisible(value, qobject_cast<CUIDevice *>(sender()));
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
		
		updateDeviceIcon(uiDevice);
		setIconVisible(uiDevice->m_ShowTrayIcon, uiDevice);
		
		connect(uiDevice, SIGNAL(showNotificationRequested(libnutcommon::DeviceState)),
			this, SLOT(showNotification(libnutcommon::DeviceState)));
		connect(uiDevice, SIGNAL(updateTrayIconRequested(libnutcommon::DeviceState)),
			this, SLOT(updateDeviceIcon()));
		connect(uiDevice, SIGNAL(showTrayIconRequested(bool)),
			this, SLOT(setIconVisible(bool)));
		
		connect(newTrayIcon, SIGNAL(messageClicked()), uiDevice->m_ShowEnvironmentsAction, SIGNAL(triggered()));
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
		m_NotificationsEnabled = value;
	}
	
	void CNotificationManager::showMessage(QString title, QString message, CUIDevice * uiDevice) {
		if (!m_NotificationsEnabled)
			return;
		
		if (uiDevice && m_UIDeviceIcons.contains(uiDevice) && m_UIDeviceIcons[uiDevice]->isVisible())
			m_UIDeviceIcons[uiDevice]->showMessage(title, message, QSystemTrayIcon::Information, 4000);
		else
			m_MainIcon->showMessage(title, message, QSystemTrayIcon::Information, 4000);
	}
	
	inline void CNotificationManager::updateDeviceIcon(CUIDevice * uiDevice) {
		QSystemTrayIcon * trayIcon = m_UIDeviceIcons[uiDevice];
		trayIcon->setToolTip(shortSummary(uiDevice->device()));
		trayIcon->setIcon(QIcon(iconFile(uiDevice->device())));
	}
	
	void CNotificationManager::updateDeviceIcon() {
		updateDeviceIcon(qobject_cast<CUIDevice *>(sender()));
	}
	
	void CNotificationManager::showNotification(libnutcommon::DeviceState state) {
		if (!m_NotificationsEnabled)
			return;
		
		CUIDevice * uiDevice = qobject_cast<CUIDevice *>(sender());
		if (!uiDevice)
			return;
		
		QString title = tr("QNUT");
		QString message;
		QSystemTrayIcon * trayIcon = m_UIDeviceIcons[uiDevice];
		
		switch (state) {
		case libnutcommon::DS_UP:
			message = tr("%2 is now up and running on network: %1")
				.arg(currentNetwork(uiDevice->device()));
			break;
		case libnutcommon::DS_UNCONFIGURED:
			message = tr("%2 got carrier (to network: %1) but needs configuration.\n\nClick here to open the device details.")
				.arg(currentNetwork(uiDevice->device()));
			break;
		case libnutcommon::DS_ACTIVATED:
			message = tr("%1 is now activated and waits for carrier.");
			break;
		case libnutcommon::DS_DEACTIVATED:
			message = tr("%1 is now deactivated");
			break;
		default:
			return;
		}
		
		if (trayIcon->isVisible())
			title += " - " + uiDevice->device()->getName();
		
		message = message.arg(trayIcon->isVisible() ? tr("Device") : uiDevice->device()->getName());
		
		if (!trayIcon->isVisible())
			trayIcon = m_MainIcon;
		
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
				target->m_ShowEnvironmentsAction->trigger();
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
