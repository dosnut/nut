#ifndef QNUT_CNOTIFICATIONMANAGER_H
#define QNUT_CNOTIFICATIONMANAGER_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QHash>
#include <libnutcommon/device.h>

class QWidget;
class QAction;

namespace qnut {
	class CUIDevice;
	
	class CNotificationManager : public QObject {
		Q_OBJECT
	public:
		CNotificationManager(QWidget * parent = NULL);
		~CNotificationManager();
		
		bool notificationsEnabled() const { return m_NotificationsEnabled; }
		
		bool isIconVisible(CUIDevice * uiDevice);
		
		void setIcon(QIcon icon, CUIDevice * uiDevice = NULL);
		
		void registerUIDevice(CUIDevice * uiDevice);
		void unregisterUIDevice(CUIDevice * uiDevice);
		
		static bool trayIconsAvailable() { return QSystemTrayIcon::isSystemTrayAvailable(); }
	public slots:
		void setIconVisible(bool value, CUIDevice * uiDevice);
		
		void setNotificationsEnabled(bool value);
		
		void showMessage(QString title, QString message, CUIDevice * uiDevice = NULL);
		void setToolTip(QString tooltip, CUIDevice * uiDevice = NULL);
	private:
		QSystemTrayIcon * m_MainIcon;
		QHash<CUIDevice *, QSystemTrayIcon *> m_UIDeviceIcons;
		
		bool m_NotificationsEnabled;
		QAction * m_InsertMarker;
		inline void updateDeviceIcon(CUIDevice * uiDevice);
	private slots:
		void setIconVisible(bool value);
		
		void updateDeviceIcon();
		void showNotification(libnutcommon::DeviceState state);
		
		void handleDeviceIconActivated(QSystemTrayIcon::ActivationReason reason);
		void handleMainIconClicks(QSystemTrayIcon::ActivationReason reason);
	};
}
#endif // QNUT_CNOTIFICATIONMANAGER_H
