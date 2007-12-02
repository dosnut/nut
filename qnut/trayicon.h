#ifndef QNUT_TRAYICON_H
#define QNUT_TRAYICON_H

#include <QSystemTrayIcon>
#include <QMenu>

namespace qnut {
	class CTrayIcon : public QSystemTrayIcon {
		Q_OBJECT
	protected:
		QMenu m_TrayMenu;
		QMenu m_DevicesMenu;
	public:
		void addDeviceMenu(QMenu * deviceMenu);
		void removeDeviceMenu(QMenu * deviceMenu);
		CTrayIcon(QObject * parent = 0);
	public slots:
		void handleClicks(QSystemTrayIcon::ActivationReason reason);
	};
};

#endif
