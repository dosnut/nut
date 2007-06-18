#ifndef QNUT_TRAYICON_H
#define QNUT_TRAYICON_H

#include <QtGui>
#include <QTranslator>
#include <QSystemTrayIcon>
#include <QMenu>

namespace qnut {
    class CTrayIcon : public QSystemTrayIcon {
        Q_OBJECT
    public:
        QMenu trayMenu;
        QMenu devicesMenu;
    
        CTrayIcon(QObject * parent = 0);
    
    public slots:
        void nothingHere();
        void handleClicks(QSystemTrayIcon::ActivationReason reason);
    };
};

#endif
