#include <QtGui>
#include <QTranslator>

#include "trayicon.h"

int main(int argc, char * argv[])
{
    QApplication app(argc, argv);

    QString locale = QLocale::system().name();
    QTranslator translator;

    translator.load(QString("qnut_") + locale);
    app.installTranslator(&translator);

    CTrayIcon mainwin;

    mainwin.show();
    return app.exec();
}
