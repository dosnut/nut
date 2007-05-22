#ifndef TRAYICON_H
#define TRAYICON_H

#include <QtGui>
#include <QTranslator>
#include <QSystemTrayIcon>
#include <QMenu>

class CTrayIcon : public QSystemTrayIcon {
    Q_OBJECT
public:
    QMenu trayMenu;
    QMenu devicesMenu;

    CTrayIcon(QObject * parent = 0);

public slots:
    void nothingHere();
};

#endif
