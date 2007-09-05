#include <QtGui>
#include <QTranslator>
#include "connectionmanager.h"
#include "constants.h"

using namespace qnut;

int main(int argc, char * argv[])
{
    common::init();
    QApplication app(argc, argv);

    QString locale = QLocale::system().name();
    QTranslator translator;
    translator.load(QString(UI_DIR_TRANSLATIONS "qnut_") + locale);
    app.installTranslator(&translator);
    app.setQuitOnLastWindowClosed(false);
    
    CConnectionManager mainwin;
    return app.exec();
}
