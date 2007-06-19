#include <QtGui>
#include <QTranslator>

#include "connman.h"

using namespace qnut;

int main(int argc, char * argv[])
{
    QApplication app(argc, argv);

    QString locale = QLocale::system().name();
    QTranslator translator;

    translator.load(QString("qnut_") + locale);
    app.installTranslator(&translator);
    app.setQuitOnLastWindowClosed(false);

    CConnectionManager mainwin;

    return app.exec();
}
