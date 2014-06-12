//
// C++ Implementation: CNotificationManager
//
// Author: Oliver Groß <z.o.gross@gmx.de>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
#include "cnotificationmanager.h"

#include <QWidget>
#include <QSystemTrayIcon>
#include <QIcon>
#include <QMenu>
#include <QAction>
#include <QApplication>

#include <libnutclient/cdevice.h>

#include "cuidevice.h"
#include "constants.h"
#include "common.h"

namespace qnut {
	using namespace libnutclient;

	CNotificationManager::CNotificationManager(QWidget * mainWindow, QObject * parent) : QObject(parent), m_MainWindow(mainWindow) {
		m_MainIcon = new QSystemTrayIcon(QIcon(UI_ICON_QNUT_SMALL), this);
		QMenu * trayMenu = new QMenu("QNUT");
		trayMenu->addAction(tr("Open Connection &Manager"), m_MainWindow, SLOT(show()));
		trayMenu->addSeparator();
		m_InsertMarker = trayMenu->addSeparator();
		trayMenu->addAction(tr("&Quit"), qApp, SLOT(quit()));
		trayMenu->setSeparatorsCollapsible(true);

		m_MainIcon->setContextMenu(trayMenu);

		connect(m_MainIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			this, SLOT(handleMainIconActivated(QSystemTrayIcon::ActivationReason)));

		if (m_MainWindow)
			qApp->setQuitOnLastWindowClosed(false);
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
			this, SLOT(showStateNotification(libnutcommon::DeviceState)));
		connect(uiDevice, SIGNAL(updateTrayIconRequested(libnutcommon::DeviceState)),
			this, SLOT(updateDeviceIcon()));
		connect(uiDevice, SIGNAL(showTrayIconRequested(bool)),
			this, SLOT(setIconVisible(bool)));

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
		if (uiDevice)
			showMessage(title, message, uiDevice->m_ShowEnvironmentsAction, SLOT(trigger()), uiDevice);
		else
			showMessage(title, message, m_MainWindow, SLOT(show()));
	}

	void CNotificationManager::showMessage(QString title, QString message, QObject * reciever, const char * slot, CUIDevice * uiDevice) {
		if (!m_NotificationsEnabled)
			return;

		QString titleResult = tr("QNUT");
		QSystemTrayIcon * trayIcon;
		if (uiDevice && m_UIDeviceIcons.contains(uiDevice) && m_UIDeviceIcons[uiDevice]->isVisible()) {
			trayIcon = m_UIDeviceIcons[uiDevice];
			titleResult += " (" + uiDevice->device()->getName() + ')';
		}
		else
			trayIcon = m_MainIcon;

		if (!title.isEmpty())
			titleResult += " - " + title;

		disconnect(trayIcon, SIGNAL(messageClicked()), NULL, NULL);
		if (reciever)
			connect(trayIcon, SIGNAL(messageClicked()), reciever, slot);
		trayIcon->showMessage(titleResult, message, QSystemTrayIcon::Information, 4000);
	}

	inline void CNotificationManager::updateDeviceIcon(CUIDevice * uiDevice) {
		QSystemTrayIcon * trayIcon = m_UIDeviceIcons[uiDevice];
		trayIcon->setToolTip(shortSummary(uiDevice->device()));
		trayIcon->setIcon(QIcon(iconFile(uiDevice->device())));
	}

	void CNotificationManager::updateDeviceIcon() {
		updateDeviceIcon(qobject_cast<CUIDevice *>(sender()));
	}

	void CNotificationManager::showStateNotification(libnutcommon::DeviceState state) {
		if (!m_NotificationsEnabled)
			return;

		CUIDevice * uiDevice = qobject_cast<CUIDevice *>(sender());
		if (!uiDevice)
			return;

		QString message;
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

		{
			QSystemTrayIcon * trayIcon = m_UIDeviceIcons.value(uiDevice, NULL);
			message = message.arg(trayIcon && trayIcon->isVisible() ? tr("Device") : uiDevice->device()->getName());
		}

		if (state == libnutcommon::DS_UNCONFIGURED)
			showMessage(QString(), message, uiDevice->m_ShowEnvironmentsAction, SLOT(trigger()), uiDevice);
		else
			showMessage(QString(), message, NULL, NULL, uiDevice);
	}

	void CNotificationManager::setToolTip(QString toolTip, CUIDevice * uiDevice) {
		if (uiDevice && m_UIDeviceIcons.contains(uiDevice))
			m_UIDeviceIcons[uiDevice]->setToolTip(toolTip);
		else
			m_MainIcon->setToolTip(toolTip);
	}

	void CNotificationManager::handleDeviceIconActivated(QSystemTrayIcon::ActivationReason reason) {
		if (reason != QSystemTrayIcon::Trigger)
			return;

		CUIDevice * target = m_UIDeviceIcons.key(qobject_cast<QSystemTrayIcon *>(sender()), NULL);
		if (target)
			target->m_ShowEnvironmentsAction->trigger();
	}

	void CNotificationManager::handleMainIconActivated(QSystemTrayIcon::ActivationReason reason) {
		if (!m_MainWindow)
			return;

		switch (reason) {
		case QSystemTrayIcon::Trigger:
			if (!m_MainWindow->isVisible() || !m_MainWindow->isActiveWindow() || m_MainWindow->isMinimized()) {
				m_MainWindow->show();
				m_MainWindow->activateWindow();
			}
			else
				m_MainWindow->close();
			break;
		default:
			break;
		}
	}
}
